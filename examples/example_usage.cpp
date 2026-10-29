// myDeque_Final.cpp: определяет точку входа для приложения.
//
#pragma once

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <memory>
#include <utility>
#include <new>
#include <string>
#include <algorithm>
#include <ranges>
#include <vector>

//#define PRINT_TECH_1
//#define PRINT_TECH_2
//#define PRINT_TECH_ALLOCATION
#define PRINT_DEQUE_ARRAY_2
#define PRINT_DEQUE_FIELD_2

#include <my_memory/my_allocator.hpp>
#include <myDeque/my_deque.hpp>
#include <tech/helper_file.hpp>


template <typename T>
using MySimpleAllocator = SimpleAlloc::MySimpleAllocator<T>;
template <typename T>
using MyPmrAllocator = PmrAllocator::MyPmrAllocator <T>;

template <typename T, typename Allocator = std::allocator<T>, size_t SIZE_ELEM_MAP = 8>
using MyDequeBuff = Deque::MyDequeBuff <T, Allocator, SIZE_ELEM_MAP>;
template <typename T, size_t SIZE_ELEM_MAP = 8>
using DequeIterator = Deque::DequeIterator <T, SIZE_ELEM_MAP>;
template <typename T, typename Allocator = std::allocator<T>, size_t SIZE_ELEM_MAP = 8>
using MyDeque = Deque::MyDeque <T, Allocator, SIZE_ELEM_MAP>;

namespace constrDestr {
    template <typename T> void construct(T* p, const T& rhs) {
        new (p) T(rhs);
    }

    template <typename T, typename ... Args> void construct(T* p, Args ... args) {
        new (p) T(std::forward<Args>(args)...);
    }

    template <class T>
    void destroy(T* p) {
        p->~T();
    }

    template <typename DequeIter>
    void destroy(DequeIter first, DequeIter last) {
        while (first != last) {
            destroy(&*first);
            ++first;
        }
    }
}

struct Controllable {
    int i;
    float f;
    std::string s;

    Controllable() : i(0), f(0.f), s("0") {}
    Controllable(int i_, float f_, std::string s_) : i(i_), f(f_), s(s_) {}
};

template <typename T, typename Allocator = std::allocator<T>, size_t CHUNK_SIZE = 8>
void printDequeArray(const MyDeque<int, Allocator, CHUNK_SIZE>& deque1) {
    for (size_t i = 0; i < deque1.size_map_; ++i) {
        if (deque1.map_[i] != nullptr) {
            for (size_t j = 0; j < deque1.size_elem_map_; ++j) {
                if (deque1.map_[i] != nullptr)
                    std::cout << deque1.map_[i][j] << "\t";
                else
                    std::cout << "NULL\t";
            }
            std::cout << "\n";
        }
        else {
            for (size_t j = 0; j < deque1.size_elem_map_; ++j)
                std::cout << "NULL\t";
            std::cout << "\n";
        }
    } std::cout << "\n";
}
template <typename T, typename Allocator = std::allocator<T>, size_t CHUNK_SIZE = 8>
void printDequeField(const MyDeque<int, Allocator, CHUNK_SIZE>& deque1) {
    std::cout << "size_ = " << deque1.size_ << ", size_map_ = " << deque1.size_map_ << ", capasity_ = " << deque1.capasity_
        << ", first_block_ = " << deque1.first_block_ << ", last_block_ = " << deque1.last_block_ << "\n";
    std::cout << "start (" << deque1.start.first_index_i << ", " << deque1.start.first_index_j << ") "
        << "and finish (" << deque1.finish.first_index_i << ", " << deque1.finish.first_index_j << "); last_index ("
        << deque1.last_index_i << ", " << deque1.last_index_j << ") \n";
}

void plasementNew1() {
    void* raw_memory = ::operator new(sizeof(Controllable));        // Создаем сырую память
    Controllable* ptr1 = static_cast<Controllable*>(raw_memory);    // Создаем указатель на нужный нам объект в созданую сырую память

    Controllable staticContr{ 3, 4.5f, "Alexey" };
    constrDestr::construct(ptr1, staticContr);                                   // конструируем в сырой памяти наш объект
    std::cout << ptr1->i << " " << ptr1->f << " " << ptr1->s << " " << "\n";
    constrDestr::destroy(ptr1);                                                  // уничтожем наш объект
    ::operator delete(raw_memory);                                  // очищаем сырую память

    void* raw_memory1 = ::operator new(sizeof(Controllable));
    Controllable* ptr2 = static_cast<Controllable*>(raw_memory1);
    constrDestr::construct(ptr2, 4, 6.9f, "Ivan");
    std::cout << ptr2->i << " " << ptr2->f << " " << ptr2->s << " " << "\n";
    constrDestr::destroy(ptr2);
    ::operator delete(raw_memory1);
    std::cout << "\n";
}
void MyDequeBuff1() {
    MyDequeBuff<float, MyPmrAllocator<float>, 10> myBuff(4);

    for (size_t i = 0; i < myBuff.size_map_; ++i)
        myBuff.map_[i] = myBuff.allocate_block();

    for (size_t i = 0; i < myBuff.size_map_; ++i) {
        if (myBuff.map_[i] != nullptr) {
            for (size_t j = 0; j < myBuff.size_elem_map_; ++j) {
                constrDestr::construct(&myBuff.map_[i][j], static_cast<int>(myBuff.size_elem_map_ * i + j));
#ifdef PRINT_DEQUE_ARRAY_2
                std::cout << myBuff.map_[i][j] << " ";
#endif // PRINT_DEQUE_ARRAY_2
            }
#ifdef PRINT_DEQUE_ARRAY_2
            std::cout << "\n";
#endif // PRINT_DEQUE_ARRAY_2
        }
    }

    for (size_t i = 0; i < myBuff.size_map_; ++i) {
        if (myBuff.map_[i] != nullptr) {
            for (size_t j = 0; j < myBuff.size_elem_map_; ++j) {
                constrDestr::destroy(&myBuff.map_[i][j]);
            }
        }
    } std::cout << "\n";
}
void MyDequeBuff2() {
    MyDequeBuff<Controllable, MyPmrAllocator<Controllable>, 12> myBuff(4);

    for (size_t i = 0; i < myBuff.size_map_; ++i)
        myBuff.map_[i] = myBuff.allocate_block();

    for (size_t i = 0; i < myBuff.size_map_; ++i) {
        if (myBuff.map_[i] != nullptr) {
            for (size_t j = 0; j < myBuff.size_elem_map_; ++j) {
                constrDestr::construct(&myBuff.map_[i][j],
                    static_cast<int>(myBuff.size_elem_map_ * i + j),
                    static_cast<float>(myBuff.size_elem_map_ * i + j),
                    std::string{ "Aleksey" } + std::to_string(myBuff.size_elem_map_ * i + j)
                );
#ifdef PRINT_DEQUE_ARRAY_2
                std::cout << myBuff.map_[i][j].i << " " << myBuff.map_[i][j].f << " " << myBuff.map_[i][j].s << "\n";
#endif // PRINT_DEQUE_ARRAY_2
            }
#ifdef PRINT_DEQUE_ARRAY_2
            std::cout << "\n";
#endif // PRINT_DEQUE_ARRAY_2
        }
    }
    for (size_t i = 0; i < myBuff.size_map_; ++i) {
        if (myBuff.map_[i] != nullptr) {
            for (size_t j = 0; j < myBuff.size_elem_map_; ++j) {
                constrDestr::destroy(&myBuff.map_[i][j]);
            }
        }
    } std::cout << "\n";
}
void DequeIterator1() {
    constexpr size_t matrix_size = 11;
    MyDequeBuff<int, MyPmrAllocator<int>, matrix_size> myBuff(4);

    for (size_t i = 0; i < myBuff.size_map_; ++i)
        myBuff.map_[i] = myBuff.allocate_block();
    size_t ii = 1;
    for (size_t i = ii; i < myBuff.size_map_; ++i) {
        if (myBuff.map_[i] != nullptr) {
            for (size_t j = 0; j < myBuff.size_elem_map_; ++j)
                constrDestr::construct(&myBuff.map_[i][j], static_cast<int>(myBuff.size_elem_map_ * i + j));
        }
    }
    DequeIterator<int, matrix_size> begin1{};
    if (myBuff.map_[ii] != nullptr)
        begin1.setBlock(&myBuff.map_[ii], ii);
    size_t count = (myBuff.size_map_ - ii) * myBuff.size_elem_map_;
    for (size_t i = 0; i < count; ++i, ++begin1) {
#ifdef PRINT_DEQUE_ARRAY_2
        std::cout << *begin1 << "(" << begin1.first_index_i << ", " << begin1.first_index_j << ");  ";
#endif // PRINT_DEQUE_ARRAY_2
    }

    for (size_t i = 0; i < myBuff.size_map_; ++i) {
        if (myBuff.map_[i] != nullptr) {
            for (size_t j = 0; j < myBuff.size_elem_map_; ++j)
                constrDestr::destroy(&myBuff.map_[i][j]);
        }
    } std::cout << "\n" << "\n";
}

void Deque1() {
    MyDeque<int, MyPmrAllocator<int>, 12> deque1(31, 66);
    //std::cout << "size_elem_map_ = " << deque1.size_elem_map_ << '\n';
    std::cout << "size_map_ = " << deque1.size_map_ << '\n';
    std::cout << "capasity_ = " << deque1.capasity_ << '\n';
    std::cout << "size_ = " << deque1.size_ << '\n';

    deque1[1] = 55;
    deque1[27] = 77;
    deque1[30] = 99;
    std::cout << " " << deque1.start.first_index_i << " " << deque1.start.first_index_j << "\n";
    std::cout << " " << deque1.finish.first_index_i << " " << deque1.finish.first_index_j << "\n";
    for (size_t i = 0; i < deque1.size_map_; ++i) {
        if (deque1.map_[i] != nullptr) {
            for (size_t j = 0; j < deque1.size_elem_map_; ++j) {
                if (deque1.map_[i] != nullptr) {
#ifdef PRINT_DEQUE_ARRAY_2
                    std::cout << deque1.map_[i][j] << "\t";
#endif // PRINT_DEQUE_ARRAY_2
                }
                else {
#ifdef PRINT_DEQUE_ARRAY_2
                    std::cout << "NULL\t";
#endif // PRINT_DEQUE_ARRAY_2
                }
            }
#ifdef PRINT_DEQUE_ARRAY_2
            std::cout << "\n";
#endif // PRINT_DEQUE_ARRAY_2
        }
        else {
            for (size_t j = 0; j < deque1.size_elem_map_; ++j) {
#ifdef PRINT_DEQUE_ARRAY_2
                std::cout << "NULL\t";
#endif // PRINT_DEQUE_ARRAY_2
            }
#ifdef PRINT_DEQUE_ARRAY_2
            std::cout << "\n";
#endif // PRINT_DEQUE_ARRAY_2
        }
    } std::cout << "\n";
}
void Deque2() {
    MyDeque<int, MyPmrAllocator<int>> deque1(29, 22);
    deque1[1] = 55;
    deque1[27] = 77;
    deque1[28] = 99;

#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2

    deque1.push_back(11); deque1.push_back(33); deque1.push_back(44); deque1.push_back(55); deque1.push_back(56);
    deque1.push_back(66); deque1.push_back(77); deque1.push_back(88); //deque1.push_back(75); 

#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2

    //deque1.realocate(65);
    deque1.push_back(89); deque1.push_back(95);

#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
    std::cout << "\n";
}
void Deque3() {
    MyDeque<int, MyPmrAllocator<int>, 6> deque1(23, 22);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    std::cout << "\n";
    deque1.push_back(11); deque1.push_back(33); deque1.push_back(44); deque1.push_back(55);
    //deque1.push_back(66); deque1.push_back(77); deque1.push_back(88);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    std::cout << "\n";
    deque1.pop_back(); deque1.pop_back(); deque1.pop_back(); deque1.pop_back(); deque1.pop_back();
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
}
void Deque4() {
    MyDeque<int, MyPmrAllocator<int>, 7> deque1(23, 22);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    deque1.resize(6, 88);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    deque1.reserve(100);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    deque1.resize(40, 88);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    deque1.push_back(99); deque1.push_back(99); deque1.push_back(99); deque1.push_back(99); deque1.push_back(99);
    deque1.push_back(99); deque1.push_back(99); deque1.push_back(99); deque1.push_back(99); deque1.push_back(99);
    deque1.push_back(66); deque1.push_back(66); deque1.push_back(66); deque1.push_back(66);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
}
void Deque5() {
    MyDeque<int, MyPmrAllocator<int>, 9> deque1(27, 11);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    for (size_t i = 0; i < 4; ++i) deque1.push_back(66);
    for (size_t i = 0; i < 5; ++i) deque1.push_front(33);
    for (size_t i = 0; i < 9; ++i) deque1.push_front(44);
    for (size_t i = 0; i < 18; ++i) deque1.push_front(99);
    for (size_t i = 0; i < 9; ++i) deque1.push_front(88);
    for (size_t i = 0; i < 1; ++i) deque1.push_front(77);
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    for (size_t i = 0; i < 10; ++i) deque1.pop_front();
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
}
void Deque6() {
    MyDeque<int, MyPmrAllocator<int>, 9> deque1(34, 11);
    auto itBeg = deque1.begin();
    auto itEnd = deque1.end();
    std::transform(itBeg, itEnd, itBeg, [](auto&& it) {static int count = 0; return count++; });

    std::vector<int> vec1{ 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71 };
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2

    //deque1.push_back(std::begin(vec1), std::end(vec1));
    //deque1.push_front(std::begin(vec1), std::end(vec1));
    //deque1.insert(std::begin(deque1) + 8, 50);
    deque1.insert(std::begin(deque1) + 1, std::begin(vec1), std::end(vec1));
#ifdef PRINT_DEQUE_FIELD_2
    printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
    printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
}
void algorithmDeque1() {
    constexpr size_t matrix_size = 14;
    MyDeque<int, MyPmrAllocator<int>, matrix_size> deque1(40);
    auto itBeg = deque1.begin();
    auto itEnd = deque1.end();

    std::transform(itBeg, itEnd, itBeg, [](auto&& it) {static int count = 0; return count++; });
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
    }

    int current = *deque1.begin() - 1;
    std::generate_n(std::front_inserter(deque1), 15, [&current]() mutable {return current--; });
    current = *(deque1.end() - 1) + 1;
    std::generate_n(std::back_inserter(deque1), 15, [&current]() mutable {return current++; });
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
    }

    deque1.push_front(*deque1.begin() - 1);
    deque1.push_back(*(deque1.end() - 1) + 1);
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
    }

    for (int i = 0; i < 6 && deque1.size_ >= 2; ++i) {
        deque1.pop_front();
        deque1.pop_back();
    }
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
    }
}
void algorithmDeque2() {
    constexpr size_t matrix_size = 10;
    MyDeque<int, MyPmrAllocator<int>, matrix_size> deque1(40);
    auto itBeg = deque1.begin();
    auto itEnd = deque1.end();

    std::transform(itBeg, itEnd, itBeg, [](auto&& it) {static int count = 0; return count++; });
    {
        //std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::ranges::copy(deque1, std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
    }
    MyDeque<int, MyPmrAllocator<int>, matrix_size> deque2(deque1.rbegin() + 3, deque1.rend() - 4, MyPmrAllocator<int> {});
    {
        std::copy(deque2.begin(), deque2.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque2);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque2);
#endif // PRINT_DEQUE_ARRAY_2

    }


    auto view = deque1
        | std::views::filter([](int x) { return x % 2 == 0; })
        | std::views::transform([](int x) { return x * 10; })
        | std::views::take(10);

    for (int x : view) {
        std::cout << x << " ";
    }
}
void copyMoveCtorDeque1() {
    std::cout << "CopyCtor: ";
    constexpr size_t matrix_size = 6;
    MyDeque<int, MyPmrAllocator<int>, matrix_size> deque1(20);
    auto itBeg = deque1.begin();
    auto itEnd = deque1.end();
    std::transform(itBeg, itEnd, itBeg, [](auto&& it) {static int count = 0; return count++; });

    auto deque2(deque1);
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
        std::copy(deque2.begin(), deque2.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque2);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque2);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
    }

    std::cout << "MoveCtor: ";
    auto deque3(std::move(deque1));
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
        std::copy(deque3.begin(), deque3.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque3);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque3);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
    }

    std::cout << "initializer_list: ";
    MyDeque<int, MyPmrAllocator<int>, matrix_size> deque4(
        { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 });
    {
        std::copy(deque4.begin(), deque4.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque4);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque4);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
    }

    std::cout << "itherators: ";
    MyDeque<int, MyPmrAllocator<int>, matrix_size> deque5(deque4.begin() + 3, deque4.end() - 4, MyPmrAllocator<int> {});
    {
        std::copy(deque5.begin(), deque5.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque5);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque5);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
    }

    std::cout << "Copy operator= : ";
    deque1 = deque4;
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
        std::copy(deque4.begin(), deque4.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque4);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque4);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
    }

    std::cout << "Move operator= : ";
    deque1 = std::move(deque5);
    {
        std::copy(deque1.begin(), deque1.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque1);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
        std::copy(deque5.begin(), deque5.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
#ifdef PRINT_DEQUE_FIELD_2
        printDequeField<int, MyPmrAllocator<int>, matrix_size>(deque5);
#endif // PRINT_DEQUE_FIELD_2
#ifdef PRINT_DEQUE_ARRAY_2
        printDequeArray<int, MyPmrAllocator<int>, matrix_size>(deque5);
#endif // PRINT_DEQUE_ARRAY_2
        std::cout << "\n";
    }
}

int main()
{
    //plasementNew1();
    //MyDequeBuff1();
    //DequeIterator1();

    //MyDequeBuff2();

    //Deque1();
    //Deque2();
    //Deque3();
    //Deque4();
    //Deque5();
    Deque6();
    //algorithmDeque1(); 
    //algorithmDeque2();
    //copyMoveCtorDeque1();
}
