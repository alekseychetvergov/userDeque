#pragma once

#include <gtest/gtest.h>
#include <myDeque/my_deque.hpp>
#include <my_memory/my_allocator.hpp>
#include <tech/helper_file.hpp>

#include <iostream>
#include <algorithm>
#include <iterator>

TEST(MyDequeIteratorTest, SetBlockUpdatesOffsetForUnallocatedBlock) {
    using Iterator = Deque::DequeIterator<int, 4>;
    int* map[2] = {};
    Iterator it;

    it.setBlock(&map[1], 1, 3);
    EXPECT_EQ(it.getFirstI(), 1);
    EXPECT_EQ(it.getFirstJ(), 3);

    it.setBlock(nullptr, 0, 2);
    EXPECT_EQ(it.getFirstJ(), 2);
}

TEST(MyDequeIteratorTest, CopyMoveConstrAndAssignmIterator) {
    using MyDequeBuff = Deque::MyDequeBuff<float>;
    using Iterator = Deque::DequeIterator<float>;

    MyDequeBuff myBuff(4);
    auto&& map = myBuff.getMap();
    const size_t sizeMap = myBuff.getSizeMap();
    const size_t sizeElemMap = myBuff.getSizeElemMap();
    for (size_t i = 0; i < sizeMap; ++i)
        map[i] = myBuff.allocate_block();

    for (size_t i = 0; i < sizeMap; ++i) {
        if (map[i] != nullptr) {
            for (size_t j = 0; j < sizeElemMap; ++j)
                Deque::construct(&map[i][j], static_cast<float>(sizeElemMap * i + j));
        }
    }
    Iterator begin1{};
    if (map[0] != nullptr)
        begin1.setBlock(&map[0], 0);
    EXPECT_EQ(*begin1, 0);
    begin1++;
    Iterator begin2(begin1);
    EXPECT_EQ(*begin2, 1);
    begin2++;
    Iterator begin3(std::move (begin2));
    EXPECT_EQ(*begin2, 2); // несмотря на перемещение, продолжает указывать на 3-й элемент
    EXPECT_EQ(*begin3, 2);
    begin2 = std::move(begin1);
    EXPECT_EQ(*begin1, 1); // несмотря на перемещение, продолжает указывать на 2-й элемент
    EXPECT_EQ(*begin2, 1);
}

TEST(MyDequeIteratorTest, CopyMoveConstrAndAssignmConstIterator) {
    const constexpr int sizeElemMap = 5;

    using Controllable = userClass::Controllable;
    using MyDequeBuff = Deque::MyDequeBuff<Controllable, std::allocator<Controllable>, sizeElemMap>;
    using ConstIterator = Deque::DequeIterator<Controllable, sizeElemMap, true>;

    MyDequeBuff myBuff(2);
    auto&& map = myBuff.getMap();
    const size_t sizeMap = myBuff.getSizeMap();
    for (size_t i = 0; i < sizeMap; ++i)
        map[i] = myBuff.allocate_block();

    for (size_t i = 0; i < sizeMap; ++i) {
        if (map[i] != nullptr) {
            for (size_t j = 0; j < sizeElemMap; ++j) {
                Deque::construct(&map[i][j],
                    static_cast<int>(sizeElemMap * i + j),
                    static_cast<float>(sizeElemMap * i + j),
                    static_cast<std::string> (std::string{ "element" } + std::to_string(sizeElemMap * i + j)));
            }
        }
    }
    ConstIterator cbegin1{};
    if (map[0] != nullptr)
        cbegin1.setBlock(&map[0], 0);
    EXPECT_EQ(*cbegin1, Controllable( 0, 0.f, std::string("element0") ));
    ConstIterator cbegin2(cbegin1);
    cbegin2 += 5;
    EXPECT_EQ(*cbegin2, Controllable(5, 5.f, std::string("element5")));
    cbegin2--;
    ConstIterator cbegin3(std::move(cbegin2));
    EXPECT_EQ(*cbegin2, Controllable(4, 4.f, std::string("element4"))); // несмотря на перемещение, продолжает указывать на 3-й элемент
    EXPECT_EQ(*cbegin3, Controllable(4, 4.f, std::string("element4")));
    cbegin1++; cbegin1++; cbegin1++;
    cbegin2 = std::move(cbegin1);
    EXPECT_EQ(*cbegin1, Controllable(3, 3.f, std::string("element3"))); // несмотря на перемещение, продолжает указывать на 2-й элемент
    EXPECT_EQ(*cbegin2, Controllable(3, 3.f, std::string("element3")));
}

TEST(MyDequeIteratorTest, CrossIteratorCopyConstrAndAssignm) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyDequeBuff = Deque::MyDequeBuff<Controllable, MyPmrAllocator, sizeElemMap>;
    using ConstIterator = Deque::DequeIterator<Controllable, sizeElemMap, true>;
    using Iterator = Deque::DequeIterator<Controllable, sizeElemMap>;

    MyDequeBuff myBuff(3);
    auto&& map = myBuff.getMap();
    const size_t sizeMap = myBuff.getSizeMap();
    for (size_t i = 0; i < sizeMap; ++i)
        map[i] = myBuff.allocate_block();

    for (size_t i = 0; i < sizeMap; ++i) {
        if (map[i] != nullptr) {
            for (size_t j = 0; j < sizeElemMap; ++j) {
                Deque::construct(&map[i][j],
                    static_cast<int>(sizeElemMap * i + j),
                    static_cast<float>(sizeElemMap * i + j),
                    static_cast<std::string> (std::string{ "element" } + std::to_string(sizeElemMap * i + j)));
            }
        }
    }
    Iterator begin1{};
    if (map[0] != nullptr)
        begin1.setBlock(&map[0], 0);
    EXPECT_EQ(*begin1, Controllable(0, 0.f, std::string("element0")));
    begin1 += 16;
    ConstIterator cbegin1(begin1); // Проверяем копирующий конструктор из не-const итератора в const
    EXPECT_EQ(*cbegin1, Controllable(16, 16.f, std::string("element16")));
    ConstIterator cbegin2{};
    cbegin2 = cbegin1; // Проверяем копирующее присваивание из не-const итератора в const
    cbegin2--; --cbegin2;
    EXPECT_EQ(*(cbegin1.operator->()), Controllable(16, 16.f, std::string("element16")));
    EXPECT_EQ(*(cbegin2.operator->()), Controllable(14, 14.f, std::string("element14")));

    constexpr bool is_assignable = std::is_assignable_v<Iterator&, ConstIterator&>; // false (копирующее присваивание из const итератора в не-const)
    constexpr bool is_assignable1 = std::is_assignable_v<ConstIterator&, const Iterator&>;
    constexpr bool can_construct = std::is_constructible_v<Iterator, const ConstIterator&>; // false (копирующий конструктор из const итератора в не-const)
    constexpr bool can_construct1 = std::is_constructible_v<const ConstIterator, const Iterator&>;

    EXPECT_FALSE(is_assignable);
    EXPECT_TRUE(is_assignable1);
    EXPECT_FALSE(can_construct);
    EXPECT_TRUE(can_construct1);
}

TEST(MyDequeIteratorTest, PrePostIncrimDecremAndDiffIter) {
    const constexpr int sizeElemMap = 11;

    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<int>;
    using MyDeque = Deque::MyDeque<int, MyPmrAllocator, sizeElemMap>;
    using constIteratorInt = Deque::DequeIterator<int, sizeElemMap, true>;
    using constReverseIterInt = std::reverse_iterator<constIteratorInt>;

    MyDeque deq1(30);
    std::ranges::transform(deq1, std::rbegin(deq1), [](auto&& it) {static int r = 0; return ++r;});

    constIteratorInt cbeginIt1 = std::cbegin(deq1) + 3;
    constIteratorInt cbeginIt2 = std::cbegin(deq1) + 3;
    EXPECT_TRUE(cbeginIt1 == cbeginIt2);
    cbeginIt1++;
    cbeginIt2--;
    EXPECT_FALSE(cbeginIt1 == cbeginIt2);
    EXPECT_EQ(*cbeginIt1, 26);
    EXPECT_EQ(*cbeginIt2, 28);
    --cbeginIt1;
    ++cbeginIt2;
    EXPECT_TRUE(cbeginIt1 == cbeginIt2);

    constReverseIterInt crbeginIt1 = std::crbegin(deq1) + 4;
    constReverseIterInt crbeginIt2 = std::crbegin(deq1) + 4;
    EXPECT_TRUE(crbeginIt1 == crbeginIt2);
    crbeginIt1++;
    crbeginIt2--;
    EXPECT_FALSE(crbeginIt1 == crbeginIt2);
    EXPECT_EQ(*crbeginIt1, 6);
    EXPECT_EQ(*crbeginIt2, 4);
    --crbeginIt1;
    ++crbeginIt2;
    EXPECT_TRUE(crbeginIt1 == crbeginIt2);

    EXPECT_TRUE(crbeginIt1.base() - cbeginIt1 == 23); // Разница между const_reverce_iteranor и const_iteranor 
    EXPECT_TRUE(crbeginIt1 - std::rbegin(deq1) == 4); // Разница между const_reverce_iteranor и reverce_iteranor 
    EXPECT_TRUE(std::end(deq1) - cbeginIt1 == 27); // Разница между iteranor и const_iteranor
    EXPECT_TRUE(std::begin(deq1) - std::rbegin(deq1).base() == -30); // Разница между iteranor и reverce_iteranor
    //std::ranges::copy(deq1, std::ostream_iterator<int>(std::cout, " "));
} 

TEST(MyDequeIteratorTest, SummDiffIterIndexEqual) {
    const constexpr int sizeElemMap = 8;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyDeque = Deque::MyDeque<Controllable, MyPmrAllocator>;

    using iteratorContr = Deque::DequeIterator<Controllable>;
    using constIteratorContr = Deque::DequeIterator<Controllable, sizeElemMap, true>;
    using reverseIteratorContr = std::reverse_iterator<iteratorContr>;
    using constReverseIteratorContr = std::reverse_iterator<constIteratorContr>;

    MyDeque deq1(10);

    std::ranges::transform(deq1, std::rbegin(deq1), [](auto&& it) {static int r = 0;  r++;
    return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        });

    iteratorContr beginIt = std::begin(deq1) + 5;
    constIteratorContr cbeginIt = std::cbegin(deq1) + 5;
    reverseIteratorContr rbeginIt = std::rbegin(deq1) + 5;
    constReverseIteratorContr crbeginIt = std::crbegin(deq1) + 5;

    EXPECT_TRUE(crbeginIt + 3 == rbeginIt + 3);
    EXPECT_EQ(*(crbeginIt + 3), Controllable(9, 9.f, std::string("element9")));
    EXPECT_TRUE(cbeginIt - 2 == beginIt - 2);
    EXPECT_EQ(*(cbeginIt - 2), Controllable(7, 7.f, std::string("element7")));

    EXPECT_TRUE( (beginIt >= cbeginIt) && (beginIt <= cbeginIt));
    EXPECT_FALSE( beginIt < cbeginIt || beginIt > cbeginIt);
    EXPECT_TRUE((rbeginIt >= crbeginIt) && (rbeginIt <= crbeginIt));
    EXPECT_FALSE(rbeginIt < crbeginIt || rbeginIt > crbeginIt);

    crbeginIt += 3;
    rbeginIt -= 3;
    beginIt += 2;
    cbeginIt -= 2;

    EXPECT_TRUE((beginIt != cbeginIt) && (rbeginIt != crbeginIt));
    EXPECT_TRUE((beginIt > cbeginIt) && (cbeginIt < beginIt));
    EXPECT_TRUE((beginIt > (beginIt - 1) && (cbeginIt < cbeginIt + 1)));
    EXPECT_TRUE((rbeginIt < crbeginIt) && (crbeginIt > rbeginIt));
    EXPECT_TRUE((rbeginIt > (rbeginIt - 2) && (crbeginIt < crbeginIt + 2)));
}

TEST(MyDequeIteratorTest, referencePointerDistance) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyDeque = Deque::MyDeque<Controllable, MyPmrAllocator, sizeElemMap>;

    using iteratorContr = Deque::DequeIterator<Controllable, sizeElemMap>;
    using constIteratorContr = Deque::DequeIterator<Controllable, sizeElemMap, true>;
    using reverseIteratorContr = std::reverse_iterator<iteratorContr>;
    using constReverseIteratorContr = std::reverse_iterator<constIteratorContr>;

    MyDeque deq1(15);

    std::ranges::transform(deq1, std::begin(deq1), [](auto&& it) {static int r = 0;  r++;
    return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        });

    iteratorContr beginIt = std::begin(deq1);
    constIteratorContr cbeginIt = std::cbegin(deq1) + 8;
    reverseIteratorContr rbeginIt = std::rbegin(deq1);
    constReverseIteratorContr crbeginIt = std::crbegin(deq1) + 7;

    EXPECT_TRUE(beginIt == cbeginIt - 8);   // проверили сравнение не-const и const
    EXPECT_TRUE(rbeginIt == crbeginIt - 7); // проверили сравнение не-const reverce и const reverce
    EXPECT_TRUE(*(crbeginIt - 1) == cbeginIt[0]);

    *rbeginIt = rbeginIt[5]; // присваиваем в разыменованный 1 элемент с права значение 5 элемента справа через []
    *beginIt = beginIt[7];   // присваиваем в разыменованный 1 элемент значение 7 элемента через []

    EXPECT_EQ(*(beginIt.operator->()), beginIt[7]); // так же проверяем работу operator->()
    EXPECT_EQ(beginIt[7], Controllable(8, 8.f, std::string("element8")));
    EXPECT_EQ(*(rbeginIt.operator->()), rbeginIt[5]); // так же проверяем работу operator->()
    EXPECT_EQ(rbeginIt[5], Controllable(10, 10.f, std::string("element10")));

    rbeginIt [3] = crbeginIt[2]; // присваиваем в 3 элемент с конца 9 (7+2) элемент с конца из const_reverce_iterator
    beginIt [4] = cbeginIt[1]; // присваиваем в 4 элемент 9 (8+1) элемент с начала из const_iterator

    EXPECT_TRUE(*(beginIt + 4) == *(std::cbegin(deq1) + 9)); 
    EXPECT_EQ(beginIt[4], Controllable(10, 10.f, std::string("element10")));
    EXPECT_TRUE(beginIt[4] == cbeginIt[1]);
    // сравниваем значение 4 слева и 10 справа (одно и то же) по разнонаправленным и разноконстантным итераторам
    EXPECT_TRUE(*((beginIt + 4).operator->()) == *(std::crbegin(deq1) + 10));

    EXPECT_TRUE(*(rbeginIt + 3) == *(std::crbegin(deq1) + 9)); 
    EXPECT_EQ(rbeginIt[3], Controllable(6, 6.f, std::string("element6"))); 
    EXPECT_TRUE(rbeginIt[3] == crbeginIt[2]);
    // сравниваем значение 11 слева и 3 справа (одно и то же) по разнонаправленным и разноконстантным итераторам
    EXPECT_TRUE(*((rbeginIt + 3).operator->()) == *(std::cbegin(deq1) + 11));
}