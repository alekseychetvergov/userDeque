#pragma once

#include <memory>
#include <utility>
#include <new>
#include <stdexcept>
#include <type_traits>

#include <myDeque/my_deque.hpp>

namespace Deque {
    template <typename T, typename Allocator = std::allocator<T>, size_t SIZE_ELEM_MAP = 8> struct MyDequeBuff {
        static_assert(SIZE_ELEM_MAP > 0, "SIZE_ELEM_MAP must be greater than zero");
    protected:
        using traits_type = std::allocator_traits<Allocator>;
        using allocator_type = Allocator;
        using map_allocator_type = typename traits_type::template rebind_alloc<T*>;
        using map_traits_type = std::allocator_traits<map_allocator_type>;

        T** map_;
        size_t size_map_ = 0;

        size_t size_elem_map_ = SIZE_ELEM_MAP;
        size_t first_block_ = 0;
        size_t last_block_ = 0;

        allocator_type alloc_;

        void move_assign_after_allocator_propagation(MyDequeBuff&& rhs, allocator_type& old_alloc);

        template <typename U, typename Alloc, size_t SIZE>
        friend class MyDeque;
    public:
        MyDequeBuff(size_t size_map, const allocator_type& alloc = allocator_type());
        MyDequeBuff(const MyDequeBuff& rhs) = delete;
        MyDequeBuff& operator= (const MyDequeBuff& rhs) = delete;

        //Move Ctor
        MyDequeBuff(MyDequeBuff&& rhs) noexcept(std::is_nothrow_move_constructible_v<allocator_type>);
        //Move Assigm
        MyDequeBuff& operator= (MyDequeBuff&& rhs) noexcept (//false);
            (traits_type::propagate_on_container_move_assignment::value &&
                std::is_nothrow_move_assignable_v<Allocator>) ||
            traits_type::is_always_equal::value);

        T* allocate_block();

        void deallocate_block(T* block) noexcept;

        ~MyDequeBuff() noexcept;
    
        size_t getSizeMap() {return size_map_;}
        size_t getSizeElemMap() { return SIZE_ELEM_MAP; }
        const T* const* getMap() const noexcept {return map_;}
        T** getMap() noexcept {return map_;}
        size_t getFirstBlock() const { return first_block_; }
        size_t getLastBlock() const { return last_block_; }
        //void setFirstBlock(size_t firstBlock) { first_block_ = firstBlock; }
        //void setLasttBlock(size_t lastBlock) { last_block_ = lastBlock; }
    };

    template <typename T, typename Allocator, size_t SIZE_ELEM_MAP>
    MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>::MyDequeBuff(size_t size_map, const allocator_type& alloc)
        : size_map_(size_map), size_elem_map_(SIZE_ELEM_MAP), alloc_(alloc)
    {
        if (size_map_ > 0) {
            map_allocator_type map_alloc(alloc_);
            map_ = map_traits_type::allocate(map_alloc, size_map_);

            try {
                for (size_t i = 0; i < size_map_; ++i) {
                    map_[i] = nullptr;
                }
            }
            catch (...) {
                map_traits_type::deallocate(map_alloc, map_, size_map_);
                throw;
            }
        }
        else {
            map_ = nullptr;
        }
    }

    template <typename T, typename Allocator, size_t SIZE_ELEM_MAP>
    MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>::MyDequeBuff(MyDequeBuff&& rhs) noexcept(std::is_nothrow_move_constructible_v<allocator_type>)
        try : alloc_(std::move(rhs.alloc_)), map_(nullptr), size_map_(0), size_elem_map_(SIZE_ELEM_MAP),
        first_block_(0), last_block_(0)
    {
        std::swap(map_, rhs.map_);
        std::swap(size_map_, rhs.size_map_);
        std::swap(size_elem_map_, rhs.size_elem_map_);
        std::swap(first_block_, rhs.first_block_);
        std::swap(last_block_, rhs.last_block_);
    }
    catch (...) {
        throw;
    }

    template <typename T, typename Allocator, size_t SIZE_ELEM_MAP>
    MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>&
        MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>::operator= (MyDequeBuff&& rhs) noexcept (//false)
            (traits_type::propagate_on_container_move_assignment::value&&
                std::is_nothrow_move_assignable_v<Allocator>) ||
            traits_type::is_always_equal::value)
    {
        if (this != &rhs) {
            if constexpr (traits_type::propagate_on_container_move_assignment::value) {
                allocator_type old_alloc = alloc_;
                alloc_ = std::move(rhs.alloc_);
                move_assign_after_allocator_propagation(std::move(rhs), old_alloc);
            }
            else {
                if constexpr (!traits_type::is_always_equal::value) {
                    if (!(alloc_ == rhs.alloc_)) {
                        throw std::invalid_argument("Cannot move-assign buffers with unequal allocators");
                    }
                }
                move_assign_after_allocator_propagation(std::move(rhs), alloc_);
            }
        }
        return *this;
    }

    template <typename T, typename Allocator, size_t SIZE_ELEM_MAP>
    void MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>::move_assign_after_allocator_propagation(
        MyDequeBuff&& rhs, allocator_type& old_alloc) {
        if (map_ != nullptr) {
            for (size_t i = 0; i < size_map_; ++i) {
                if (map_[i] != nullptr) {
                    traits_type::deallocate(old_alloc, map_[i], SIZE_ELEM_MAP);
                }
            }
            map_allocator_type map_alloc(old_alloc);
            map_traits_type::deallocate(map_alloc, map_, size_map_);
        }

        map_ = rhs.map_;
        size_map_ = rhs.size_map_;
        size_elem_map_ = rhs.size_elem_map_;
        first_block_ = rhs.first_block_;
        last_block_ = rhs.last_block_;

        rhs.map_ = nullptr;
        rhs.size_map_ = 0;
        rhs.size_elem_map_ = SIZE_ELEM_MAP;
        rhs.first_block_ = 0;
        rhs.last_block_ = 0;
    }

    template <typename T, typename Allocator, size_t SIZE_ELEM_MAP>
    T* MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>::allocate_block() {
        return traits_type::allocate(alloc_, SIZE_ELEM_MAP);
    }

    template <typename T, typename Allocator, size_t SIZE_ELEM_MAP>
    void MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>::deallocate_block(T* block) noexcept {
        traits_type::deallocate(alloc_, block, SIZE_ELEM_MAP);
    }

    template <typename T, typename Allocator, size_t SIZE_ELEM_MAP>
    MyDequeBuff<T, Allocator, SIZE_ELEM_MAP>::~MyDequeBuff() noexcept {
        if (map_ != nullptr) {
            for (size_t i = 0; i < size_map_; ++i) {
                if (map_[i] != nullptr) {
                    deallocate_block(map_[i]); // ������ ::operator delete
                }
            }

            map_allocator_type map_alloc(alloc_);
            map_traits_type::deallocate(map_alloc, map_, size_map_);
        }
    }
}
