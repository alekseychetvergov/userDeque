#pragma once

#include <gtest/gtest.h>
#include <myDeque/my_deque.hpp>
#include <my_memory/my_allocator.hpp>
#include <tech/helper_file.hpp>

#include <type_traits>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <iterator>
#include <deque>
#include <vector>
#include <list>
#include <cassert>


TEST(MyDequeConstructorTest, DefaultConstructorMathValue) {
    Deque::MyDeque<int> d{};

    // EXPECT_TRUE(d.size() == 0);
    EXPECT_EQ(d.size(), 0);
}

TEST(MyDequeConstructorTest, SizeConstructorMath) {
    const constexpr int sizeElemMap = 10;

    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<int>;
    using MyDeque = Deque::MyDeque<int, MyPmrAllocator, sizeElemMap>;

    MyDeque deq1(0);
    MyDeque deq2(1);
    MyDeque deq3(10);
    MyDeque deq4(22);
    MyDeque deq5(80);
    MyDeque deq6(509);

    EXPECT_TRUE(deq1.size() == 0);            // размер
    EXPECT_TRUE(deq1.capasity() == 30);       // вместимость
    EXPECT_TRUE(deq1.getSizeMap() == 3);      // количество блоков
    EXPECT_TRUE(deq1.getFirstBlock() == 1);   // первый блок 
    EXPECT_TRUE(deq1.getLastBlock() == 1);    // последний блок

    EXPECT_TRUE(deq2.size() == 1);
    EXPECT_TRUE(deq2.capasity() == 30);
    EXPECT_TRUE(deq2.getSizeMap() == 3);
    EXPECT_TRUE(deq2.getFirstBlock() == 1);
    EXPECT_TRUE(deq2.getLastBlock() == 1);

    EXPECT_TRUE(deq3.size() == 10);
    EXPECT_TRUE(deq3.capasity() == 40);
    EXPECT_TRUE(deq3.getSizeMap() == 4);
    EXPECT_TRUE(deq2.getFirstBlock() == 1);
    EXPECT_TRUE(deq3.getLastBlock() == 2);

    EXPECT_TRUE(deq4.size() == 22);
    EXPECT_TRUE(deq4.capasity() == 50);
    EXPECT_TRUE(deq4.getSizeMap() == 5);
    EXPECT_TRUE(deq4.getFirstBlock() == 1);
    EXPECT_TRUE(deq4.getLastBlock() == 3);

    EXPECT_TRUE(deq5.size() == 80);
    EXPECT_TRUE(deq5.capasity() == 160);
    EXPECT_TRUE(deq5.getSizeMap() == 16);
    EXPECT_TRUE(deq5.getFirstBlock() == 4);
    EXPECT_TRUE(deq5.getLastBlock() == 11);

    EXPECT_TRUE(deq6.size() == 509);
    EXPECT_TRUE(deq6.capasity() == 1000);
    EXPECT_TRUE(deq6.getSizeMap() == 100);
    EXPECT_TRUE(deq6.getFirstBlock() == 24);
    EXPECT_TRUE(deq6.getLastBlock() == 75);

    //std::cout << "1 - " << deq1.capasity() << " , 2 - " << deq2.capasity() << " , 3 - " << deq3.capasity() << " , 4 - "
    //    << deq4.capasity() << " , 5 - " << deq5.capasity() << " , 6 - " << deq6.capasity() << "\n";
}

TEST(MyDequeConstructorTest, SizeConstructorAllocate) {
    const constexpr int sizeElemMap = 10;

    using Controllable = userClass::Controllable;
    using MockMemoryResource = FaultyAllocator::MockMemoryResource;               // Аллокатор считающий память
    using FaultyAllocator = FaultyAllocator::FaultyAllocator <Controllable>;      // Аллокатор выбрасывающий "std::bad_alloc"
    using ThrowingElement = userClass::ThrowingElement;                           // Класс выбрасывающий "std::runtime_error"

    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeFallAlloc = Deque::MyDeque<Controllable, FaultyAllocator, sizeElemMap>;
    using MyDequeMockAllocContr = Deque::MyDeque<Controllable, MyPmrAllocator, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mock_res;
    MyPmrAllocator alloc(&mock_res);

    EXPECT_THROW((MyDequePMRAllocThrowElem(10, ThrowingElement())), std::runtime_error);
    EXPECT_THROW((MyDequeFallAlloc(10, Controllable())), std::bad_alloc);

    MyDequeMockAllocContr dequqPmr(4, Controllable(1, 1.f, std::string{ "element1" }), alloc);
    EXPECT_NO_THROW(dequqPmr);

    MockMemoryResource mock_res_throw;
    MyPmrAllocatorThrow alloc_throw(&mock_res_throw);
    EXPECT_THROW({MyDequePMRAllocThrowElem bad_deque(10, ThrowingElement(), alloc_throw);}, std::runtime_error);
    EXPECT_NO_THROW({ MyDequePMRAllocThrowElem bad_deque(4, ThrowingElement(), alloc_throw); }, std::runtime_error);

    EXPECT_EQ(mock_res_throw.active_allocations, 0);
    EXPECT_EQ(mock_res_throw.deallocate_calls, mock_res_throw.allocate_calls);
}

TEST(MyDequeConstructorTest, ElementsConstructionFailure) {
    const constexpr int sizeElemMap = 7;
    
    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyDequeStdAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;

    MockMemoryResource mock_res;
    MyPmrAllocatorContr allocContr(&mock_res);
    MyPmrAllocatorInt allocInt(&mock_res);

    {
        MyDequeStdAllocInt deque1(18, 16, allocInt);
        MyDequePmrAllocContr deque2(10, Controllable(), allocContr);

        auto sizeDeqint = deque1.size();
        for (size_t j = 0; j < sizeDeqint; ++j) {
            EXPECT_EQ(deque1[j], 16);
        }
        auto sizeDeqCont = deque2.size();
        auto ItDeq2Begin = std::rbegin(deque2);
        auto ItDeq2End = std::rend(deque2);
        while (ItDeq2Begin != ItDeq2End) {
            EXPECT_EQ(*ItDeq2Begin, Controllable());
            ++ItDeq2Begin;
        }
    } // Проверяем, что все освобождено
    EXPECT_EQ(mock_res.active_allocations, 0);
    EXPECT_EQ(mock_res.deallocate_calls, mock_res.allocate_calls);
}

TEST(MyDequeConstructorTest, CopyConstructionAndAssigmMath) {
    const constexpr int sizeElemMap = 11;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;

    MyDequePmrAllocContr dequeContr1(13, Controllable(9, 9.f, std::string("element9")));
    MyDequePmrAllocInt dequeInt1(16, 22);

    MyDequePmrAllocContr dequeContr2(dequeContr1);
    MyDequePmrAllocInt dequeInt2(dequeInt1);

    MyDequePmrAllocContr dequeContr3{};
    MyDequePmrAllocInt dequeInt3{};

    std::ranges::transform(dequeContr1, std::rbegin(dequeContr1), [](auto&& it) {static int r = 0;  r++;
    return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        });
    std::ranges::transform(dequeInt1, std::begin(dequeInt1), [](auto&& it) {static int r = 0;  r++;
    return (r);
        });

    dequeContr3 = dequeContr1;
    dequeInt3 = dequeInt1;

    EXPECT_TRUE((dequeContr2.size() == 13) && (dequeContr3.size() == 13) && (dequeContr1.size() == 13));
    EXPECT_TRUE((dequeContr2.capasity() == 44) && (dequeContr3.capasity() == 44) && (dequeContr1.capasity() == 44));
    EXPECT_TRUE((dequeContr2.getSizeMap() == 4) && (dequeContr3.getSizeMap() == 4) && (dequeContr1.getSizeMap() == 4));
    EXPECT_TRUE((dequeContr2.getFirstBlock() == 1) && (dequeContr3.getFirstBlock() == 1) && (dequeContr1.getFirstBlock() == 1));
    EXPECT_TRUE((dequeContr2.getLastBlock() == 2) && (dequeContr3.getLastBlock() == 2) && (dequeContr1.getLastBlock() == 2));

    EXPECT_TRUE((dequeInt2.size() == 16) && (dequeInt3.size() == 16) && (dequeInt1.size() == 16));
    EXPECT_TRUE((dequeInt2.capasity() == 44) && (dequeInt3.capasity() == 44) && (dequeInt1.capasity() == 44));
    EXPECT_TRUE((dequeInt2.getSizeMap() == 4) && (dequeInt3.getSizeMap() == 4) && (dequeInt1.getSizeMap() == 4));
    EXPECT_TRUE((dequeInt2.getFirstBlock() == 1) && (dequeInt3.getFirstBlock() == 1) && (dequeInt1.getFirstBlock() == 1));
    EXPECT_TRUE((dequeInt2.getLastBlock() == 2) && (dequeInt3.getLastBlock() == 2) && (dequeInt1.getLastBlock() == 2));

    auto ItDeqInt1 = std::begin(dequeInt1);
    auto size = dequeInt2.size();
    for (size_t j = 0; j < size; ++j) {
        EXPECT_EQ(*ItDeqInt1, j + 1);
        EXPECT_EQ(dequeInt2[j], 22);
        EXPECT_EQ(dequeInt3[j], *ItDeqInt1);
        ++ItDeqInt1;
    }

    auto ItDeqContr1 = std::begin(dequeContr1);
    size = dequeContr2.size();
    for (size_t j = 0; j < size; ++j) {
        auto r = 13 - j;
        EXPECT_EQ(*ItDeqContr1, Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        EXPECT_EQ(dequeContr2[j], Controllable(9, static_cast<float>(9), (std::string{ "element" } + std::to_string(9))));
        EXPECT_EQ(dequeContr3[j], *ItDeqContr1);
        ++ItDeqContr1;
    }
}

TEST(MyDequeConstructorTest, CopyConstructionAndAssigmAllocate) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockResContr;
    MyPmrAllocatorContr allocContr(&mockResContr);
    MyPmrAllocatorInt allocInt(&mockResContr);
    MyPmrAllocatorThrow allocThrow(&mockResContr);

    {
        ThrowingElement::max_success_allocs = 10;
        MyDequePMRAllocThrowElem dequeThrow1(8, ThrowingElement(), allocThrow);
        EXPECT_NO_THROW(dequeThrow1);
        //std::cout << ThrowingElement::construct_count << " " << ThrowingElement::max_success_allocs << "\n";

        ThrowingElement::max_success_allocs = 17;
        MyDequePMRAllocThrowElem dequeThrow2(dequeThrow1);
        EXPECT_THROW((MyDequePMRAllocThrowElem(dequeThrow1)), std::runtime_error);
        //std::cout << ThrowingElement::construct_count << " " << ThrowingElement::max_success_allocs << "\n";
    }
    //std::cout << mockResContr.allocate_calls << " " << mockResContr.deallocate_calls << " " << mockResContr.active_allocations << "\n";
    EXPECT_EQ(mockResContr.active_allocations, 0);
    EXPECT_EQ(mockResContr.deallocate_calls, mockResContr.allocate_calls);
    EXPECT_EQ(mockResContr.deallocate_calls, 9);

    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);

    MyDequePmrAllocContr dequeContr1(13, Controllable(9, 9.f, std::string("element9")), allocContrBad);
    EXPECT_NO_THROW(dequeContr1);
    FaultyAllocator::BadMemoryResource::max_size = 100;
    EXPECT_THROW((MyDequePmrAllocContr(dequeContr1)), std::bad_alloc);

    {
        MyDequePmrAllocContr dequeContr3(13, Controllable(7, 7.f, std::string("element7")), allocContr);
        MyDequePmrAllocContr dequeContr4(dequeContr3);
        MyDequePmrAllocContr dequeContr5{};
        dequeContr5 = dequeContr4;
    }
    //std::cout << mockResContr.allocate_calls << " " << mockResContr.deallocate_calls << " " << mockResContr.active_allocations << "\n";
    EXPECT_EQ(mockResContr.active_allocations, 0);
    EXPECT_EQ(mockResContr.deallocate_calls, mockResContr.allocate_calls);
    EXPECT_EQ(mockResContr.deallocate_calls, 15);

    {
        MyDequePmrAllocInt dequeInt1(16, 22, allocInt);
        MyDequePmrAllocInt dequeInt2(dequeInt1);
        MyDequePmrAllocInt dequeInt3{};

        dequeInt3 = dequeInt1;
    }
    //std::cout << mockResContr.allocate_calls << " " << mockResContr.deallocate_calls << " " << mockResContr.active_allocations << "\n";
    EXPECT_EQ(mockResContr.active_allocations, 0);
    EXPECT_EQ(mockResContr.deallocate_calls, mockResContr.allocate_calls);
    EXPECT_EQ(mockResContr.deallocate_calls, 23);
}

TEST(MyDequeConstructorTest, MoveConstructionAndAssigmMath) {
    const constexpr int sizeElemMap = 15;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;

    MyDequePmrAllocContr dequeContr1(17, Controllable(9, 9.f, std::string("element9")));
    MyDequePmrAllocContr dequeContr2(std::move(dequeContr1));

    EXPECT_TRUE((dequeContr2.size() == 17) && (dequeContr1.size() == 0));
    EXPECT_TRUE((dequeContr2.capasity() == 60) && (dequeContr1.capasity() == 0));
    EXPECT_TRUE((dequeContr2.getSizeMap() == 4) && (dequeContr1.getSizeMap() == 0));
    EXPECT_TRUE((dequeContr2.getFirstBlock() == 1) && (dequeContr1.getFirstBlock() == 0));
    EXPECT_TRUE((dequeContr2.getLastBlock() == 2) && (dequeContr1.getLastBlock() == 0));

    //dequeContr1.push_back(Controllable(9, 9.f, std::string("element9")));
    //std::cout << dequeContr1.size() << " " << dequeContr1.capasity() << "\n";
    dequeContr1.size();
    
    std::ranges::generate_n(std::back_inserter(dequeContr1), 8, []() mutable {static int r = 0; r++;
            return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));; });
    std::ranges::generate_n(std::front_inserter(dequeContr1), 10, []() mutable {static int r = 0; r--;
        return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));; });
    //std::ranges::copy(dequeContr1, std::ostream_iterator<Controllable>(std::cout, "\n"));

    MyDequePmrAllocContr dequeContr3{};
    dequeContr3 = std::move(dequeContr2);
    dequeContr2 = std::move(dequeContr1);

    EXPECT_TRUE((dequeContr3.size() == 17) && (dequeContr2.size() == 18) && (dequeContr1.size() == 0));
    //std::cout << dequeContr2.size() << " " << dequeContr2.capasity() << " " << dequeContr2.getFirstBlock() << "\n";
    EXPECT_TRUE((dequeContr3.capasity() == 60) && (dequeContr2.capasity() == 45) && (dequeContr1.capasity() == 0));
    EXPECT_TRUE((dequeContr3.getSizeMap() == 4) && (dequeContr2.getSizeMap() == 3) && (dequeContr1.getSizeMap() == 0));
    EXPECT_TRUE((dequeContr3.getFirstBlock() == 1) && (dequeContr2.getFirstBlock() == 0) && (dequeContr1.getFirstBlock() == 0));
    EXPECT_TRUE((dequeContr3.getLastBlock() == 2) && (dequeContr2.getLastBlock() == 1) && (dequeContr1.getLastBlock() == 0));
    auto ItDeqContrBeg2 = std::begin(dequeContr2);
    auto ItDeqContrEnd2 = std::end(dequeContr2);
    int r = -10;
    while (ItDeqContrBeg2 != ItDeqContrEnd2) {
        EXPECT_EQ(*ItDeqContrBeg2, Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        if (r == -1) ++r;
        ++ItDeqContrBeg2;
        ++r;
    }
    int size = dequeContr3.size();
    for (size_t j = 0; j < size; ++j)
        EXPECT_EQ(dequeContr3[j], Controllable(9, 9.f, std::string("element9")));

    MyDequePmrAllocInt dequeInt1(16, 22);
    MyDequePmrAllocInt dequeInt2(std::move(dequeInt1));

    EXPECT_TRUE((dequeInt2.size() == 16) && (dequeInt1.size() == 0));
    EXPECT_TRUE((dequeInt2.capasity() == 60) && (dequeInt1.capasity() == 0));
    EXPECT_TRUE((dequeInt2.getSizeMap() == 4) && (dequeInt1.getSizeMap() == 0));
    EXPECT_TRUE((dequeInt2.getFirstBlock() == 1) && (dequeInt1.getFirstBlock() == 0));
    EXPECT_TRUE((dequeInt2.getLastBlock() == 2) && (dequeInt1.getLastBlock() == 0));

    std::ranges::generate_n(std::back_inserter(dequeInt1), 17, []() mutable {static int r = 0; r++;
    return r; });

    MyDequePmrAllocInt dequeInt3{};

    dequeInt3 = std::move(dequeInt2);
    dequeInt2 = std::move(dequeInt1);

    EXPECT_TRUE((dequeInt3.size() == 16) && (dequeInt2.size() == 17) && (dequeInt1.size() == 0));
    EXPECT_TRUE((dequeInt3.capasity() == 60) && (dequeInt2.capasity() == 45) && (dequeInt1.capasity() == 0));
    EXPECT_TRUE((dequeInt3.getSizeMap() == 4) && (dequeInt2.getSizeMap() == 3) && (dequeInt1.getSizeMap() == 0));
    EXPECT_TRUE((dequeInt3.getFirstBlock() == 1) && (dequeInt2.getFirstBlock() == 1) && (dequeInt1.getFirstBlock() == 0));
    EXPECT_TRUE((dequeInt3.getLastBlock() == 2) && (dequeInt2.getLastBlock() == 2) && (dequeInt1.getLastBlock() == 0));
    //std::ranges::copy(dequeInt2, std::ostream_iterator<int>(std::cout, " "));

    auto ItDeqIntBeg2 = std::begin(dequeInt2);
    auto ItDeqIntEnd2 = std::end(dequeInt2);
    r = 1;
    while (ItDeqIntBeg2 != ItDeqIntEnd2) {
        EXPECT_EQ(*ItDeqIntBeg2, r);
        ++ItDeqIntBeg2;
        ++r;
    }
    size = dequeInt3.size();
    for (size_t j = 0; j < size; ++j)
        EXPECT_EQ(dequeInt3[j], 22);

    dequeInt3 = std::move(dequeInt2);

    EXPECT_TRUE((dequeInt2.size() == 0) && (dequeInt3.size() == 17));
    EXPECT_TRUE((dequeInt2.capasity() == 0) && (dequeInt3.capasity() == 45));
    EXPECT_TRUE((dequeInt2.getSizeMap() == 0) && (dequeInt3.getSizeMap() == 3));
    EXPECT_TRUE((dequeInt2.getFirstBlock() == 0) && (dequeInt3.getFirstBlock() == 1));
    EXPECT_TRUE((dequeInt2.getLastBlock() == 0) && (dequeInt3.getLastBlock() == 2));
}

TEST(MyDequeConstructorTest, MoveConstructionAndAssigmAllocate) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);
    {
        ThrowingElement::max_success_allocs = 10;
        ThrowingElement::max_moves_elem = 7;
        MyDequePMRAllocThrowElem dequeThrow1(9, ThrowingElement(), allocThrow);
        EXPECT_NO_THROW(dequeThrow1);
        EXPECT_NO_THROW((MyDequePMRAllocThrowElem(std::move(dequeThrow1))));
        static_assert(std::is_nothrow_move_constructible_v<MyDequePmrAllocInt>);
        static_assert(!std::is_nothrow_move_assignable_v<MyDequePmrAllocInt>);
    }

    {
        ExeptAllocatorInt::should_throw_on_move = true;
        MyDequeExceptAllocInt dequeExcept1(11, 33);
        EXPECT_NO_THROW(dequeExcept1);
        //EXPECT_NO_THROW((MyDequePMRAllocThrowElem(std::move(dequeThrow1))));
        EXPECT_THROW({MyDequeExceptAllocInt destination(std::move(dequeExcept1));
            }, std::overflow_error);
        ExeptAllocatorInt::should_throw_on_move = false;
        MyDequeExceptAllocInt dequeExcept2(10, 22);

        EXPECT_EQ(dequeExcept1.size(), 11);
        EXPECT_EQ(dequeExcept1.front(), 33);
        EXPECT_EQ(*dequeExcept1.rbegin(), 33);
        EXPECT_EQ(dequeExcept2.size(), 10);
        EXPECT_EQ(dequeExcept2.front(), 22);
        EXPECT_EQ(*dequeExcept2.rbegin(), 22);

        ExeptAllocatorInt::should_throw_on_move = true;

        static_assert(std::is_nothrow_move_assignable_v<Deque::MyDeque<int, std::allocator<int>>>);
        static_assert(!std::is_nothrow_move_assignable_v<MyDequeExceptAllocInt>);

        EXPECT_THROW({ dequeExcept2 = std::move(dequeExcept1); }, std::invalid_argument);
        //EXPECT_THROW({
        //    // Этот Guard уничтожится СРАЗУ при выходе из этих {} скобок, 
        //    // гарантируя выключение флага до того, как gtest пойдет уничтожать деки!
        //    struct LocalGuard {
        //        ~LocalGuard() {
        //            ExeptAllocatorInt::should_throw_on_move = false;
        //        }
        //    } local_guard;

        //    dequeExcept2 = std::move(dequeExcept1);
        //    }, std::exception); 
        ExeptAllocatorInt::should_throw_on_move = false;

        EXPECT_EQ(dequeExcept1.size(), 11);
        EXPECT_EQ(dequeExcept1.front(), 33);
        EXPECT_EQ(*dequeExcept1.rbegin(), 33);
        EXPECT_EQ(dequeExcept2.size(), 10);
        EXPECT_EQ(dequeExcept2.front(), 22);
        EXPECT_EQ(*dequeExcept2.rbegin(), 22);

        //std::cout << dequeExcept2.size() << " " << dequeExcept2.capasity() << "\n";
    }

    static_assert(std::is_nothrow_move_constructible_v<MyDequePmrAllocContr>);
    static_assert(!std::is_nothrow_move_assignable_v<MyDequePmrAllocContr>);

    //std::cout << mockRes.allocate_calls << " " << mockRes.deallocate_calls << " " << mockRes.active_allocations << "\n";

    MyDequePmrAllocContr dequeContr3(13, Controllable(7, 7.f, std::string("element7")), allocContr);
    //std::cout << mockRes.allocate_calls << " " << mockRes.deallocate_calls << " " << mockRes.active_allocations << "\n";
    EXPECT_NO_THROW({ MyDequePmrAllocContr dequeContr4(std::move(dequeContr3));});
    MyDequePmrAllocContr dequeContr5{};
    EXPECT_NO_THROW({ dequeContr5 = dequeContr3;});
    //std::cout << mockRes.allocate_calls << " " << mockRes.deallocate_calls << " " << mockRes.active_allocations << "\n";

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 6);
    EXPECT_EQ(mockRes.allocate_calls, 6);
}

TEST(MyDequeConstructorTest, InitialiserConstructionMath) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;

    MyDequePmrAllocContr dequeContr1({ 
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")), 
        Controllable(9, 9.f, std::string("element9")) });
    MyDequePmrAllocInt dequeInt1({99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80});

    EXPECT_TRUE((dequeInt1.size() == 20) && (dequeContr1.size() == 9));
    EXPECT_TRUE((dequeInt1.capasity() == 35) && (dequeContr1.capasity() == 28));
    EXPECT_TRUE((dequeInt1.getSizeMap() == 5) && (dequeContr1.getSizeMap() == 4));
    EXPECT_TRUE((dequeInt1.getFirstBlock() == 1) && (dequeContr1.getFirstBlock() == 1));
    EXPECT_TRUE((dequeInt1.getLastBlock() == 3) && (dequeContr1.getLastBlock() == 2));

    auto itContrRbegin = std::crbegin(dequeContr1);
    auto itContrRend = std::crend(dequeContr1);
    auto r = 9;
    while (itContrRbegin != itContrRend) {
        EXPECT_EQ(*(itContrRbegin.operator->()), Controllable(r, static_cast<float>(r), (std::string{"element"} + std::to_string(r))));
        itContrRbegin++;
        r--;
    }
    r = 99;
    auto size = dequeInt1.size();
    for (int i = 0; i < size; ++i) 
        EXPECT_EQ(dequeInt1[i], r - i);
}

TEST(MyDequeConstructorTest, InitialiserConstructionAllocate) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;
    using MyDequeFallAllocIntElem = Deque::MyDeque<int, FaultyAlloc, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);
    {
        ThrowingElement::max_success_allocs = 17;
        ThrowingElement::max_moves_elem = 9;
        //std::list<ThrowingElement> listContr1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(), 
            //ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement()});
        MyDequePMRAllocThrowElem dequeThrow1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() }, allocThrow);
        EXPECT_NO_THROW(dequeThrow1);
        EXPECT_THROW({ MyDequePMRAllocThrowElem dequeThrow2(
            { ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() }, allocThrow); }, std::runtime_error);
    }
    //std::cout << mockRes.allocate_calls << " " << mockRes.deallocate_calls << " " << mockRes.active_allocations << "\n";

    {
        MyDequePmrAllocInt dequeInt1 ({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, allocInt);
        EXPECT_NO_THROW(dequeInt1);
        EXPECT_NO_THROW({ MyDequePmrAllocInt dequeInt1({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, allocInt); });
    }

    EXPECT_THROW({ MyDequeFallAllocIntElem deq({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}); }, std::bad_alloc);

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 10);
    EXPECT_EQ(mockRes.allocate_calls, 10);
}

TEST(MyDequeConstructorTest, IteratorConstructionMath) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;

    std::list<Controllable> listContr1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")) });
    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80 });

    MyDequePmrAllocContr dequeContr1(listContr1.cbegin(), listContr1.cend());
    MyDequePmrAllocInt dequeInt1(std::crbegin(vecInt1), std::crend(vecInt1));

    EXPECT_TRUE((dequeInt1.size() == 20) && (dequeContr1.size() == 9));
    EXPECT_TRUE((dequeInt1.capasity() == 35) && (dequeContr1.capasity() == 21));
    EXPECT_TRUE((dequeInt1.getSizeMap() == 5) && (dequeContr1.getSizeMap() == 3));
    EXPECT_TRUE((dequeInt1.getFirstBlock() == 1) && (dequeContr1.getFirstBlock() == 1));
    EXPECT_TRUE((dequeInt1.getLastBlock() == 3) && (dequeContr1.getLastBlock() == 2));

    auto itContrRbegin = std::crbegin(dequeContr1);
    auto itContrRend = std::crend(dequeContr1);
    auto r = 9;
    while (itContrRbegin != itContrRend) {
        EXPECT_EQ(*(itContrRbegin.operator->()), Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        itContrRbegin++;
        r--;
    }
    r = 80;
    auto size = dequeInt1.size();
    for (int i = 0; i < size; ++i)
        EXPECT_EQ(dequeInt1[i], r + i);
}

TEST(MyDequeConstructorTest, IteratorConstructionAllocate) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;
    using MyDequeFallAllocIntElem = Deque::MyDeque<int, FaultyAlloc, sizeElemMap>;

    std::list<Controllable> listContr1({
    Controllable(1, 1.f, std::string("element1")),
    Controllable(2, 2.f, std::string("element2")),
    Controllable(3, 3.f, std::string("element3")),
    Controllable(4, 4.f, std::string("element4")),
    Controllable(5, 5.f, std::string("element5")),
    Controllable(6, 6.f, std::string("element6")),
    Controllable(7, 7.f, std::string("element7")),
    Controllable(8, 8.f, std::string("element8")),
    Controllable(9, 9.f, std::string("element9")) });
    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80 });
    std::deque<ThrowingElement> deqThrow1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() });

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 17;
        ThrowingElement::max_moves_elem = 9;
        MyDequePMRAllocThrowElem dequeThrow2(std::cbegin(deqThrow1), std::cend(deqThrow1), allocThrow);
        EXPECT_NO_THROW(dequeThrow2);
        EXPECT_THROW({ MyDequePMRAllocThrowElem dequeThrow2(std::cbegin(deqThrow1), std::cend(deqThrow1), allocThrow); }, std::runtime_error);
    }

    {
        MyDequePmrAllocInt dequeInt1(std::cbegin(vecInt1), std::cend(vecInt1), allocInt);
        EXPECT_NO_THROW(dequeInt1);
        EXPECT_NO_THROW({ MyDequePmrAllocInt dequeInt2(std::cbegin(vecInt1), std::cend(vecInt1), allocInt); });
    }
    EXPECT_THROW({ MyDequeFallAllocIntElem deq(std::cbegin(vecInt1), std::cend(vecInt1)); }, std::bad_alloc);

    EXPECT_NO_THROW({ MyDequePmrAllocContr deqlistContr1(std::cbegin(listContr1), std::cend(listContr1), allocContr); } );
    
    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 15);
    EXPECT_EQ(mockRes.allocate_calls, 15);
}

TEST(MyDequeMethodTest, push_backMetodTrivialMath) {
    const constexpr int sizeElemMap = 12;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")) });

    contrContr1.push_back(Controllable(10, 10.f, std::string("element10")));
    contrContr1.push_back(Controllable(11, 11.f, std::string("element11")));
    EXPECT_EQ(contrContr1[9], Controllable(10, 10.f, std::string("element10")));
    EXPECT_EQ(contrContr1[10], Controllable(11, 11.f, std::string("element11")));
    EXPECT_EQ(contrContr1.size(), 11);
    EXPECT_EQ(contrContr1.capasity(), 36);
    EXPECT_TRUE(contrContr1.getSizeMap() == 3);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);

    std::ranges::generate_n(std::back_inserter(contrContr1), 11, []() mutable {static int r = 11; r++;
        return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));; });
    EXPECT_EQ(contrContr1.size(), 22);
    EXPECT_EQ(contrContr1.capasity(), 36);
    EXPECT_TRUE(contrContr1.getSizeMap() == 3);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    EXPECT_EQ(contrContr1[13], Controllable(14, 14.f, std::string("element14")));
    EXPECT_EQ(contrContr1[17], Controllable(18, 18.f, std::string("element18")));
    EXPECT_EQ(contrContr1[21], Controllable(22, 22.f, std::string("element22")));

    contrContr1.push_back(Controllable(23, 23.f, std::string("element23")));
    EXPECT_EQ(contrContr1.size(), 23);
    EXPECT_EQ(contrContr1.capasity(), 72);
    EXPECT_TRUE(contrContr1.getSizeMap() == 6);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 4);

    Deque::MyDeque<int, std::allocator<int>, sizeElemMap> contrInt1({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});

    contrInt1.push_back(16); contrInt1.push_back(17);
    EXPECT_EQ(contrInt1[15], 16);
    EXPECT_EQ(contrInt1[16], 17);
    EXPECT_EQ(contrInt1.size(), 17);
    EXPECT_EQ(contrInt1.capasity(), 48);
    EXPECT_TRUE(contrInt1.getSizeMap() == 4);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 2);

    std::ranges::generate_n(std::back_inserter(contrInt1), 14, []() mutable {static int r = 17; r++; return r; });
    EXPECT_EQ(contrInt1[20], 21);
    EXPECT_EQ(contrInt1[30], 31);
    EXPECT_EQ(contrInt1.size(), 31);
    EXPECT_EQ(contrInt1.capasity(), 48);
    EXPECT_TRUE(contrInt1.getSizeMap() == 4);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 3);

    contrInt1.push_back(32);
    EXPECT_EQ(contrInt1[31], 32);
    EXPECT_EQ(contrInt1.size(), 32);
    EXPECT_EQ(contrInt1.capasity(), 96);
    EXPECT_TRUE(contrInt1.getSizeMap() == 8);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 2);
    EXPECT_TRUE(contrInt1.getLastBlock() == 5);
}

TEST(MyDequeMethodTest, push_backMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 7;
    
    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;
        MyDequePMRAllocThrowElem dequeThrow1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() }, allocThrow);
        ThrowingElement::max_success_allocs = 8;
        EXPECT_NO_THROW(dequeThrow1);
        EXPECT_THROW({ dequeThrow1.push_back(ThrowingElement()); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 8);
        EXPECT_EQ(dequeThrow1.capasity(), 28);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
    }
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")) }, allocContrBad);
        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        EXPECT_NO_THROW({std::ranges::generate_n(std::back_inserter(contrContr1), 8, []() mutable {static int r = 6; r++;
            return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));; }); });
        EXPECT_EQ(contrContr1.size(), 14);
        EXPECT_EQ(contrContr1.capasity(), 21);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        //std::cout << contrContr1[1] << " " << contrContr1[3] << " " << contrContr1[4] << "\n";
        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({ contrContr1.push_back(Controllable(15, 15.f, std::string{ "element15" })); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 14);
        EXPECT_EQ(contrContr1.capasity(), 21);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        //std::cout << sizeof(contrContr1) << "\n";
        //std::cout << contrContr1[1] << " " << contrContr1[1] << " " << contrContr1[1] << "\n";
    }

    {
        MyDequePmrAllocInt dequeInt1(0, int{}, allocInt);
        EXPECT_NO_THROW({ std::ranges::copy(std::views::iota(0, 40), std::back_inserter(dequeInt1)); });
        EXPECT_EQ(dequeInt1.size(), 40);
        EXPECT_EQ(dequeInt1.capasity(), 84);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 13);
    EXPECT_EQ(mockRes.allocate_calls, 13);

}

TEST(MyDequeMethodTest, push_frontMetodTrivialMath) {
    const constexpr int sizeElemMap = 12;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")) });

    //std::cout << std::begin(contrContr1).getFirstI() << " " << std::begin(contrContr1).getFirstJ() << "\n";

    contrContr1.push_front(Controllable(10, 10.f, std::string("element10")));
    contrContr1.push_front(Controllable(11, 11.f, std::string("element11")));
    EXPECT_EQ(contrContr1[0], Controllable(11, 11.f, std::string("element11")));
    EXPECT_EQ(contrContr1[1], Controllable(10, 10.f, std::string("element10")));
    EXPECT_EQ(contrContr1.size(), 11);
    EXPECT_EQ(contrContr1.capasity(), 36);
    EXPECT_TRUE(contrContr1.getSizeMap() == 3);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 1);
    //std::cout << contrContr1.size() << " " << contrContr1.capasity() << " " << contrContr1.getSizeMap() << "\n";
    //std::cout << std::begin(contrContr1).getFirstI() << " " << std::begin(contrContr1).getFirstJ() << "\n";
    /*auto&& map = contrContr1.getMap();
    for (size_t i = 0; i < contrContr1.getSizeMap(); ++i) {
        if (map[i] != nullptr) {
            for (size_t j = 0; j < contrContr1.size_elem_map_; ++j) {
                if (map[i] != nullptr)
                    std::cout << map[i][j].i << "\t";
                else
                    std::cout << "NULL\t";
            }
            std::cout << "\n";
        }
        else {
            for (size_t j = 0; j < contrContr1.size_elem_map_; ++j)
                std::cout << "NULL\t";
            std::cout << "\n";
        }
    } std::cout << "\n" << "\n";*/

    std::ranges::generate_n(std::front_inserter(contrContr1), 12, []() mutable {static int r = 11; r++;
            return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r)))); });

    contrContr1.push_front(Controllable(24, 24.f, std::string("element24")));
    EXPECT_EQ(contrContr1.size(), 24);
    EXPECT_EQ(contrContr1.capasity(), 72);
    EXPECT_TRUE(contrContr1.getSizeMap() == 6);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 3);
    //std::cout << std::begin(contrContr1).getFirstI() << " " << std::begin(contrContr1).getFirstJ() << "\n";
    EXPECT_EQ(contrContr1[0], Controllable(24, 24.f, std::string("element24")));
    EXPECT_EQ(contrContr1[6], Controllable(18, 18.f, std::string("element18")));
    EXPECT_EQ(contrContr1[21], Controllable(7, 7.f, std::string("element7")));

}

TEST(MyDequeMethodTest, push_frontMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;
        MyDequePMRAllocThrowElem dequeThrow1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() }, allocThrow);
        ThrowingElement::max_success_allocs = 8;
        EXPECT_NO_THROW(dequeThrow1);
        EXPECT_THROW({ dequeThrow1.push_front(ThrowingElement()); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 8);
        EXPECT_EQ(dequeThrow1.capasity(), 27);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
    }
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")) }, allocContrBad);
        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        EXPECT_NO_THROW({ std::ranges::generate_n(std::back_inserter(contrContr1), 13, []() mutable {static int r = 9; r++;
            return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));; }); });
        EXPECT_EQ(contrContr1.size(), 22);
        EXPECT_EQ(contrContr1.capasity(), 36);
        EXPECT_TRUE(contrContr1.getSizeMap() == 4);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 3);
        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        //std::cout << contrContr1[16].f << " " << contrContr1.end().getFirstI() << " " << contrContr1.end().getFirstJ() << "\n";
        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({ contrContr1.push_back(Controllable(23, 23.f, std::string{ "element23" })); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 22);
        EXPECT_EQ(contrContr1.capasity(), 36);
        EXPECT_TRUE(contrContr1.getSizeMap() == 4);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 3);
        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        //std::cout << sizeof(contrContr1) << "\n";
        //std::cout << contrContr1[1] << " " << contrContr1[1] << " " << contrContr1[1] << "\n";
    }
    {
        MyDequePmrAllocInt dequeInt1(0, int{}, allocInt);
        EXPECT_NO_THROW({ std::ranges::copy(std::views::iota(0, 40), std::front_inserter(dequeInt1)); });
        EXPECT_EQ(dequeInt1.size(), 40);
        EXPECT_EQ(dequeInt1.capasity(), 108);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 11);
    EXPECT_EQ(mockRes.allocate_calls, 11);
}

TEST(MyDequeMethodTest, push_backMetodItheratorMath) {
    const constexpr int sizeElemMap = 6;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    
    MyDequePmrAllocContr contrContr1;
    std::deque< Controllable> deq1 ({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")) });
    contrContr1.push_back(std::cbegin(deq1), std::cend(deq1));
    EXPECT_EQ(contrContr1.size(), 9);
    EXPECT_EQ(contrContr1.capasity(), 18);
    EXPECT_TRUE(contrContr1.getSizeMap() == 3);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 1);
    EXPECT_EQ(contrContr1[4], Controllable(5, 5.f, std::string("element5")));
    EXPECT_EQ(contrContr1[7], Controllable(8, 8.f, std::string("element8")));
    //std::ranges::copy(contrContr1 | std::views::transform(&Controllable::f), std::ostream_iterator<int>(std::cout, " "));

    Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap> contrInt1;
    EXPECT_EQ(contrInt1.size(), 0);
    EXPECT_EQ(contrInt1.capasity(), 18);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);
    //std::cout << contrInt1.cbegin().getFirstI() << " " << contrInt1.cbegin().getFirstJ() << "\n";
    //std::cout << contrInt1.size() << " " << contrInt1.capasity() << "\n";
    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80 });
    contrInt1.push_back(std::begin(vecInt1), std::begin(vecInt1) + 3);
    contrInt1.push_back(std::begin(vecInt1) + 3, std::begin(vecInt1) + 6);
    contrInt1.push_back(std::begin(vecInt1), std::end(vecInt1));

    EXPECT_EQ(contrInt1.size(), 26);
    EXPECT_EQ(contrInt1.capasity(), 66);
    EXPECT_TRUE(contrInt1.getSizeMap() == 11);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 4);
    EXPECT_TRUE(contrInt1.getLastBlock() == 8);

    //std::ranges::copy(contrInt1, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << contrInt1.size() << " " << contrInt1.capasity() << "\n";
}

TEST(MyDequeMethodTest, push_backMetodItheratorAllocate) {
    const constexpr int sizeElemMap = 6;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;
        std::deque<ThrowingElement> deq1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() });
        MyDequePMRAllocThrowElem dequeThrow1 (allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 0);
        EXPECT_EQ(dequeThrow1.capasity(), 18);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);

        ThrowingElement::max_success_allocs = 8;
        EXPECT_NO_THROW(dequeThrow1);
        EXPECT_THROW({ dequeThrow1.push_back(std::cbegin(deq1), std::cend(deq1)); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 0);
        EXPECT_EQ(dequeThrow1.capasity(), 18);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
    }
    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 3);
    EXPECT_EQ(mockRes.allocate_calls, 3);

    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1 (allocContrBad);
        std::deque<Controllable> deq1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")),
        Controllable(10, 10.f, std::string("element10")),
        Controllable(11, 11.f, std::string("element11")),
        Controllable(12, 12.f, std::string("element12")) });

        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        EXPECT_NO_THROW({ contrContr1.push_back(std::cbegin(deq1), std::cbegin(deq1) + 4); });
        EXPECT_EQ(contrContr1.size(), 4);
        EXPECT_EQ(contrContr1.capasity(), 18);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 1);
        FaultyAllocator::BadMemoryResource::max_size = 1;

        EXPECT_THROW({contrContr1.push_back(std::cbegin(deq1), std::cend(deq1)); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 4);
        EXPECT_EQ(contrContr1.capasity(), 18);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 1);
    }
    {
        std::vector<int> vec1(std::views::iota(0, 40).begin(), std::views::iota(0, 40).end());
        MyDequePmrAllocInt dequeInt1(allocInt);
        EXPECT_NO_THROW({ dequeInt1.push_back(std::cbegin(vec1), std::cend(vec1)); });
        EXPECT_EQ(dequeInt1.size(), 40);
        EXPECT_EQ(dequeInt1.capasity(), 90);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 13);
    EXPECT_EQ(mockRes.allocate_calls, 13);
}

TEST(MyDequeMethodTest, push_frontMetodItheratorMath) {
    const constexpr int sizeElemMap = 10;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;

    Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap> contrInt1;

    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80 });
    //std::cout << contrInt1.cbegin().getFirstI() << " " << contrInt1.cbegin().getFirstJ() << "\n";
    auto itBegin = std::begin(vecInt1);
    //contrInt1.push_back(77);
    contrInt1.push_front(itBegin, (itBegin + 6));
    EXPECT_EQ(contrInt1.size(), 6);
    EXPECT_EQ(contrInt1.capasity(), 30);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);
    //contrInt1.push_front(itBegin, (itBegin + 2));
    //std::cout << contrInt1.cbegin().getFirstI() << " " << contrInt1.cbegin().getFirstJ() << "\n";
    contrInt1.push_front((itBegin + 6), (itBegin + 9));
    contrInt1.push_front((itBegin + 9), (itBegin + 12));
    EXPECT_EQ(contrInt1.size(), 12);
    EXPECT_EQ(contrInt1.capasity(), 30);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 0);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);
    //std::cout << contrInt1.cbegin().getFirstI() << " " << contrInt1.cbegin().getFirstJ() << "\n";
    contrInt1.push_front((itBegin + 12), (itBegin + 20));
    EXPECT_EQ(contrInt1.size(), 20);
    EXPECT_EQ(contrInt1.capasity(), 70);
    EXPECT_TRUE(contrInt1.getSizeMap() == 7);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 2);
    EXPECT_TRUE(contrInt1.getLastBlock() == 4);

    /*auto&& map = contrInt1.getMap();
    for (size_t i = 0; i < contrInt1.getSizeMap(); ++i) {
        if (map[i] != nullptr) {
            for (size_t j = 0; j < contrInt1.size_elem_map_; ++j) {
                if (map[i] != nullptr)
                    std::cout << map[i][j] << "\t";
                else
                    std::cout << "NULL\t";
            }
            std::cout << "\n";
        }
        else {
            for (size_t j = 0; j < contrInt1.size_elem_map_; ++j)
                std::cout << "NULL\t";
            std::cout << "\n";
        }
    } std::cout << "\n" << "\n";*/
    MyDequePmrAllocContr contrContr1;
    std::deque< Controllable> deq1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")) });

    contrContr1.push_front(std::cbegin(deq1), std::cend(deq1));
    EXPECT_EQ(contrContr1.size(), 9);
    EXPECT_EQ(contrContr1.capasity(), 30);
    EXPECT_TRUE(contrContr1.getSizeMap() == 3);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 0);
    EXPECT_TRUE(contrContr1.getLastBlock() == 1);
    EXPECT_EQ(contrContr1[2], Controllable(3, 3.f, std::string("element3")));

    contrContr1.push_front(std::cbegin(deq1), std::cend(deq1));
    EXPECT_EQ(contrContr1.size(), 18);
    EXPECT_EQ(contrContr1.capasity(), 70);
    EXPECT_TRUE(contrContr1.getSizeMap() == 7);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 4);
    EXPECT_EQ(contrContr1[2], Controllable(3, 3.f, std::string("element3")));
}

TEST(MyDequeMethodTest, push_frontMetodItheratorAllocate) {
    const constexpr int sizeElemMap = 6;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;
        std::deque<ThrowingElement> deq1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() });
        MyDequePMRAllocThrowElem dequeThrow1(allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 0);
        EXPECT_EQ(dequeThrow1.capasity(), 18);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);

        ThrowingElement::max_success_allocs = 8;
        EXPECT_NO_THROW(dequeThrow1);
        EXPECT_THROW({ dequeThrow1.push_front(std::cbegin(deq1), std::cend(deq1)); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 0);
        EXPECT_EQ(dequeThrow1.capasity(), 18);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
    }

    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        std::deque<Controllable> deq1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")),
        Controllable(10, 10.f, std::string("element10")),
        Controllable(11, 11.f, std::string("element11")),
        Controllable(12, 12.f, std::string("element12")) });

        //std::cout << contrContr1.size() << " " << contrContr1.capasity() << "\n";
        EXPECT_NO_THROW({ contrContr1.push_front(std::cbegin(deq1), std::cbegin(deq1) + 4); });
        EXPECT_EQ(contrContr1.size(), 4);
        EXPECT_EQ(contrContr1.capasity(), 18);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 1);
        FaultyAllocator::BadMemoryResource::max_size = 1;

        EXPECT_THROW({ contrContr1.push_front(std::cbegin(deq1), std::cend(deq1)); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 4);
        EXPECT_EQ(contrContr1.capasity(), 18);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 1);
    }

    {
        std::vector<int> vec1(std::views::iota(0, 40).begin(), std::views::iota(0, 40).end());
        MyDequePmrAllocInt dequeInt1(allocInt);
        EXPECT_NO_THROW({ dequeInt1.push_front(std::cbegin(vec1), std::cend(vec1)); });
        EXPECT_EQ(dequeInt1.size(), 40);
        EXPECT_EQ(dequeInt1.capasity(), 78);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 12);
    EXPECT_EQ(mockRes.allocate_calls, 12);
}

TEST(MyDequeMethodTest, InsertMetodTrivialMath) {
    const constexpr int sizeElemMap = 9;

    using constIteratorInt = Deque::DequeIterator<int, sizeElemMap, true>;
    using reverseIterControlInt = std::reverse_iterator<Deque::DequeIterator<int, sizeElemMap>>;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyDeque = Deque::MyDeque<Controllable, MyPmrAllocator, sizeElemMap>;
    using ConstReversIteratorControl = std::reverse_iterator <Deque::DequeIterator<Controllable, sizeElemMap, true>>;
    using IteratorControl = Deque::DequeIterator<Controllable, sizeElemMap>;

    Deque::MyDeque<int, std::allocator<int>, sizeElemMap> deq1(20);
    EXPECT_TRUE(deq1.size() == 20);
    EXPECT_EQ(deq1.capasity(), 45);
    EXPECT_TRUE(deq1.getSizeMap() == 5);
    EXPECT_TRUE(deq1.getFirstBlock() == 1);
    EXPECT_TRUE(deq1.getLastBlock() == 3);
    constIteratorInt cbegin1 = std::cbegin(deq1) + 14;
    ++cbegin1;
    auto it = deq1.insert(cbegin1, 55);
    EXPECT_EQ(*(it), 55);
    EXPECT_EQ(it, (std::cbegin(deq1) + 15));
    reverseIterControlInt rbegin1 = std::rbegin(deq1) + 14;
    auto it1 = deq1.insert(rbegin1.base(), 99);

    EXPECT_EQ(*(it1), 99);
    EXPECT_EQ(it1, std::begin(deq1) + 8);

    EXPECT_TRUE(deq1.size() == 22);
    EXPECT_EQ(deq1.capasity(), 45);
    EXPECT_TRUE(deq1.getSizeMap() == 5);
    EXPECT_TRUE(deq1.getFirstBlock() == 1);
    EXPECT_TRUE(deq1.getLastBlock() == 3);
    //std::ranges::copy(deq1, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << "\n";

    std::deque <Controllable> deqStd2(10);
    auto itStd = std::begin(deqStd2);
    MyDeque deq2(10);

    std::ranges::transform(deq2, std::begin(deq2), [](auto&& it) {static int r = 0; r++;
    return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        });

    EXPECT_TRUE(deq2.size() == 10);
    EXPECT_EQ(deq2.capasity(), 36);
    EXPECT_TRUE(deq2.getSizeMap() == 4);
    EXPECT_TRUE(deq2.getFirstBlock() == 1);
    EXPECT_TRUE(deq2.getLastBlock() == 2);

    deq2.insert(std::begin(deq2) + 3, Controllable(11, 11.f, (std::string{ "element11" })));
    deq2.insert(std::crbegin(deq2).base(), Controllable(12, 12.f, (std::string{ "element12" })));
    deq2.insert(std::begin(deq2), Controllable(13, 13.f, (std::string{ "element13" })));
    deq2.insert((std::rbegin(deq2) + 1).base(), Controllable(14, 14.f, (std::string{ "element14" })));

    EXPECT_TRUE(deq2.size() == 14);
    EXPECT_EQ(deq2.capasity(), 36);
    EXPECT_TRUE(deq2.getSizeMap() == 4);
    EXPECT_TRUE(deq2.getFirstBlock() == 1);
    EXPECT_TRUE(deq2.getLastBlock() == 2);
    //std::ranges::copy(deq2 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
    //std::cout << "\n";
}

TEST(MyDequeMethodTest, InsertMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;

        MyDequePMRAllocThrowElem dequeThrow1(2, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 2);
        EXPECT_EQ(dequeThrow1.capasity(), 21);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 17;
        ThrowingElement::max_moves_elem = 20;
        //std::cout << ThrowingElement::max_success_allocs << " " << ThrowingElement::max_moves_elem << '\n';
        EXPECT_NO_THROW({ for (size_t i = 0; i < 7; ++i) {
                            //std::cout << "ThrowingElement::construct_count - " << ThrowingElement::construct_count << "\n";
                            dequeThrow1.insert(std::begin(dequeThrow1) + i, ThrowingElement());
                        }; });
        EXPECT_EQ(dequeThrow1.size(), 9);
        EXPECT_EQ(dequeThrow1.capasity(), 21);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_success_allocs = 10;
        EXPECT_THROW({ dequeThrow1.insert(std::cbegin(dequeThrow1), ThrowingElement()); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 9);
        EXPECT_EQ(dequeThrow1.capasity(), 21);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_success_allocs = 20;
        EXPECT_NO_THROW({ dequeThrow1.insert(std::begin(dequeThrow1) + 6, ThrowingElement());});
        EXPECT_EQ(dequeThrow1.size(), 10);
    }
    ThrowingElement::construct_count = 0;
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        contrContr1.resize(11);
        EXPECT_EQ(contrContr1.size(), 11);
        EXPECT_EQ(contrContr1.capasity(), 21);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        std::ranges::transform(contrContr1, std::begin(contrContr1), [](auto&& it) {static int r = 0; r++;
                return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
            });

        EXPECT_NO_THROW({ for (size_t i = 0; i < 5; ++i) {
            //std::cout << "ThrowingElement::construct_count - " << ThrowingElement::construct_count << "\n";
            contrContr1.insert(std::begin(contrContr1) + i + 3, Controllable(10 + i, static_cast<float>(10 + i), (std::string{ "element" } + std::to_string(10 + i))));
        }; });
        EXPECT_EQ(contrContr1.size(), 16);
        EXPECT_EQ(contrContr1.capasity(), 21);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 0);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);

        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({ contrContr1.insert((std::crbegin(contrContr1) + 1).base(), Controllable(14, 14.f, (std::string{ "element14" }))); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 16);
        EXPECT_EQ(contrContr1.capasity(), 21);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 0);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);

        //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
        //std::cout << "\n";
    }

    {
        MyDequePmrAllocInt dequeInt1(2, int{}, allocInt);
        EXPECT_NO_THROW({ for (size_t i = 0; i < 40; ++i) {
            dequeInt1.insert(std::begin(dequeInt1) + i + 1, i);
        }; });
        //std::ranges::copy(dequeInt1, std::ostream_iterator<int>(std::cout, " "));
        EXPECT_EQ(dequeInt1.size(), 42);
        EXPECT_EQ(dequeInt1.capasity(), 84);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 14);
    EXPECT_EQ(mockRes.allocate_calls, 14);
}

TEST(MyDequeMethodTest, InsertMetodItheratorMath) {
    const constexpr int sizeElemMap = 8;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDeque = Deque::MyDeque<Controllable, MyPmrAllocator>;
    using ConstReversIteratorControl = std::reverse_iterator <Deque::DequeIterator<Controllable, sizeElemMap, true>>;
    using IteratorControl = Deque::DequeIterator<Controllable>;

    std::deque <Controllable> deqStd1(10);
    auto itStd = std::begin(deqStd1);
    MyDeque deq1(10);

    std::ranges::transform(deqStd1, itStd, [](auto&& it) {static int r = 9; r++;
        return (Controllable (r, static_cast<float>(r), (std::string {"element"} + std::to_string(r)) ) );
        });
    std::ranges::transform(deq1, std::begin(deq1), [](auto&& it) {static int r = 0; r++;
    return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
        });
    deq1.insert(std::begin(deq1) + 3, itStd + 2, itStd + 5);
    EXPECT_TRUE(deq1.size() == 13);
    EXPECT_EQ(*(std::begin(deq1) + 3), Controllable(12, 12.f, std::string("element12")));
    EXPECT_EQ(*(std::rbegin(deq1) + 8), Controllable(13, 13.f, std::string("element13")));
    EXPECT_EQ(*(std::rbegin(deq1) + 7), Controllable(14, 14.f, std::string("element14")));

    ConstReversIteratorControl itBeg = std::crbegin(deq1) + 2;

    deq1.insert(itBeg.base(), itStd + 7, itStd + 9);
    EXPECT_TRUE(deq1.size() == 15);
    EXPECT_EQ(*(std::begin(deq1) + 11), Controllable(17, 17.f, std::string("element17")));
    EXPECT_EQ(*(std::rbegin(deq1) + 2), Controllable(18, 18.f, std::string("element18")));

    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80 });
    auto itIntBeg = std::cbegin(vecInt1);
    Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap> contrInt1;
    EXPECT_EQ(contrInt1.size(), 0);
    EXPECT_EQ(contrInt1.capasity(), 24);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);

    contrInt1.insert(std::begin(contrInt1), itIntBeg, itIntBeg + 2);
    contrInt1.insert(std::begin(contrInt1) + 1, itIntBeg + 2, itIntBeg + 6);
    contrInt1.insert(std::begin(contrInt1) + 4, itIntBeg + 6, std::cend(vecInt1));
    EXPECT_EQ(contrInt1.size(), 20);
    EXPECT_EQ(contrInt1.capasity(), 48);
    EXPECT_TRUE(contrInt1.getSizeMap() == 6);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 2);
    EXPECT_TRUE(contrInt1.getLastBlock() == 4);
    EXPECT_EQ(*(std::begin(contrInt1) + 6), 91);
    EXPECT_EQ(*(std::crbegin(contrInt1) + 1), 94);

    //std::ranges::copy(contrInt1, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << "\n" << contrInt1.size() << " " << contrInt1.capasity() << "\n";
}

TEST(MyDequeMethodTest, InsertMetodItheratorAllocate) {
    const constexpr int sizeElemMap = 5;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 30;
        std::deque<ThrowingElement> deq1({ ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement(),
            ThrowingElement(), ThrowingElement(), ThrowingElement(), ThrowingElement() });
        MyDequePMRAllocThrowElem dequeThrow1(allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 0);
        EXPECT_EQ(dequeThrow1.capasity(), 15);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 20;
        ThrowingElement::max_moves_elem = 20;
        //std::cout << ThrowingElement::max_success_allocs << " " << ThrowingElement::max_moves_elem << '\n';
        EXPECT_NO_THROW({ dequeThrow1.insert(std::begin(dequeThrow1), std::begin(deq1), std::begin(deq1) + 3);});
        EXPECT_EQ(dequeThrow1.size(), 3);
        EXPECT_EQ(dequeThrow1.capasity(), 15);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 10;
        EXPECT_THROW({ dequeThrow1.insert(std::cbegin(dequeThrow1), std::begin(deq1), std::end(deq1)); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 3);
        EXPECT_EQ(dequeThrow1.capasity(), 15);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 20;
    }
    ThrowingElement::construct_count = 0;
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        std::deque<Controllable> deq1({
        Controllable(1, 1.f, std::string("element1")),
        Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")),
        Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")),
        Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")),
        Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")),
        Controllable(10, 10.f, std::string("element10")),
        Controllable(11, 11.f, std::string("element11")),
        Controllable(12, 12.f, std::string("element12")) });

        EXPECT_NO_THROW({ contrContr1.insert(std::begin(contrContr1), (std::crbegin(deq1) + 7).base(), (std::crbegin(deq1) + 2).base()); });
        EXPECT_EQ(contrContr1.size(), 5);
        EXPECT_EQ(contrContr1.capasity(), 15);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        FaultyAllocator::BadMemoryResource::max_size = 1;

        EXPECT_THROW({contrContr1.push_front(std::cbegin(deq1), std::cend(deq1));}, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 5);
        EXPECT_EQ(contrContr1.capasity(), 15);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
    }
    {
        std::vector<int> vec1(std::views::iota(0, 40).begin(), std::views::iota(0, 40).end());
        MyDequePmrAllocInt dequeInt1({41, 42, 43, 44, 45, 46}, allocInt);
        EXPECT_NO_THROW({ dequeInt1.insert(std::begin (dequeInt1), std::cbegin(vec1), std::cend(vec1)); });
        EXPECT_EQ(dequeInt1.size(), 46);
        EXPECT_EQ(dequeInt1.capasity(), 100);
    }
    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 23);
    EXPECT_EQ(mockRes.allocate_calls, 23);
}

TEST(MyDequeMethodTest, emplase_frontMetodTrivialMath) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDeque = Deque::MyDeque<Controllable, MyPmrAllocator, sizeElemMap>;
    using ConstReversIteratorControl = std::reverse_iterator <Deque::DequeIterator<Controllable, sizeElemMap, true>>;
    using IteratorControl = Deque::DequeIterator<Controllable, sizeElemMap>;

    MyDeque deqContr1;

    deqContr1.emplace_front(0, 0.f, (std::string{ "element0" }));
    deqContr1.emplace_front(1, 1.f, (std::string{ "element1" }));
    deqContr1.emplace_front(2, 2.f, (std::string{ "element2" }));
    EXPECT_TRUE(deqContr1.size() == 3);
    EXPECT_EQ(deqContr1.capasity(), 27);
    EXPECT_TRUE(deqContr1.getSizeMap() == 3);
    EXPECT_TRUE(deqContr1.getFirstBlock() == 1);
    EXPECT_TRUE(deqContr1.getLastBlock() == 1);
    for (size_t i = 3; i < 20; ++i) {
        deqContr1.emplace_front(Controllable(i, static_cast<float>(i), (std::string{ "element" } + std::to_string(i))));
    }
    EXPECT_TRUE(deqContr1.size() == 20);
    EXPECT_EQ(deqContr1.capasity(), 54);
    EXPECT_TRUE(deqContr1.getSizeMap() == 6);
    EXPECT_TRUE(deqContr1.getFirstBlock() == 1);
    EXPECT_TRUE(deqContr1.getLastBlock() == 3);

    Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap> contrInt1;
    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70 });
    auto itBegin = std::crbegin(vecInt1);
    //contrInt1.push_back(77);
    for (size_t i = 1; i < 5; ++i) {
        contrInt1.emplace_front(*itBegin);
        ++itBegin;
    }
    EXPECT_EQ(contrInt1.size(), 4);
    EXPECT_EQ(contrInt1.capasity(), 27);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);

    for (size_t i = 0; i < 20; ++i) {
        contrInt1.emplace_front(*itBegin);
        ++itBegin;
    }
    EXPECT_EQ(contrInt1.size(), 24);
    EXPECT_EQ(contrInt1.capasity(), 54);
    EXPECT_TRUE(contrInt1.getSizeMap() == 6);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 0);
    EXPECT_TRUE(contrInt1.getLastBlock() == 3);
    //std::ranges::copy(contrInt1, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << "\n";
    auto itDeqEnd = std::cbegin(contrInt1);
    for (size_t i = 0; i < 24; ++i) {
        //std::cout << "("<< *itDeqEnd << "; " << i << ")\n";
        EXPECT_EQ(*itDeqEnd, 93 - i);
        ++itDeqEnd;
    }
}

TEST(MyDequeMethodTest, emplase_frontMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 10;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;

        MyDequePMRAllocThrowElem dequeThrow1(2, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 2);
        EXPECT_EQ(dequeThrow1.capasity(), 30);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 17;
        ThrowingElement::max_moves_elem = 20;
        //std::cout << dequeThrow1.begin().getFirstI() << " " << dequeThrow1.begin().getFirstJ() << '\n';
        EXPECT_NO_THROW({ for (size_t i = 0; i < 4; ++i) {
            //std::cout << "ThrowingElement::construct_count - " << ThrowingElement::construct_count << "\n";
            dequeThrow1.emplace_front(ThrowingElement());
        }; });
        //std::cout << dequeThrow1.begin().getFirstI() << " " << dequeThrow1.begin().getFirstJ() << '\n';
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 30);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_moves_elem = 1;
        ThrowingElement::max_success_allocs = 1;
        EXPECT_THROW({ dequeThrow1.emplace_front(ThrowingElement()); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 30);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 30;
        ThrowingElement::max_moves_elem = 20;
        EXPECT_NO_THROW({ dequeThrow1.emplace_front(ThrowingElement()); });
        EXPECT_EQ(dequeThrow1.size(), 7);
    }
    ThrowingElement::construct_count = 0;
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        contrContr1.resize(9);
        EXPECT_EQ(contrContr1.size(), 9);
        EXPECT_EQ(contrContr1.capasity(), 30);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        std::ranges::transform(contrContr1, std::begin(contrContr1), [](auto&& it) {static int r = 0; r++;
        return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
            });
        //std::cout << contrContr1.begin().getFirstI() << " " << contrContr1.begin().getFirstJ() << '\n';
        EXPECT_NO_THROW({for (size_t i = 0; i < 5; ++i) {
            //std::cout << "ThrowingElement::construct_count - " << ThrowingElement::construct_count << "\n";
            contrContr1.emplace_front(Controllable(10 + i, static_cast<float>(10 + i), (std::string{ "element" } + std::to_string(10 + i))));
        }; });
        EXPECT_EQ(contrContr1.size(), 14);
        EXPECT_EQ(contrContr1.capasity(), 30);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        //std::cout << contrContr1.begin().getFirstI() << " " << contrContr1.begin().getFirstJ() << '\n';

        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({ contrContr1.emplace_front(Controllable(15, 15.f, (std::string{ "element15" }))); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 14);
        EXPECT_EQ(contrContr1.capasity(), 30);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);

        //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
        //std::cout << "\n";
    }
    {
        MyDequePmrAllocInt dequeInt1(2, int{}, allocInt);
        EXPECT_NO_THROW({ for (size_t i = 0; i < 40; ++i) {
            dequeInt1.emplace_front(i);
        }; });
        //std::ranges::copy(dequeInt1, std::ostream_iterator<int>(std::cout, " "));
        EXPECT_EQ(dequeInt1.size(), 42);
        EXPECT_EQ(dequeInt1.capasity(), 120);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 12);
    EXPECT_EQ(mockRes.allocate_calls, 12);
}

TEST(MyDequeMethodTest, emplase_backMetodTrivialMath) {
    const constexpr int sizeElemMap = 5;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDeque = Deque::MyDeque<Controllable, MyPmrAllocator, sizeElemMap>;
    using ConstReversIteratorControl = std::reverse_iterator <Deque::DequeIterator<Controllable, sizeElemMap, true>>;
    using IteratorControl = Deque::DequeIterator<Controllable, sizeElemMap>;

    MyDeque deqContr1;

    deqContr1.emplace_back(0, 0.f, (std::string{ "element0" }));
    EXPECT_TRUE(deqContr1.size() == 1);
    EXPECT_EQ(deqContr1.capasity(), 15);
    EXPECT_TRUE(deqContr1.getSizeMap() == 3);
    EXPECT_TRUE(deqContr1.getFirstBlock() == 1);
    EXPECT_TRUE(deqContr1.getLastBlock() == 1);

    for (size_t i = 1; i < 14; ++i) {
        deqContr1.emplace_back(Controllable(i, static_cast<float>(i), (std::string{ "element" } + std::to_string(i))));
    }
    EXPECT_TRUE(deqContr1.size() == 14);
    EXPECT_EQ(deqContr1.capasity(), 30);
    EXPECT_TRUE(deqContr1.getSizeMap() == 6);
    EXPECT_TRUE(deqContr1.getFirstBlock() == 2);
    EXPECT_TRUE(deqContr1.getLastBlock() == 5);
    //std::ranges::copy(deqContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
    //std::cout << "\n";

    Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap> contrInt1;
    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80 });
    auto itBegin = std::begin(vecInt1);
    //contrInt1.push_back(77);
    for (size_t i = 1; i < 4; ++i) {
        contrInt1.emplace_back(*itBegin);
        ++itBegin;
    }
    EXPECT_EQ(contrInt1.size(), 3);
    EXPECT_EQ(contrInt1.capasity(), 15);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);

    //itBegin += 8;
    for (size_t i = 0; i < 8; ++i) {
        contrInt1.emplace_back(*itBegin);
        ++itBegin;
    }
    EXPECT_EQ(contrInt1.size(), 11);
    EXPECT_EQ(contrInt1.capasity(), 30);
    EXPECT_TRUE(contrInt1.getSizeMap() == 6);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 2);
    EXPECT_TRUE(contrInt1.getLastBlock() == 4);
    //std::ranges::copy(contrInt1, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << "\n";
    auto itDeqEnd = std::crbegin(contrInt1);
    for (size_t i = 0; i < 11; ++i) {
        //std::cout << "("<< *itDeqEnd << "; " << i << ")\n";
        EXPECT_EQ(*itDeqEnd, 89 + i);
        ++itDeqEnd;
    }
}

TEST(MyDequeMethodTest, emplase_backMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 10;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;

        MyDequePMRAllocThrowElem dequeThrow1(2, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 2);
        EXPECT_EQ(dequeThrow1.capasity(), 30);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 17;
        ThrowingElement::max_moves_elem = 20;
        //std::cout << dequeThrow1.begin().getFirstI() << " " << dequeThrow1.begin().getFirstJ() << '\n';
        EXPECT_NO_THROW({ for (size_t i = 0; i < 4; ++i) {
            //std::cout << "ThrowingElement::construct_count - " << ThrowingElement::construct_count << "\n";
            dequeThrow1.emplace_back(ThrowingElement());
        }; });
        //std::cout << dequeThrow1.end().getFirstI() << " " << dequeThrow1.end().getFirstJ() << '\n';
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 30);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_moves_elem = 1;
        ThrowingElement::max_success_allocs = 1;
        EXPECT_THROW({ dequeThrow1.emplace_back(ThrowingElement()); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 30);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 30;
        ThrowingElement::max_moves_elem = 20;
        EXPECT_NO_THROW({ dequeThrow1.emplace_back(ThrowingElement()); });
        EXPECT_EQ(dequeThrow1.size(), 7);
    }
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        contrContr1.resize(9);
        EXPECT_EQ(contrContr1.size(), 9);
        EXPECT_EQ(contrContr1.capasity(), 30);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        std::ranges::transform(contrContr1, std::begin(contrContr1), [](auto&& it) {static int r = 0; r++;
        return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
            });
        //std::cout << contrContr1.end().getFirstI() << " " << contrContr1.end().getFirstJ() << '\n';
        EXPECT_NO_THROW({ for (size_t i = 0; i < 6; ++i) {
            //std::cout << "ThrowingElement::construct_count - " << ThrowingElement::construct_count << "\n";
            contrContr1.emplace_back(Controllable(10 + i, static_cast<float>(10 + i), (std::string{ "element" } + std::to_string(10 + i))));
        }; });
        EXPECT_EQ(contrContr1.size(), 15);
        EXPECT_EQ(contrContr1.capasity(), 30);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        //std::cout << contrContr1.end().getFirstI() << " " << contrContr1.end().getFirstJ() << '\n';

        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({ contrContr1.emplace_back(Controllable(15, 15.f, (std::string{ "element15" }))); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 15);
        EXPECT_EQ(contrContr1.capasity(), 30);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);

        //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
        //std::cout << "\n";
    }
    {
        MyDequePmrAllocInt dequeInt1(2, int{}, allocInt);
        EXPECT_NO_THROW({ for (size_t i = 0; i < 40; ++i) {
            dequeInt1.emplace_back(i);
        }; });
        //std::ranges::copy(dequeInt1, std::ostream_iterator<int>(std::cout, " "));
        EXPECT_EQ(dequeInt1.size(), 42);
        EXPECT_EQ(dequeInt1.capasity(), 120);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 12);
    EXPECT_EQ(mockRes.allocate_calls, 12);
}

TEST(MyDequeMethodTest, emplaseMetodTrivialMath) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using MyPmrAllocator = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyDeque = Deque::MyDeque<Controllable, MyPmrAllocator, sizeElemMap>;
    using ConstReversIteratorControl = std::reverse_iterator <Deque::DequeIterator<Controllable, sizeElemMap, true>>;
    using IteratorControl = Deque::DequeIterator<Controllable, sizeElemMap>;

    MyDeque deqContr1;
    deqContr1.emplace(std::begin(deqContr1), 0, 0.f, (std::string{ "element0" }));
    deqContr1.emplace(std::begin(deqContr1) + 1, 1, 1.f, (std::string{ "element1" }));
    EXPECT_TRUE(deqContr1.size() == 2);
    EXPECT_EQ(deqContr1.capasity(), 27);
    EXPECT_TRUE(deqContr1.getSizeMap() == 3);
    EXPECT_TRUE(deqContr1.getFirstBlock() == 1);
    EXPECT_TRUE(deqContr1.getLastBlock() == 1);
    for (size_t i = 1; i < 18; ++i) {
        deqContr1.emplace(std::begin(deqContr1) + i, i, static_cast<float>(i), (std::string{ "element" } + std::to_string(i)));
    }
    EXPECT_TRUE(deqContr1.size() == 19);
    EXPECT_EQ(deqContr1.capasity(), 54);
    EXPECT_TRUE(deqContr1.getSizeMap() == 6);
    EXPECT_TRUE(deqContr1.getFirstBlock() == 2);
    EXPECT_TRUE(deqContr1.getLastBlock() == 4);
    EXPECT_EQ(*(std::rbegin(deqContr1)), Controllable(1, 1.f, std::string("element1")));
    EXPECT_EQ(*(std::begin(deqContr1) + 7), Controllable(7, 7.f, std::string("element7")));
    EXPECT_EQ(*(std::rbegin(deqContr1) + 5), Controllable(13, 13.f, std::string("element13")));
    //std::ranges::copy(deqContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
    //std::cout << "\n";

    Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap> contrInt1;
    std::vector<int> vecInt1({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70 });
    auto itBegin = std::crbegin(vecInt1);
    auto itBeginDeq = std::cbegin(contrInt1);
    //contrInt1.push_back(77);
    for (size_t i = 1; i < 5; ++i) {
        contrInt1.emplace(itBeginDeq, *itBegin);
        ++itBegin;
    }
    EXPECT_EQ(contrInt1.size(), 4);
    EXPECT_EQ(contrInt1.capasity(), 27);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);

    for (size_t i = 0; i < 20; ++i) {
        contrInt1.emplace_front(*itBegin);
        ++itBegin;
    }
    EXPECT_EQ(contrInt1.size(), 24);
    EXPECT_EQ(contrInt1.capasity(), 54);
    EXPECT_TRUE(contrInt1.getSizeMap() == 6);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 3);
    //std::ranges::copy(contrInt1, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << "\n";
    auto itDeqEnd = std::cbegin(contrInt1);
    ++itDeqEnd;
    for (size_t i = 0; i < 20; ++i) {
        //std::cout << "("<< *itDeqEnd << "; " << i << ")\n";
        EXPECT_EQ(*itDeqEnd, 93 - i - 1);
        ++itDeqEnd;
    }
}

TEST(MyDequeMethodTest, emplaseMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;

        MyDequePMRAllocThrowElem dequeThrow1(2, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 2);
        EXPECT_EQ(dequeThrow1.capasity(), 27);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 25;
        ThrowingElement::max_moves_elem = 25;
        //std::cout << ThrowingElement::max_success_allocs << " " << ThrowingElement::max_moves_elem << '\n';
        EXPECT_NO_THROW({ for (size_t i = 0; i < 7; ++i) {
            //std::cout << "ThrowingElement::max_moves_elem - " << ThrowingElement::construct_count << "\n";
            dequeThrow1.emplace(std::begin(dequeThrow1) + i, ThrowingElement());
        }; });
        EXPECT_EQ(dequeThrow1.size(), 9);
        EXPECT_EQ(dequeThrow1.capasity(), 27);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 10;
        //ThrowingElement::max_success_allocs = 10;
        EXPECT_THROW({dequeThrow1.emplace(std::cbegin(dequeThrow1), ThrowingElement());}, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 9);
        EXPECT_EQ(dequeThrow1.capasity(), 27);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 25;
        //ThrowingElement::max_success_allocs = 25;
        EXPECT_NO_THROW({ dequeThrow1.emplace(std::begin(dequeThrow1) + 6, ThrowingElement()); });
        EXPECT_EQ(dequeThrow1.size(), 10);
    }
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        contrContr1.resize(11);
        EXPECT_EQ(contrContr1.size(), 11);
        EXPECT_EQ(contrContr1.capasity(), 27);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        std::ranges::transform(contrContr1, std::begin(contrContr1), [](auto&& it) {static int r = 0; r++;
        return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
            });
        //std::cout << "Start (" << contrContr1.cbegin().getFirstI() << " ; " << contrContr1.cbegin().getFirstJ() << ")   Finish (" 
        //    << contrContr1.cend().getFirstI() << " ; " << contrContr1.cend().getFirstJ() << ")\n";
        EXPECT_NO_THROW({for (size_t i = 0; i < 3; ++i) {
            contrContr1.emplace((std::rbegin(contrContr1) + i).base(), 10 + i, static_cast<float>(10 + i), (std::string{ "element" } + std::to_string(10 + i)));
        }; });
        EXPECT_EQ(contrContr1.size(), 14);
        EXPECT_EQ(contrContr1.capasity(), 27);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);

        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({contrContr1.emplace((std::crbegin(contrContr1) + 1).base(), 14, 14.f, (std::string{"element14"}));}, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 14);
        EXPECT_EQ(contrContr1.capasity(), 27);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);

        //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
        //std::cout << "\n";
    }
    {
        MyDequePmrAllocInt dequeInt1(2, int{}, allocInt);
        EXPECT_NO_THROW({ for (size_t i = 0; i < 40; ++i) {
            dequeInt1.emplace(std::begin(dequeInt1) + i + 1, i);
        }; });
        //std::ranges::copy(dequeInt1, std::ostream_iterator<int>(std::cout, " "));
        EXPECT_EQ(dequeInt1.size(), 42);
        EXPECT_EQ(dequeInt1.capasity(), 90);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 241);
    EXPECT_EQ(mockRes.allocate_calls, 241);
}

TEST(MyDequeMethodTest, eraseMetodTrivialMath) {
    const constexpr int sizeElemMap = 11;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")), Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")), Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")), Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")), Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")), Controllable(10, 10.f, std::string("element10")), 
        Controllable(11, 11.f, std::string("element11")), Controllable(12, 12.f, std::string("element12")), 
        Controllable(13, 13.f, std::string("element13")) });
    EXPECT_EQ(*(std::cbegin(contrContr1) + 1), Controllable(2, 2.f, std::string("element2")));
    EXPECT_EQ(*(std::crbegin(contrContr1) + 1), Controllable(12, 12.f, std::string("element12")));
    EXPECT_EQ(contrContr1.size(), 13);
    EXPECT_EQ(contrContr1.capasity(), 44);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    //auto itBegContr = std::cbegin(contrContr1);
    contrContr1.erase(std::cbegin(contrContr1) + 1);
    contrContr1.erase((std::crbegin(contrContr1) + 2).base());
    EXPECT_EQ(*(std::cbegin(contrContr1) + 1), Controllable(3, 3.f, std::string("element3")));
    EXPECT_EQ(*(std::crbegin(contrContr1) + 1), Controllable(11, 11.f, std::string("element11")));
    EXPECT_EQ(contrContr1.size(), 11);
    EXPECT_EQ(contrContr1.capasity(), 44);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);

    Deque::MyDeque<int, std::allocator<int>, sizeElemMap> contrInt1(0);
    EXPECT_EQ(contrInt1.size(), 0);
    EXPECT_EQ(contrInt1.capasity(), 33);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);

    EXPECT_EQ(contrInt1.erase(contrInt1.cbegin()), contrInt1.begin());
    EXPECT_EQ(contrInt1.size(), 0);
    EXPECT_EQ(contrInt1.capasity(), 33);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);
}

TEST(MyDequeMethodTest, eraseMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;

        MyDequePMRAllocThrowElem dequeThrow1(8, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 8);
        EXPECT_EQ(dequeThrow1.capasity(), 28);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 25;
        //std::cout << ThrowingElement::max_success_allocs << " " << ThrowingElement::max_moves_elem << '\n';
        EXPECT_NO_THROW({ for (size_t i = 0; i < 2; ++i) {
            //std::cout << "ThrowingElement::max_moves_elem - " << ThrowingElement::construct_count << "\n";
            dequeThrow1.erase(std::begin(dequeThrow1) + i + 1);
        }; });
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 28);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 2;
        EXPECT_THROW({ dequeThrow1.erase(std::begin(dequeThrow1) + 4); }, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 28);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 25;
        EXPECT_NO_THROW({ dequeThrow1.erase(std::begin(dequeThrow1) + 1); });
        EXPECT_EQ(dequeThrow1.size(), 5);
    }
    ThrowingElement::construct_count = 0;
    {
        MyDequePmrAllocInt dequeInt1(allocInt);
        dequeInt1.reserve(40);
        std::ranges::generate_n(std::back_inserter(dequeInt1), 40, []() mutable {static int r = 0; return ++r; });
        EXPECT_EQ(dequeInt1.size(), 40);
        EXPECT_EQ(dequeInt1.capasity(), 84);
        auto itBeg = dequeInt1.cbegin();
        for (size_t i = 0; i < 8; ++i) {
            EXPECT_EQ(*itBeg, i + 1);
            ++itBeg;
        }
        EXPECT_NO_THROW({ for (size_t i = 0; i < 8; ++i) {
            dequeInt1.erase(std::begin(dequeInt1) + i + 1);
        }; });
        itBeg = dequeInt1.cbegin();
        for (size_t i = 0; i < 8; ++i) {
            EXPECT_EQ(*itBeg, 2 * i + 1);
            ++itBeg;
        }
        //std::ranges::copy(dequeInt1, std::ostream_iterator<int>(std::cout, " "));
        EXPECT_EQ(dequeInt1.size(), 32);
        EXPECT_EQ(dequeInt1.capasity(), 84);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 13);
    EXPECT_EQ(mockRes.allocate_calls, 13);
}

TEST(MyDequeMethodTest, eraseMetodItheratorMath) {
    const constexpr int sizeElemMap = 13;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")), Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")), Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")), Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")), Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")), Controllable(10, 10.f, std::string("element10")),
        Controllable(11, 11.f, std::string("element11")), Controllable(12, 12.f, std::string("element12")),
        Controllable(13, 13.f, std::string("element13")), Controllable(14, 14.f, std::string("element14")),
        Controllable(15, 15.f, std::string("element15")) });
    EXPECT_EQ(*(std::cbegin(contrContr1) + 1), Controllable(2, 2.f, std::string("element2")));
    EXPECT_EQ(*(std::crbegin(contrContr1) + 1), Controllable(14, 14.f, std::string("element14")));
    EXPECT_EQ(contrContr1.size(), 15);
    EXPECT_EQ(contrContr1.capasity(), 52);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    contrContr1.erase(std::cbegin(contrContr1), std::cbegin(contrContr1) + 2);
    contrContr1.erase((std::crbegin(contrContr1) + 2).base(), std::crbegin(contrContr1).base());
    EXPECT_EQ(*(std::cbegin(contrContr1) + 1), Controllable(4, 4.f, std::string("element4")));
    EXPECT_EQ(*(std::crbegin(contrContr1) + 1), Controllable(12, 12.f, std::string("element12")));
    EXPECT_EQ(contrContr1.size(), 11);
    EXPECT_EQ(contrContr1.capasity(), 52);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    contrContr1.erase(std::begin(contrContr1), std::cend(contrContr1));
    EXPECT_EQ(contrContr1.size(), 0);
    EXPECT_EQ(contrContr1.capasity(), 52);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);

    Deque::MyDeque<int, std::allocator<int>, sizeElemMap> contrInt1(0);
    EXPECT_EQ(contrInt1.size(), 0);
    EXPECT_EQ(contrInt1.capasity(), 39);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);

    EXPECT_EQ(contrInt1.erase(contrInt1.cbegin(), contrInt1.cend()), contrInt1.begin());
    EXPECT_EQ(contrInt1.size(), 0);
    EXPECT_EQ(contrInt1.capasity(), 39);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);
}

TEST(MyDequeMethodTest, eraseMetodItheratorAllocate) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);
    {
        ThrowingElement::max_success_allocs = 18;

        MyDequePMRAllocThrowElem dequeThrow1(9, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 9);
        EXPECT_EQ(dequeThrow1.capasity(), 36);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 25;
        //std::cout << ThrowingElement::max_success_allocs << " " << ThrowingElement::max_moves_elem << '\n';
        EXPECT_NO_THROW({ dequeThrow1.erase(std::begin(dequeThrow1) + 1, std::begin(dequeThrow1) + 4);});
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 36);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 2;
        EXPECT_THROW({dequeThrow1.erase(std::begin(dequeThrow1) + 1, std::begin(dequeThrow1) + 3);}, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 36);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 35;
        EXPECT_NO_THROW({ dequeThrow1.erase((std::rbegin(dequeThrow1) + 3).base(), (std::rbegin(dequeThrow1) + 1).base()); });
        EXPECT_THROW({ dequeThrow1.erase((std::rbegin(dequeThrow1) + 1).base(), (std::rbegin(dequeThrow1) + 2).base()); }, std::length_error);
        EXPECT_EQ(dequeThrow1.size(), 4);
    }
    ThrowingElement::construct_count = 0;
    {
        MyDequePmrAllocInt dequeInt1(allocInt);
        dequeInt1.reserve(40);
        std::ranges::generate_n(std::back_inserter(dequeInt1), 40, []() mutable {static int r = 0; return ++r; });
        EXPECT_EQ(dequeInt1.size(), 40);
        EXPECT_EQ(dequeInt1.capasity(), 90);
        auto itBeg = dequeInt1.cbegin();
        auto itRBeg = dequeInt1.crbegin();
        for (size_t i = 0; i < 5; ++i) {
            EXPECT_EQ(*itBeg, i + 1);
            ++itBeg;
        }
        for (size_t i = 0; i < 5; ++i) {
            EXPECT_EQ(*itRBeg, 40 - i);
            ++itRBeg;
        }
        EXPECT_NO_THROW({ dequeInt1.erase(std::begin(dequeInt1), std::begin(dequeInt1) + 5);});
        EXPECT_NO_THROW({ dequeInt1.erase((std::rbegin(dequeInt1) + 5).base(), (std::rbegin(dequeInt1)).base()); });
        itBeg = dequeInt1.cbegin();
        itRBeg = dequeInt1.crbegin();
        for (size_t i = 0; i < 5; ++i) {
            EXPECT_EQ(*itBeg, i + 6);
            ++itBeg;
        }
        for (size_t i = 0; i < 5; ++i) {
            EXPECT_EQ(*itRBeg, 35 - i);
            ++itRBeg;
        }
        //std::ranges::copy(dequeInt1, std::ostream_iterator<int>(std::cout, " "));
        EXPECT_EQ(dequeInt1.size(), 30);
        EXPECT_EQ(dequeInt1.capasity(), 90);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 11);
    EXPECT_EQ(mockRes.allocate_calls, 11);
}

TEST(MyDequeMethodTest, reserveMetodTrivialMath) {
    const constexpr int sizeElemMap = 10;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;

    std::deque <Controllable> deqContr1({
        Controllable(1, 1.f, std::string("element1")), Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")), Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")), Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")), Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")), Controllable(10, 10.f, std::string("element10")),
        Controllable(11, 11.f, std::string("element11")), Controllable(12, 12.f, std::string("element12")) });
    MyDequePmrAllocContr contrContr1 (0);
    EXPECT_EQ(contrContr1.size(), 0);
    EXPECT_EQ(contrContr1.capasity(), 30);
    EXPECT_TRUE(contrContr1.getSizeMap() == 3);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 1);
    contrContr1.reserve(46);
    EXPECT_EQ(contrContr1.size(), 0);
    EXPECT_EQ(contrContr1.capasity(), 50);
    EXPECT_TRUE(contrContr1.getSizeMap() == 5);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    contrContr1.insert(std::begin(contrContr1), std::begin(deqContr1), std::end(deqContr1));
    EXPECT_EQ(contrContr1.size(), 12);
    EXPECT_EQ(contrContr1.capasity(), 50);
    EXPECT_TRUE(contrContr1.getSizeMap() == 5);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 3);
    contrContr1.reserve(0);
    EXPECT_EQ(contrContr1.size(), 12);
    EXPECT_EQ(contrContr1.capasity(), 50);
    EXPECT_TRUE(contrContr1.getSizeMap() == 5);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 3);
    contrContr1.reserve(25);
    EXPECT_EQ(contrContr1.size(), 12);
    EXPECT_EQ(contrContr1.capasity(), 50);
    EXPECT_TRUE(contrContr1.getSizeMap() == 5);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 3);
    //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
    //std::cout << "\n";
}

TEST(MyDequeMethodTest, reserveMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 7;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);
    {
        ThrowingElement::max_success_allocs = 16;

        MyDequePMRAllocThrowElem dequeThrow1(6, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 21);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 3);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 1);
        ThrowingElement::max_success_allocs = 25;
        ThrowingElement::max_moves_elem = 25;
        //std::cout << ThrowingElement::max_success_allocs << " " << ThrowingElement::max_moves_elem << '\n';
        EXPECT_NO_THROW({ dequeThrow1.reserve(30); });
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 35);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 5);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 2);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 3;
        ThrowingElement::max_success_allocs = 3;
        EXPECT_NO_THROW({ dequeThrow1.reserve(40); });
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 42);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 6);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 2);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
    }
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        contrContr1.resize(9);
        EXPECT_EQ(contrContr1.size(), 9);
        EXPECT_EQ(contrContr1.capasity(), 21);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        std::ranges::transform(contrContr1, std::begin(contrContr1), [](auto&& it) {static int r = 0; r++;
        return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
            });
        //std::cout << "Start (" << contrContr1.cbegin().getFirstI() << " ; " << contrContr1.cbegin().getFirstJ() << ")   Finish (" 
        //    << contrContr1.cend().getFirstI() << " ; " << contrContr1.cend().getFirstJ() << ")\n";
        EXPECT_NO_THROW({ contrContr1.reserve(33); });
        EXPECT_EQ(contrContr1.size(), 9);
        EXPECT_EQ(contrContr1.capasity(), 35);
        EXPECT_TRUE(contrContr1.getSizeMap() == 5);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);

        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({ contrContr1.reserve(50); }, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 9);
        EXPECT_EQ(contrContr1.capasity(), 35);
        EXPECT_TRUE(contrContr1.getSizeMap() == 5);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
        //std::cout << "\n";
    }
    {
        MyDequePmrAllocInt dequeInt1(30, int{}, allocInt);
        EXPECT_NO_THROW({ dequeInt1.reserve(89); });
        //std::ranges::copy(dequeInt1, std::ostream_iterator<int>(std::cout, " "));
        EXPECT_EQ(dequeInt1.size(), 30);
        EXPECT_EQ(dequeInt1.capasity(), 91);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 11);
    EXPECT_EQ(mockRes.allocate_calls, 11);
}

TEST(MyDequeMethodTest, resizeMetodTrivialMath) {
    const constexpr int sizeElemMap = 14;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")), Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")), Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")), Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")), Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")), Controllable(10, 10.f, std::string("element10")),
        Controllable(11, 11.f, std::string("element11")), Controllable(12, 12.f, std::string("element12")),
        Controllable(13, 13.f, std::string("element13")), Controllable(14, 14.f, std::string("element14")),
        Controllable(15, 15.f, std::string("element15")), Controllable(16, 16.f, std::string("element16")) });
    EXPECT_EQ(*(std::crbegin(contrContr1)), Controllable(16, 16.f, std::string("element16")));
    EXPECT_EQ(contrContr1.size(), 16);
    EXPECT_EQ(contrContr1.capasity(), 56);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    contrContr1.resize(20, Controllable(0, 0.f, std::string("element0")));
    EXPECT_EQ(*(std::crbegin(contrContr1)), Controllable(0, 0.f, std::string("element0")));
    EXPECT_EQ(contrContr1.size(), 20);
    EXPECT_EQ(contrContr1.capasity(), 56);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    contrContr1.resize(10);
    EXPECT_EQ(*(std::crbegin(contrContr1)), Controllable(10, 10.f, std::string("element10")));
    EXPECT_EQ(contrContr1.size(), 10);
    EXPECT_EQ(contrContr1.capasity(), 56);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
    //std::cout << "\n";

    Deque::MyDeque<int, std::allocator<int>, sizeElemMap> contrInt1(0);
    EXPECT_EQ(contrInt1.size(), 0);
    EXPECT_EQ(contrInt1.capasity(), 42);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 1);
    contrInt1.resize(20);
    std::ranges::transform(contrInt1, std::begin(contrInt1), [](auto&& it) {static int i = 0; return ++i; });
    EXPECT_EQ(contrInt1[16], 17);
    EXPECT_EQ(contrInt1.size(), 20);
    EXPECT_EQ(contrInt1.capasity(), 42);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 2);
    contrInt1.resize(10);
    EXPECT_EQ(contrInt1.size(), 10);
    EXPECT_EQ(contrInt1.capasity(), 42);
    EXPECT_TRUE(contrInt1.getSizeMap() == 3);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 2);
    contrInt1.resize(40);
    EXPECT_EQ(contrInt1[16], 0);
    EXPECT_EQ(contrInt1.size(), 40);
    EXPECT_EQ(contrInt1.capasity(), 84);
    EXPECT_TRUE(contrInt1.getSizeMap() == 6);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 2);
    EXPECT_TRUE(contrInt1.getLastBlock() == 5);
}

TEST(MyDequeMethodTest, resizeMetodTrivialAllocate) {
    const constexpr int sizeElemMap = 8;

    using Controllable = userClass::Controllable;
    using ThrowingElement = userClass::ThrowingElement;
    using ExeptAllocatorInt = FaultyAllocator::ExeptAllocator<int>;
    using FaultyAlloc = FaultyAllocator::FaultyAllocator<int>;

    using MockMemoryResource = FaultyAllocator::MockMemoryResource;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;
    using MyPmrAllocatorInt = PmrAllocator::MyPmrAllocator<int>;
    using MyPmrAllocatorThrow = PmrAllocator::MyPmrAllocator<ThrowingElement>;

    using MyDequeExceptAllocInt = Deque::MyDeque<int, ExeptAllocatorInt, sizeElemMap>;
    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    using MyDequePmrAllocInt = Deque::MyDeque<int, MyPmrAllocatorInt, sizeElemMap>;
    using MyDequePMRAllocThrowElem = Deque::MyDeque<ThrowingElement, MyPmrAllocatorThrow, sizeElemMap>;

    MockMemoryResource mockRes;
    MyPmrAllocatorContr allocContr(&mockRes);
    MyPmrAllocatorInt allocInt(&mockRes);
    MyPmrAllocatorThrow allocThrow(&mockRes);

    {
        ThrowingElement::max_success_allocs = 16;

        MyDequePMRAllocThrowElem dequeThrow1(9, ThrowingElement(), allocThrow);
        EXPECT_EQ(dequeThrow1.size(), 9);
        EXPECT_EQ(dequeThrow1.capasity(), 32);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_success_allocs = 25;
        ThrowingElement::max_moves_elem = 25;
        //std::cout << ThrowingElement::max_success_allocs << " " << ThrowingElement::max_moves_elem << '\n';
        EXPECT_NO_THROW({ dequeThrow1.resize(6); });
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 32);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 4);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 1);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 2);
        ThrowingElement::max_moves_elem = 10;
        ThrowingElement::max_success_allocs = 10;
        EXPECT_THROW({ dequeThrow1.resize(38);}, std::runtime_error);
        EXPECT_EQ(dequeThrow1.size(), 6);
        EXPECT_EQ(dequeThrow1.capasity(), 40);
        EXPECT_TRUE(dequeThrow1.getSizeMap() == 5);
        EXPECT_TRUE(dequeThrow1.getFirstBlock() == 2);
        EXPECT_TRUE(dequeThrow1.getLastBlock() == 3);
        ThrowingElement::max_moves_elem = 25;
        ThrowingElement::max_success_allocs = 25;
    }
    FaultyAllocator::BadMemoryResource badResource;
    MyPmrAllocatorContr allocContrBad(&badResource);
    {
        FaultyAllocator::BadMemoryResource::max_size = 10000;
        MyDequePmrAllocContr contrContr1(allocContrBad);
        contrContr1.resize(11);
        EXPECT_EQ(contrContr1.size(), 11);
        EXPECT_EQ(contrContr1.capasity(), 24);
        EXPECT_TRUE(contrContr1.getSizeMap() == 3);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
        EXPECT_TRUE(contrContr1.getLastBlock() == 2);
        std::ranges::transform(contrContr1, std::begin(contrContr1), [](auto&& it) {static int r = 0; r++;
        return (Controllable(r, static_cast<float>(r), (std::string{ "element" } + std::to_string(r))));
            });
        //std::cout << "Start (" << contrContr1.cbegin().getFirstI() << " ; " << contrContr1.cbegin().getFirstJ() << ")   Finish (" 
        //    << contrContr1.cend().getFirstI() << " ; " << contrContr1.cend().getFirstJ() << ")\n";
        EXPECT_NO_THROW({ contrContr1.resize(13, Controllable(0, 0.f, std::string{ "element0" })); });
        EXPECT_EQ(contrContr1.size(), 13);
        EXPECT_EQ(contrContr1.capasity(), 48);
        EXPECT_TRUE(contrContr1.getSizeMap() == 6);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
        EXPECT_TRUE(contrContr1.getLastBlock() == 4);

        FaultyAllocator::BadMemoryResource::max_size = 1;
        EXPECT_THROW({ contrContr1.resize(59, Controllable(0, 0.f, std::string{ "element0" }));}, std::bad_alloc);
        EXPECT_EQ(contrContr1.size(), 13);
        EXPECT_EQ(contrContr1.capasity(), 48);
        EXPECT_TRUE(contrContr1.getSizeMap() == 6);
        EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
        EXPECT_TRUE(contrContr1.getLastBlock() == 4);

        //std::ranges::copy(contrContr1 | std::views::transform([](const Controllable& u) { return u.f; }), std::ostream_iterator<float>(std::cout, " "));
        //std::cout << "\n";
    }
    {
        MyDequePmrAllocInt dequeInt1(2, 2, allocInt);
        EXPECT_NO_THROW({ dequeInt1.resize(60); });
        EXPECT_EQ(dequeInt1 [1], 2);
        EXPECT_EQ(dequeInt1.size(), 60);
        EXPECT_EQ(dequeInt1.capasity(), 128);
        EXPECT_NO_THROW({ dequeInt1.resize(2); });
        EXPECT_EQ(dequeInt1[1], 2);
        EXPECT_EQ(dequeInt1.size(), 2);
        EXPECT_EQ(dequeInt1.capasity(), 128);
    }

    EXPECT_EQ(mockRes.active_allocations, 0);
    EXPECT_EQ(mockRes.deallocate_calls, 15);
    EXPECT_EQ(mockRes.allocate_calls, 15);
}

TEST(MyDequeMethodTest, clearMetodTrivialMath) {
    const constexpr int sizeElemMap = 9;

    using Controllable = userClass::Controllable;
    using MyPmrAllocatorContr = PmrAllocator::MyPmrAllocator<Controllable>;

    using MyDequePmrAllocContr = Deque::MyDeque<Controllable, MyPmrAllocatorContr, sizeElemMap>;
    MyDequePmrAllocContr contrContr1({
        Controllable(1, 1.f, std::string("element1")), Controllable(2, 2.f, std::string("element2")),
        Controllable(3, 3.f, std::string("element3")), Controllable(4, 4.f, std::string("element4")),
        Controllable(5, 5.f, std::string("element5")), Controllable(6, 6.f, std::string("element6")),
        Controllable(7, 7.f, std::string("element7")), Controllable(8, 8.f, std::string("element8")),
        Controllable(9, 9.f, std::string("element9")), Controllable(10, 10.f, std::string("element10")),
        Controllable(11, 11.f, std::string("element11")) });
    EXPECT_EQ(contrContr1.size(), 11);
    EXPECT_EQ(contrContr1.capasity(), 36);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 1);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
    EXPECT_NO_THROW({ contrContr1.clear(); });
    EXPECT_EQ(contrContr1.size(), 0);
    EXPECT_EQ(contrContr1.capasity(), 36);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);

    Deque::MyDeque<int, std::allocator<int>, sizeElemMap> contrInt1(20);
    EXPECT_EQ(contrInt1.size(), 20);
    EXPECT_EQ(contrInt1.capasity(), 45);
    EXPECT_TRUE(contrInt1.getSizeMap() == 5);
    EXPECT_TRUE(contrInt1.getFirstBlock() == 1);
    EXPECT_TRUE(contrInt1.getLastBlock() == 3);
    EXPECT_NO_THROW({ contrInt1.clear(); });
    EXPECT_EQ(contrContr1.size(), 0);
    EXPECT_EQ(contrContr1.capasity(), 36);
    EXPECT_TRUE(contrContr1.getSizeMap() == 4);
    EXPECT_TRUE(contrContr1.getFirstBlock() == 2);
    EXPECT_TRUE(contrContr1.getLastBlock() == 2);
}

//{
//    ExeptAllocatorInt::should_throw_on_move = true;
//    bool exception_caught = false;
//
//    try {
//        dequeExcept2 = std::move(dequeExcept1);
//    }
//    catch (const std::exception& e) {
//        exception_caught = true;
//
//        // КРИТИЧЕСКИЙ ШАГ: Немедленно выключаем флаг, как только поймали исключение!
//        ExeptAllocatorInt::should_throw_on_move = false;
//
//        // Чтобы предотвратить Double Free и зависание в деструкторах поврежденных деков,
//        // мы вручную сбрасываем их в безопасное "пустое" состояние прямо здесь.
//        // Благодаря этому деструкторы при выходе из {} не будут трогать память.
//        dequeExcept1.clear();
//        dequeExcept2.clear();
//    }
//
//    // Проверяем, что исключение действительно было выброшено
//    EXPECT_TRUE(exception_caught) << "Expected an exception from allocator move assignment, but none was thrown.";
//}