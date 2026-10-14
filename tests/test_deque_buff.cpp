#pragma once

#include <string>

#include <gtest/gtest.h>
#include <myDeque/my_deque.hpp>
#include <my_memory/my_allocator.hpp>
#include <tech/helper_file.hpp>

TEST(MyDequeBuffTest, SizeConstructor) {
    const constexpr int sizeElemMap = 10;
    Deque::MyDequeBuff<int, PmrAllocator::MyPmrAllocator<int>, sizeElemMap> deqPmr(20);
    Deque::MyDequeBuff<int> deqZero(0);

    EXPECT_NO_THROW(deqPmr);
    EXPECT_THROW( (Deque::MyDequeBuff<int, FaultyAllocator::FaultyAllocator<int>>(10) ), std::bad_alloc);
    EXPECT_EQ(deqPmr.getSizeMap(), 20);
    EXPECT_EQ(deqPmr.getSizeElemMap(), sizeElemMap);
    EXPECT_EQ(deqZero.getSizeMap(), 0);
}

TEST(MyDequeBuffTest, MoveConstrAndAssignm) {
    const constexpr int sizeElemMap = 10;

    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<int>;
    using MyDequeBuff = Deque::MyDequeBuff<int, MyPmrAllocator, sizeElemMap>;

    MyDequeBuff deqPmr1(40);
    MyDequeBuff deqPmr2(30);
    MyDequeBuff deqPmr3(std::move(deqPmr2));
    deqPmr2 = std::move(deqPmr1);
    EXPECT_EQ(deqPmr2.getSizeMap(), 40);
    EXPECT_EQ(deqPmr3.getSizeMap(), 30);
    EXPECT_EQ(deqPmr1.getSizeMap(), 0);
}

TEST(MyDequeBuffTest, AllocateDeallocate) {
    Deque::MyDequeBuff<float, PmrAllocator::MyPmrAllocator<float>, 10> myBuff(4);

    myBuff.getMap()[1] = myBuff.allocate_block();
    myBuff.getMap()[2] = myBuff.allocate_block();
    size_t sizeElem = myBuff.getSizeElemMap();
    for (size_t j = 0; j < sizeElem; ++j) {
        Deque::construct(&myBuff.getMap()[1][j], static_cast<float>(sizeElem + j));
        Deque::construct(&myBuff.getMap()[2][j], static_cast<float>(sizeElem * 2 + j));
    }

    float expectedValues1[10] = { 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f, 17.f, 18.f, 19.f };
    float expectedValues2[10] = { 20.f, 21.f, 22.f, 23.f, 24.f, 25.f, 26.f, 27.f, 28.f, 29.f };

    for (size_t j = 0; j < sizeElem; ++j) {
        EXPECT_EQ(myBuff.getMap()[1][j], expectedValues1[j]);
        EXPECT_EQ(myBuff.getMap()[2][j], expectedValues2[j]);
    }

    for (size_t j = 0; j < sizeElem; ++j) {
        Deque::destroy(&myBuff.getMap()[1][j]);
        Deque::destroy(&myBuff.getMap()[2][j]);
    }
}

TEST(MyDequeBuffTest, SizeAndMoveConstructorUserClass) {
    const constexpr int sizeElemMap = 12;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyDequeBuff = Deque::MyDequeBuff<Controllable, MyPmrAllocator, sizeElemMap>;

    MyDequeBuff deqPmr1(3);
    MyDequeBuff deqPmr2(5);
    MyDequeBuff deqPmr3(std::move(deqPmr2));
    deqPmr2 = std::move(deqPmr1);

    EXPECT_TRUE(deqPmr2.getSizeMap() == 3);
    EXPECT_TRUE(deqPmr3.getSizeMap() == 5);
    EXPECT_TRUE(deqPmr1.getSizeMap() == 0);
}

TEST(MyDequeBuffTest, AllocateDeallocateUserClass) {
    const constexpr int sizeElemMap = 4;
    using Controllable = userClass::Controllable;

    Deque::MyDequeBuff<Controllable, PmrAllocator::MyPmrAllocator<Controllable>, sizeElemMap> myBuff(4);

    myBuff.getMap()[0] = myBuff.allocate_block();
    myBuff.getMap()[3] = myBuff.allocate_block();

    size_t sizeElem = myBuff.getSizeElemMap();
    for (size_t j = 0; j < sizeElem; ++j) {
        Deque::construct(&myBuff.getMap()[0][j], 
                         static_cast<int>(sizeElem + j), 
                         static_cast<float>(sizeElem + j),
                         static_cast<std::string> (std::string{ "element" } + std::to_string(sizeElem + j)));
        Deque::construct(&myBuff.getMap()[3][j],
                         static_cast<int>(sizeElem * 2 + j),
                         static_cast<float>(sizeElem * 2 + j),
                         static_cast<std::string> (std::string{ "element" } + std::to_string(sizeElem * 2 + j)));
    }

    Controllable expectedValues1[4] = { {4, 4.f, "element4"}, {5, 5.f, "element5"}, {6, 6.f, "element6"}, {7, 7.f, "element7"} };
    Controllable expectedValues2[4] = { {8, 8.f, "element8"}, {9, 9.f, "element9"}, {10, 10.f, "element10"}, {11, 11.f, "element11"} };

    for (size_t j = 0; j < sizeElem; ++j) {
        EXPECT_EQ(myBuff.getMap()[0][j], expectedValues1[j]);
        EXPECT_EQ(myBuff.getMap()[3][j], expectedValues2[j]);
    }

    for (size_t j = 0; j < sizeElem; ++j) {
        Deque::destroy(&myBuff.getMap()[0][j]);
        Deque::destroy(&myBuff.getMap()[3][j]);
    }
}

