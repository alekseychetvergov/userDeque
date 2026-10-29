
#pragma once

#include <iostream>
#include <iterator>
#include <myDeque/my_deque.hpp>
#include <my_memory/my_allocator.hpp>

namespace FaultyAllocator {

    template <typename T>
    class FaultyAllocator : public std::allocator<T> {
    public:
        T* allocate(std::size_t n) {
            throw std::bad_alloc(); // Нарочно имитируем нехватку памяти
        }
    };

    class MockMemoryResource : public PmrAllocator::MyMemoryResource {
    public:
        size_t allocate_calls = 0;   // Сколько раз просили память
        size_t deallocate_calls = 0; // Сколько раз возвращали память
        size_t active_allocations = 0; // Сколько блоков занято прямо сейчас

        virtual void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) override {
            allocate_calls++;
            active_allocations++;
            return PmrAllocator::MyMemoryResource::allocate(bytes, alignment);
        }

        virtual void deallocate(void* ptr, size_t bytes, size_t alignment = alignof(std::max_align_t)) noexcept override {
            if (ptr) {
                deallocate_calls++;
                active_allocations--;
            }
            PmrAllocator::MyMemoryResource::deallocate(ptr, bytes, alignment);
        }

        // Метод для проверки, что вся память была возвращена ресурсу
        bool has_leaks() const {
            return active_allocations != 0;
        }
    };
    class BadMemoryResource : public PmrAllocator::MyMemoryResource {
    public:
        static inline size_t max_size = 500;   // Сколько раз просили память

        virtual void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) override {
            if (bytes > max_size) throw std::bad_alloc();
            return PmrAllocator::MyMemoryResource::allocate(bytes, alignment);
        }

        virtual void deallocate(void* ptr, size_t bytes, size_t alignment = alignof(std::max_align_t)) noexcept override {
            if (ptr) {
            }
            PmrAllocator::MyMemoryResource::deallocate(ptr, bytes, alignment);
        }
        ~BadMemoryResource() = default;

        // Сравнение ресурсов (два разных ресурса не могут освобождать память друг друга)
        bool is_equal(const MyMemoryResource & other) const noexcept {
            return this == &other;
        }
    }; 

    template <typename T>
    class ExeptAllocator {
    public:
        inline static bool should_throw_on_move = false;
        using propagate_on_container_move_assignment = std::true_type;
        using value_type = T;
        int dummy_field = 0;

        ExeptAllocator() noexcept = default;
        ExeptAllocator(const ExeptAllocator&) = default;

        // Шаблонный конструктор копирования для rebind
        template <typename U>
        ExeptAllocator(const ExeptAllocator<U>&) noexcept {}

        // 1. ШАБЛОННЫЙ конструктор перемещения (перехватит любой тип при rebind)
        template <typename U>
        ExeptAllocator(ExeptAllocator<U>&&) noexcept(false) {
            if (should_throw_on_move) {
                //std::cout << "ExeptAllocator(ExeptAllocator<U>&&) - throw std::overflow_error\n";
                throw std::overflow_error("&&&");
            }
        }

        // Обычный конструктор перемещения (требуется для явного std::move)
        ExeptAllocator(ExeptAllocator&&) noexcept(false) {
            if (should_throw_on_move) {
                //std::cout << "ExeptAllocator(ExeptAllocator&&) - throw std::overflow_error\n";
                throw std::overflow_error("&&&");
            }
        }

        // 2. ШАБЛОННЫЙ оператор перемещающего присваивания
        // Он гарантированно сработает, даже если STL подменит тип T на T* при перемещении буфера
        template <typename U>
        ExeptAllocator& operator=(ExeptAllocator<U>&& rhs) noexcept(false) {
            if (should_throw_on_move) {
                //std::cout << "ExeptAllocator& operator=(ExeptAllocator<U>&& rhs) - throw std::invalid_argument\n";
                throw std::invalid_argument("***");
            }
            return *this;
        }

        // Обычный оператор перемещающего присваивания
        ExeptAllocator& operator=(ExeptAllocator&& rhs) noexcept(false) {
            if (should_throw_on_move) {
                //std::cout << "ExeptAllocator& operator=(ExeptAllocator&& rhs) - throw std::invalid_argument\n";
                throw std::invalid_argument("***");
            }
            return *this;
        }

        T* allocate(size_t n) {
            if (n == 0) return nullptr;
            void* ptr = ::operator new(n * sizeof(T));
            return static_cast<T*>(ptr);
        }

        void deallocate(T* p, size_t n) noexcept {
            if (p == nullptr) return;
            ::operator delete(p);
        }

        template <typename U> bool operator==(const ExeptAllocator<U>&) const noexcept { return true; }
        template <typename U> bool operator!=(const ExeptAllocator<U>&) const noexcept { return false; }
    };
}



namespace userClass {
    struct Controllable {
        int i;
        float f;
        std::string s;

        Controllable() : i(0), f(0.f), s("0") {}
        Controllable(int i_, float f_, std::string s_) : i(i_), f(f_), s(s_) {}
        
        bool operator==(const Controllable& other) const noexcept {
            return ( (i == other.i) && (f == other.f) && (s == other.s) );
        } 

        bool operator!=(const Controllable& other) const noexcept {
            return !(*this == other);
        }
        auto operator<=>(const Controllable& other) const noexcept {
            return f <=> other.f;
        }
        friend std::ostream& operator<<(std::ostream& os, const Controllable& obj) {
            os << "{ i: " << obj.i << ", f: " << obj.f << ", s: \"" << obj.s << "\" }";
            return os;
        }
    };

    struct ThrowingElement {
        static inline int construct_count = 0;
        static inline int max_success_allocs = 5; // Упадет на 6-м элементе
        static inline int max_moves_elem = 100; // Упадет на 6-м элементе при перемещении

        ThrowingElement() { ++construct_count;
            //std::cout << "DefCtor++construct_count ("<< construct_count << ")\n";
        }
        ThrowingElement(const ThrowingElement&) {
            ++construct_count;
            if (construct_count > max_success_allocs) {
                --construct_count;
                throw std::runtime_error("Copy constructor failure!");
            }
            //std::cout << "CopyCtor++construct_count (" << construct_count << ")\n";
        };
        ThrowingElement(ThrowingElement&&) noexcept(false)
        {
            ++construct_count;
            if (construct_count > max_moves_elem) {
                --construct_count;
                throw std::runtime_error("Move constructor failure!");
            }
            //std::cout << "MoveCtor++construct_count (" << construct_count << ")\n";
        }
        ThrowingElement operator=(const ThrowingElement&) {
            if (construct_count > max_success_allocs) {
                throw std::runtime_error("Copy operator= constructor failure!");
            }
            //std::cout << "Copy operator= construct_count (" << construct_count << ")\n";
        }

        ThrowingElement operator=(ThrowingElement&&) {
            if (construct_count > max_moves_elem) {
                throw std::runtime_error("Move operator= constructor failure!");
            }
            //std::cout << "Move operator= construct_count " << construct_count << "\n";
        }

        ~ThrowingElement() { --construct_count; 
            //std::cout << "Dctor--construct_count (" << construct_count << ")\n";
        }
    };
}