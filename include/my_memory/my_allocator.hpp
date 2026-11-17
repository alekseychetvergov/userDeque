#pragma once

#include <cstddef>
#include <memory>
#include <iostream>
#include <new>

namespace SimpleAlloc {
    template <typename T>
    class MySimpleAllocator {
    public:
        using value_type = T;

        MySimpleAllocator() noexcept = default;

        template <typename U>
        MySimpleAllocator(const MySimpleAllocator<U>&) noexcept {}

        T* allocate(size_t n) {
            if (n == 0) return nullptr;

#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator] Memory allocation for " << n
                << " element(s) with a size of " << sizeof(T) << " byte all .\n";
#endif // PRINT_TECH_ALLOCATION

            void* ptr = ::operator new(n * sizeof(T));
            return static_cast<T*>(ptr);
        }

        void deallocate(T* p, size_t n) noexcept {
            if (p == nullptr) return;
#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator] Freeing up memory from " << n
                << " element(s).\n";
#endif // PRINT_TECH_ALLOCATION
            ::operator delete(p);
        }

        template <typename U>
        bool operator==(const MySimpleAllocator<U>&) const noexcept { return true; }

        template <typename U>
        bool operator!=(const MySimpleAllocator<U>&) const noexcept { return false; }
    };
}

namespace PmrAllocator {
    class MyMemoryResource {
    public:
        virtual void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) {
#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Resource] Physical isolation " << bytes
                << " byte (alignment): " << alignment << ").\n";
#endif // PRINT_TECH_ALLOCATION
            return ::operator new(bytes, std::align_val_t(alignment));
        }

        virtual void deallocate(void* ptr, size_t bytes, size_t alignment = alignof(std::max_align_t)) noexcept {
            if (!ptr) return;
#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Resource] Physical liberation " << bytes << " byte.\n";
#endif // PRINT_TECH_ALLOCATION
            ::operator delete(ptr, std::align_val_t(alignment));
        }

        virtual ~MyMemoryResource() = default;

        bool is_equal(const MyMemoryResource& other) const noexcept {
            return this == &other;
        }
    };

    static MyMemoryResource default_res;

    template <typename T>
    class MyPmrAllocator {
    private:
        MyMemoryResource* res_ = nullptr; 

    public:
        using value_type = T;
        using is_always_equal = std::false_type;

        MyPmrAllocator() noexcept {
            res_ = &default_res;
        }

        explicit MyPmrAllocator(MyMemoryResource* res) noexcept : res_(res) {
            if (!res_)
                res_ = &default_res;
        }

        template <typename U>
        MyPmrAllocator(const MyPmrAllocator<U>& other) noexcept : res_(other.resource()) {}

        MyPmrAllocator(const MyPmrAllocator& other) noexcept : res_(other.res_) {}

        MyPmrAllocator& operator=(const MyPmrAllocator& other) noexcept {
            if (this != &other) {
                res_ = other.res_;
            }
            return *this;
        }
        MyPmrAllocator(MyPmrAllocator&& other) noexcept : res_(other.res_) {
        }
        MyPmrAllocator& operator=(MyPmrAllocator&& other) noexcept {
            if (this != &other) {
                res_ = other.res_;
                other.res_ = &default_res; 
            }
            return *this;
        }

        MyMemoryResource* resource() const noexcept { return res_; }

        T* allocate(size_t n) {
            if (n == 0) return nullptr;

#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator <" << typeid(T).name() << ">] I am redirecting the request to "
                << n << " elements in MyMemoryResource.\n";
#endif // PRINT_TECH_ALLOCATION

            size_t total_bytes = n * sizeof(T);
            void* raw_ptr = res_->allocate(total_bytes, alignof(T));

            return static_cast<T*>(raw_ptr);
        }

        void deallocate(T* p, size_t n) noexcept {
            if (!p) return;

#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator <" << typeid(T).name() << ">] Redirecting the release. "
                << n << " elements in MyMemoryResource.\n";
#endif // PRINT_TECH_ALLOCATION

            size_t total_bytes = n * sizeof(T);
            res_->deallocate(p, total_bytes, alignof(T));
        }

        template <typename U, typename... Args>
        void construct(U* p, Args&&... args) {
            if constexpr (std::uses_allocator_v<U, MyPmrAllocator>) {

                std::allocator_traits<MyPmrAllocator>::construct(*this, p, std::forward<Args>(args)...);
            }
            else {

                ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
            }
        }

        template <typename U>
        void destroy(U* p) noexcept {
            p->~U(); 
        }

        template <typename ForwardIterator>
        void destroy(ForwardIterator first, ForwardIterator last) noexcept {
            while (first != last) {
                destroy(std::addressof(*first));
                ++first;
            }
        }

        template <typename U>
        bool operator==(const MyPmrAllocator<U>& other) const noexcept {
            return res_->is_equal(*other.resource());
        }

        template <typename U>
        bool operator!=(const MyPmrAllocator<U>& other) const noexcept {
            return !(*this == other);
        }
    };
}