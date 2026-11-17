#pragma once

#include <iterator>
#include <myDeque/my_deque.hpp>

namespace Deque {
    template <typename T, size_t CHUNK_SIZE = 8, bool IsConst = false>
    struct DequeIterator {
        static_assert(CHUNK_SIZE > 0, "CHUNK_SIZE must be greater than zero");
    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using block_pointer = std::conditional_t<IsConst, const T* const*, T**>;

    private:
        pointer current = nullptr;
        pointer first = nullptr;
        pointer last = nullptr;
        block_pointer block = nullptr;

        size_t size_elem_map_ = CHUNK_SIZE;
        size_t first_index_i = 0;
        size_t first_index_j = 0;
    public:
        template <typename U, typename Alloc, size_t CS> 
        friend class MyDeque;
        template <typename U, size_t CS, bool BC> 
        friend struct DequeIterator;

    private:
        DequeIterator(pointer current_elem, block_pointer map_node, size_t index_i, size_t index_j);
    public:
        DequeIterator() = default;
        DequeIterator(const DequeIterator & other) = default;
        DequeIterator(DequeIterator && other) noexcept = default;
        DequeIterator& operator=(const DequeIterator & other) = default;
        DequeIterator& operator=(DequeIterator && other) noexcept = default;
        ~DequeIterator() = default;

        template <bool OtherConst, typename = std::enable_if_t<IsConst && !OtherConst>>
        DequeIterator(const DequeIterator<T, CHUNK_SIZE, OtherConst>& other);
        template <bool OtherConst, typename = std::enable_if_t<IsConst && !OtherConst>>
        DequeIterator& operator=(const DequeIterator<T, CHUNK_SIZE, OtherConst>& other);

        void setBlock(block_pointer newNode, size_t index_i, size_t index_j = 0);
        DequeIterator& operator++ ();
        DequeIterator& operator-- ();
        DequeIterator operator++ (int);
        DequeIterator operator-- (int);
        DequeIterator operator+ (difference_type index) const;
        DequeIterator operator- (difference_type index) const;
        DequeIterator& operator+= (difference_type index);
        DequeIterator& operator-= (difference_type index);

        template <bool OtherConst>
        bool operator== (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const;
        template <bool OtherConst>
        bool operator!= (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const;
        template <bool OtherConst>
        bool operator< (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const;
        template <bool OtherConst>
        std::strong_ordering operator<=> (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const noexcept;
        
        template <bool OtherConst>
        difference_type operator- (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const;

        bool operator>  (const DequeIterator& rhs) const { return rhs < *this; }
        bool operator<= (const DequeIterator& rhs) const { return !(*this > rhs); }
        bool operator>= (const DequeIterator& rhs) const { return !(*this < rhs); }

        reference operator* () const { return *current; }
        pointer operator-> () const { return current; }
        reference operator[] (size_t index) const {return *(*this + index);}

        size_t getFirstI() { return first_index_i; }
        size_t getFirstJ() { return first_index_j; }
    };

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst>::DequeIterator(pointer current_elem, block_pointer map_node, size_t index_i, size_t index_j) {
        setBlock(map_node, index_i);
        current = current_elem;
        first_index_j = index_j;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    template <bool OtherConst, typename Enable>
    DequeIterator<T, CHUNK_SIZE, IsConst>::DequeIterator(const DequeIterator<T, CHUNK_SIZE, OtherConst>& other) 
    {
        current = other.current;
        first = other.first;
        last = other.last;
        block = other.block; 
        first_index_i = other.first_index_i;
        first_index_j = other.first_index_j;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    template <bool OtherConst, typename Enable>
    DequeIterator<T, CHUNK_SIZE, IsConst>& 
            DequeIterator<T, CHUNK_SIZE, IsConst>::operator=(const DequeIterator<T, CHUNK_SIZE, OtherConst>& other)
    {
        current = other.current;
        first = other.first;
        last = other.last;
        block = other.block;
        first_index_i = other.first_index_i;
        first_index_j = other.first_index_j;
        return *this;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    void DequeIterator<T, CHUNK_SIZE, IsConst>::setBlock(block_pointer newNode, size_t index_i, size_t index_j) {
        block = newNode;
        first_index_i = index_i;
        first_index_j = index_j;
        if (block != nullptr && *block != nullptr) {
            first = *newNode;
            last = first + CHUNK_SIZE;
            current = first + index_j; 
        }
        else if (block != nullptr) {
            first = nullptr;
            last = nullptr;
            current = nullptr;
        }
        else {
            first = last = nullptr;
            current = nullptr;
        }
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst>& DequeIterator<T, CHUNK_SIZE, IsConst>::operator++ () {
        ++current;
        ++first_index_j;
        if (current == last) {
            ++first_index_i;
            setBlock(block + 1, first_index_i);
            current = first;
            first_index_j = 0;
        }
        return *this;
    }
    
    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst>& DequeIterator<T, CHUNK_SIZE, IsConst>::operator-- () {
        if (current == first) {
            first_index_i--;
            setBlock(block - 1, first_index_i);
            current = last;
            first_index_j = CHUNK_SIZE;
        }
        current--;
        first_index_j--;
        return *this;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst> DequeIterator<T, CHUNK_SIZE, IsConst>::operator++ (int) {
        DequeIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst> DequeIterator<T, CHUNK_SIZE, IsConst>::operator-- (int) {
        DequeIterator tmp = *this;
        --(*this);
        return tmp;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst> DequeIterator<T, CHUNK_SIZE, IsConst>::operator+ (difference_type index) const {
        DequeIterator tmp = *this;

        difference_type absolute_j = static_cast<difference_type>(tmp.first_index_j) + index;

        difference_type now_index_i = 0;
        difference_type now_index_j = 0;

        if (absolute_j >= 0) {
            now_index_i = absolute_j / static_cast<difference_type>(CHUNK_SIZE);
            now_index_j = absolute_j % static_cast<difference_type>(CHUNK_SIZE);
        }
        else {
            now_index_i = (absolute_j - static_cast<difference_type>(CHUNK_SIZE) + 1) / static_cast<difference_type>(CHUNK_SIZE);
            now_index_j = absolute_j - now_index_i * static_cast<difference_type>(CHUNK_SIZE);
        }

        tmp.first_index_i = static_cast<size_t>(static_cast<difference_type>(tmp.first_index_i) + now_index_i);
        tmp.setBlock(tmp.block + now_index_i, tmp.first_index_i, static_cast<size_t>(now_index_j));

        return tmp;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst> DequeIterator<T, CHUNK_SIZE, IsConst>::operator- (difference_type index) const {
        return *this + (-index);
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst>& DequeIterator<T, CHUNK_SIZE, IsConst>::operator+= (difference_type index) {
        *this = *this + index;
        return *this;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    DequeIterator<T, CHUNK_SIZE, IsConst>& DequeIterator<T, CHUNK_SIZE, IsConst>::operator-= (difference_type index) {
        *this = *this - index;
        return *this;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    template <bool OtherConst>
    bool DequeIterator<T, CHUNK_SIZE, IsConst>::operator== (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const {
        return first_index_i == rhs.first_index_i && first_index_j == rhs.first_index_j;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    template <bool OtherConst>
    bool DequeIterator<T, CHUNK_SIZE, IsConst>::operator!= (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const {
        return !(*this == rhs);
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    template <bool OtherConst>
    bool DequeIterator<T, CHUNK_SIZE, IsConst>::operator< (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const {
        if (first_index_i != rhs.first_index_i) return first_index_i < rhs.first_index_i;
        return first_index_j < rhs.first_index_j;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    template <bool OtherConst>
    std::strong_ordering DequeIterator<T, CHUNK_SIZE, IsConst>::operator<=> (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const noexcept {
        if (auto cmp = first_index_i <=> rhs.first_index_i; cmp != 0) return cmp;
        return first_index_j <=> rhs.first_index_j;
    }

    template <typename T, size_t CHUNK_SIZE, bool IsConst>
    template <bool OtherConst>
    typename DequeIterator<T, CHUNK_SIZE, IsConst>::difference_type 
        DequeIterator<T, CHUNK_SIZE, IsConst>::operator- (const DequeIterator<T, CHUNK_SIZE, OtherConst>& rhs) const 
    {
            difference_type blocks_dist = static_cast<difference_type>(first_index_i) - rhs.first_index_i;
            return blocks_dist * CHUNK_SIZE + (static_cast<difference_type>(first_index_j) - rhs.first_index_j);
    }
}
