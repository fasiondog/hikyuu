/*
 * test_IndicatorImpBuffer.cpp
 *
 *  Created on: 2024-01-XX
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "hikyuu/indicator/IndicatorImpBuffer.h"

using namespace hku;

TEST_CASE("test_IndicatorImpBuffer_basic") {
    IndicatorImpBuffer buf;
    CHECK_UNARY(buf.empty());
    CHECK_EQ(buf.size(), 0);

    buf.push_back(1.0);
    buf.push_back(2.0);
    CHECK_FALSE(buf.empty());
    CHECK_EQ(buf.size(), 2);
    CHECK_EQ(buf[0], 1.0);
    CHECK_EQ(buf[1], 2.0);

    buf.clear();
    CHECK_UNARY(buf.empty());
    CHECK_EQ(buf.size(), 0);
}

TEST_CASE("test_IndicatorImpBuffer_constructors") {
    // Default constructor
    IndicatorImpBuffer buf1;
    CHECK_EQ(buf1.size(), 0);

    // Constructor with an initial size
    IndicatorImpBuffer buf2(5);
    CHECK_EQ(buf2.size(), 5);
    CHECK_EQ(buf2[0], 0.0);
    CHECK_EQ(buf2[4], 0.0);

    // Constructor with an initial size and value
    IndicatorImpBuffer buf3(3, 1.5);
    CHECK_EQ(buf3.size(), 3);
    CHECK_EQ(buf3[0], 1.5);
    CHECK_EQ(buf3[2], 1.5);

    // Copy constructor
    IndicatorImpBuffer buf4(buf3);
    CHECK_EQ(buf4.size(), 3);
    CHECK_EQ(buf4[0], 1.5);
    CHECK_EQ(buf4[2], 1.5);

    // Move constructor
    IndicatorImpBuffer buf5(std::move(buf4));
    CHECK_EQ(buf5.size(), 3);
    CHECK_EQ(buf5[0], 1.5);
    CHECK_EQ(buf5[2], 1.5);
}

TEST_CASE("test_IndicatorImpBuffer_assignment") {
    IndicatorImpBuffer buf1(3, 1.0);
    IndicatorImpBuffer buf2(2, 2.0);

    // Copy assignment
    buf2 = buf1;
    CHECK_EQ(buf2.size(), 3);
    CHECK_EQ(buf2[0], 1.0);
    CHECK_EQ(buf2[2], 1.0);

    // Move assignment
    IndicatorImpBuffer buf3(1, 3.0);
    buf3 = std::move(buf2);
    CHECK_EQ(buf3.size(), 3);
    CHECK_EQ(buf3[0], 1.0);
    CHECK_EQ(buf3[2], 1.0);
}

TEST_CASE("test_IndicatorImpBuffer_iterators") {
    IndicatorImpBuffer buf = {1.0, 2.0, 3.0};

    // Test begin/end
    auto it = buf.begin();
    CHECK_EQ(*it, 1.0);
    ++it;
    CHECK_EQ(*it, 2.0);

    // Test the range based for loop
    double sum = 0.0;
    for (const auto& val : buf) {
        sum += val;
    }
    CHECK_EQ(sum, 6.0);
}

TEST_CASE("test_IndicatorImpBuffer_capacity") {
    IndicatorImpBuffer buf;

    CHECK_EQ(buf.size(), 0);
    CHECK_GE(buf.capacity(), 0);

    buf.reserve(10);
    CHECK_GE(buf.capacity(), 10);

    buf.push_back(1.0);
    CHECK_EQ(buf.size(), 1);

    buf.shrink_to_fit();
    CHECK_EQ(buf.capacity(), buf.size());
}

TEST_CASE("test_IndicatorImpBuffer_element_access") {
    IndicatorImpBuffer buf = {1.0, 2.0, 3.0};

    // Test operator[]
    CHECK_EQ(buf[0], 1.0);
    CHECK_EQ(buf[2], 3.0);

    // Test at()
    CHECK_EQ(buf.at(1), 2.0);

    // Test front/back
    CHECK_EQ(buf.front(), 1.0);
    CHECK_EQ(buf.back(), 3.0);

    // Test data()
    CHECK_EQ(buf.data()[1], 2.0);
}

TEST_CASE("test_IndicatorImpBuffer_modifiers") {
    IndicatorImpBuffer buf;

    // Test push_back
    buf.push_back(1.0);
    buf.push_back(2.0);
    buf.push_back(3.0);
    CHECK_EQ(buf.size(), 3);
    CHECK_EQ(buf[0], 1.0);
    CHECK_EQ(buf[2], 3.0);

    // Test emplace_back
    buf.emplace_back(4.0);
    CHECK_EQ(buf.size(), 4);
    CHECK_EQ(buf.back(), 4.0);

    // Test inserting a single element
    auto it = buf.insert(buf.begin() + 1, 1.5);
    CHECK_EQ(buf.size(), 5);
    CHECK_EQ(buf[1], 1.5);
    CHECK_EQ(*it, 1.5);

    // Test inserting multiple elements
    it = buf.insert(buf.begin() + 2, 2, 2.5);
    CHECK_EQ(buf.size(), 7);
    CHECK_EQ(buf[2], 2.5);
    CHECK_EQ(buf[3], 2.5);

    // Test erasing a single element
    it = buf.erase(buf.begin() + 3);
    CHECK_EQ(buf.size(), 6);
    CHECK_EQ(buf[3], 2.0);  // Fixed: the original 4th element (2.5) was erased, so it is 2.0
    CHECK_EQ(*it, 2.0);     // Fixed: the returned iterator points to the next element

    // Test erasing a range
    it = buf.erase(buf.begin() + 1, buf.begin() + 3);
    CHECK_EQ(buf.size(), 4);
    CHECK_EQ(buf[1], 2.0);  // Fixed: after erasing [1.5, 2.5] the 2nd element is 2.0
    CHECK_EQ(*it, 2.0);     // Fixed: the iterator points to the first element after the range

    // Test pop_back
    buf.pop_back();
    CHECK_EQ(buf.size(), 3);
    CHECK_EQ(buf.back(), 3.0);

    // Test clear
    buf.clear();
    CHECK_UNARY(buf.empty());
    CHECK_EQ(buf.size(), 0);
}

TEST_CASE("test_IndicatorImpBuffer_algorithms") {
    IndicatorImpBuffer buf = {3.0, 1.0, 4.0, 1.0, 5.0};

    // Test sort
    buf.sort();
    CHECK_EQ(buf[0], 1.0);
    CHECK_EQ(buf[1], 1.0);
    CHECK_EQ(buf[4], 5.0);

    // Test erase_if
    buf.erase_if([](double x) { return x < 3.0; });
    CHECK_EQ(buf.size(), 3);
    CHECK_EQ(buf[0], 3.0);
    CHECK_EQ(buf[1], 4.0);
    CHECK_EQ(buf[2], 5.0);
}

TEST_CASE("test_IndicatorImpBuffer_swap") {
    IndicatorImpBuffer buf1 = {1.0, 2.0, 3.0};
    IndicatorImpBuffer buf2 = {4.0, 5.0};

    buf1.swap(buf2);

    CHECK_EQ(buf1.size(), 2);
    CHECK_EQ(buf1[0], 4.0);
    CHECK_EQ(buf1[1], 5.0);

    CHECK_EQ(buf2.size(), 3);
    CHECK_EQ(buf2[0], 1.0);
    CHECK_EQ(buf2[2], 3.0);
}

TEST_CASE("test_IndicatorImpBuffer_new_delete") {
    // Test the new operator
    IndicatorImpBuffer* buf = new IndicatorImpBuffer(5, 2.0);
    CHECK_EQ(buf->size(), 5);
    CHECK_EQ((*buf)[0], 2.0);
    delete buf;

    // Test the new[] operator
    IndicatorImpBuffer* bufs = new IndicatorImpBuffer[2];
    bufs[0] = IndicatorImpBuffer(3, 1.0);
    bufs[1] = IndicatorImpBuffer(2, 2.0);
    CHECK_EQ(bufs[0].size(), 3);
    CHECK_EQ(bufs[1].size(), 2);
    delete[] bufs;
}

TEST_CASE("test_IndicatorImpBuffer_missing_coverage") {
    // Test the various cases of the resize method
    IndicatorImpBuffer buf;

    // resize enlarges the capacity
    buf.resize(5);
    CHECK_EQ(buf.size(), 5);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[4], 0.0);

    // resize shrinks the capacity
    buf.resize(3);
    CHECK_EQ(buf.size(), 3);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[2], 0.0);

    // resize with a default value enlarges it
    buf.resize(6, 2.5);
    CHECK_EQ(buf.size(), 6);
    CHECK_EQ(buf[3], 2.5);
    CHECK_EQ(buf[5], 2.5);

    // Test max_size
    CHECK_GT(buf.max_size(), 0);

    // Test the bounds check of at
    CHECK_THROWS_AS(buf.at(10), std::out_of_range);

    // Test the boundary case of an empty container (note: front/back on an empty container is
    // undefined behavior)
    IndicatorImpBuffer empty_buf;
    // empty_buf.front() and empty_buf.back() are not tested, because that is undefined behavior
    CHECK_UNARY(empty_buf.empty());
    CHECK_EQ(empty_buf.size(), 0);
}

TEST_CASE("test_IndicatorImpBuffer_move_semantics") {
    // Test push_back with the move semantics
    IndicatorImpBuffer buf;
    double value = 3.14;
    buf.push_back(std::move(value));
    CHECK_EQ(buf.size(), 1);
    CHECK_EQ(buf[0], doctest::Approx(3.14));

    // Test insert with the move semantics
    double insert_value = 2.71;
    buf.insert(buf.begin(), std::move(insert_value));
    CHECK_EQ(buf.size(), 2);
    CHECK_EQ(buf[0], doctest::Approx(2.71));
}

TEST_CASE("test_IndicatorImpBuffer_iterator_operations") {
    IndicatorImpBuffer buf = {1.0, 2.0, 3.0, 4.0, 5.0};

    // Test the reverse iterator
    auto rit = buf.rbegin();
    CHECK_EQ(*rit, 5.0);
    ++rit;
    CHECK_EQ(*rit, 4.0);

    // Test the const iterator
    const IndicatorImpBuffer& const_buf = buf;
    auto cit = const_buf.cbegin();
    CHECK_EQ(*cit, 1.0);

    // Test the iterator arithmetic
    auto it1 = buf.begin();
    auto it2 = buf.begin() + 2;
    CHECK_EQ(it2 - it1, 2);
    CHECK_EQ(*(it1 + 2), 3.0);

    // Test the iterator comparison
    CHECK_FALSE(it1 == it2);
    CHECK_UNARY(it1 < it2);
    CHECK_UNARY(it2 > it1);
}

TEST_CASE("test_IndicatorImpBuffer_range_insert") {
    IndicatorImpBuffer buf = {1.0, 2.0, 3.0};
    std::vector<double> source = {4.0, 5.0, 6.0};

    // Insert a range from a vector
    buf.insert(buf.begin() + 1, source.begin(), source.end());
    CHECK_EQ(buf.size(), 6);
    CHECK_EQ(buf[1], 4.0);
    CHECK_EQ(buf[2], 5.0);
    CHECK_EQ(buf[3], 6.0);

    // Insert a range at the beginning
    std::vector<double> prefix = {0.0, 0.5};
    buf.insert(buf.begin(), prefix.begin(), prefix.end());
    CHECK_EQ(buf.size(), 8);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[1], 0.5);

    // Insert a range at the end
    std::vector<double> suffix = {7.0, 8.0};
    buf.insert(buf.end(), suffix.begin(), suffix.end());
    CHECK_EQ(buf.size(), 10);
    CHECK_EQ(buf[8], 7.0);
    CHECK_EQ(buf[9], 8.0);
}

TEST_CASE("test_IndicatorImpBuffer_initializer_list") {
    // Test the initializer list constructor
    IndicatorImpBuffer buf1{1.0, 2.0, 3.0, 4.0};
    CHECK_EQ(buf1.size(), 4);
    CHECK_EQ(buf1[0], 1.0);
    CHECK_EQ(buf1[3], 4.0);

    // Test an empty initializer list
    IndicatorImpBuffer buf2{};
    CHECK_EQ(buf2.size(), 0);
    CHECK_UNARY(buf2.empty());
}

TEST_CASE("test_IndicatorImpBuffer_comparison_operators") {
    IndicatorImpBuffer buf1{1.0, 2.0, 3.0};
    IndicatorImpBuffer buf2{1.0, 2.0, 3.0};
    IndicatorImpBuffer buf3{1.0, 2.0, 4.0};
    IndicatorImpBuffer buf4{1.0, 2.0};

    // Test the equality
    CHECK_UNARY(buf1 == buf2);
    CHECK_FALSE(buf1 == buf3);
    CHECK_FALSE(buf1 == buf4);

    // Test the inequality
    CHECK_FALSE(buf1 != buf2);
    CHECK_UNARY(buf1 != buf3);
    CHECK_UNARY(buf1 != buf4);

    // Test the less-than comparison
    CHECK_FALSE(buf1 < buf2);  // Equal
    CHECK_UNARY(buf1 < buf3);  // Lexicographically smaller
    CHECK_FALSE(buf1 < buf4);  // A greater length

    // Test the other comparison operators
    CHECK_UNARY(buf1 <= buf2);  // Equal
    CHECK_FALSE(buf3 < buf1);   // Greater
    CHECK_UNARY(buf3 > buf1);   // Greater
    CHECK_UNARY(buf3 >= buf1);  // Greater than or equal
}

TEST_CASE("test_IndicatorImpBuffer_edge_cases") {
    // Test the operations with many elements
    IndicatorImpBuffer buf;
    for (int i = 0; i < 1000; ++i) {
        buf.push_back(static_cast<double>(i));
    }
    CHECK_EQ(buf.size(), 1000);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[999], 999.0);

    // Test the frequent insert and erase operations
    for (int i = 0; i < 100; ++i) {
        buf.insert(buf.begin() + i, static_cast<double>(i + 1000));
        buf.erase(buf.begin() + i + 1);
    }
    CHECK_EQ(buf.size(), 1000);

    // Test the memory reallocation scenario
    buf.clear();
    buf.reserve(10);
    for (int i = 0; i < 20; ++i) {
        buf.push_back(static_cast<double>(i));
    }
    CHECK_GE(buf.capacity(), 20);

    // Test the extreme capacity operations
    buf.clear();
    buf.resize(1000000, 1.0);  // One million elements
    CHECK_EQ(buf.size(), 1000000);
    CHECK_EQ(buf[500000], 1.0);
}

TEST_CASE("test_IndicatorImpBuffer_exception_safety") {
    // Test the basic exception safety scenario
    IndicatorImpBuffer buf1{1.0, 2.0, 3.0};
    IndicatorImpBuffer buf2{4.0, 5.0};

    // A normal swap operation
    buf1.swap(buf2);
    CHECK_EQ(buf1.size(), 2);
    CHECK_EQ(buf2.size(), 3);

    // Test the non-member swap function
    swap(buf1, buf2);
    CHECK_EQ(buf1.size(), 3);
    CHECK_EQ(buf2.size(), 2);
}
