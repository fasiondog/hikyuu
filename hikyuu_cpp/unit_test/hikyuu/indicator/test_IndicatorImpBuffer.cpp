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
    // 默认构造函数
    IndicatorImpBuffer buf1;
    CHECK_EQ(buf1.size(), 0);

    // 带初始大小的构造函数
    IndicatorImpBuffer buf2(5);
    CHECK_EQ(buf2.size(), 5);
    CHECK_EQ(buf2[0], 0.0);
    CHECK_EQ(buf2[4], 0.0);

    // 带初始大小和值的构造函数
    IndicatorImpBuffer buf3(3, 1.5);
    CHECK_EQ(buf3.size(), 3);
    CHECK_EQ(buf3[0], 1.5);
    CHECK_EQ(buf3[2], 1.5);

    // 拷贝构造函数
    IndicatorImpBuffer buf4(buf3);
    CHECK_EQ(buf4.size(), 3);
    CHECK_EQ(buf4[0], 1.5);
    CHECK_EQ(buf4[2], 1.5);

    // 移动构造函数
    IndicatorImpBuffer buf5(std::move(buf4));
    CHECK_EQ(buf5.size(), 3);
    CHECK_EQ(buf5[0], 1.5);
    CHECK_EQ(buf5[2], 1.5);
}

TEST_CASE("test_IndicatorImpBuffer_assignment") {
    IndicatorImpBuffer buf1(3, 1.0);
    IndicatorImpBuffer buf2(2, 2.0);

    // 拷贝赋值
    buf2 = buf1;
    CHECK_EQ(buf2.size(), 3);
    CHECK_EQ(buf2[0], 1.0);
    CHECK_EQ(buf2[2], 1.0);

    // 移动赋值
    IndicatorImpBuffer buf3(1, 3.0);
    buf3 = std::move(buf2);
    CHECK_EQ(buf3.size(), 3);
    CHECK_EQ(buf3[0], 1.0);
    CHECK_EQ(buf3[2], 1.0);
}

TEST_CASE("test_IndicatorImpBuffer_iterators") {
    IndicatorImpBuffer buf = {1.0, 2.0, 3.0};

    // 测试begin/end
    auto it = buf.begin();
    CHECK_EQ(*it, 1.0);
    ++it;
    CHECK_EQ(*it, 2.0);

    // 测试范围for循环
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

    // 测试operator[]
    CHECK_EQ(buf[0], 1.0);
    CHECK_EQ(buf[2], 3.0);

    // 测试at()
    CHECK_EQ(buf.at(1), 2.0);

    // 测试front/back
    CHECK_EQ(buf.front(), 1.0);
    CHECK_EQ(buf.back(), 3.0);

    // 测试data()
    CHECK_EQ(buf.data()[1], 2.0);
}

TEST_CASE("test_IndicatorImpBuffer_modifiers") {
    IndicatorImpBuffer buf;

    // 测试push_back
    buf.push_back(1.0);
    buf.push_back(2.0);
    buf.push_back(3.0);
    CHECK_EQ(buf.size(), 3);
    CHECK_EQ(buf[0], 1.0);
    CHECK_EQ(buf[2], 3.0);

    // 测试emplace_back
    buf.emplace_back(4.0);
    CHECK_EQ(buf.size(), 4);
    CHECK_EQ(buf.back(), 4.0);

    // 测试insert单个元素
    auto it = buf.insert(buf.begin() + 1, 1.5);
    CHECK_EQ(buf.size(), 5);
    CHECK_EQ(buf[1], 1.5);
    CHECK_EQ(*it, 1.5);

    // 测试insert多个元素
    it = buf.insert(buf.begin() + 2, 2, 2.5);
    CHECK_EQ(buf.size(), 7);
    CHECK_EQ(buf[2], 2.5);
    CHECK_EQ(buf[3], 2.5);

    // 测试erase单个元素
    it = buf.erase(buf.begin() + 3);
    CHECK_EQ(buf.size(), 6);
    CHECK_EQ(buf[3], 2.0);  // 修正：删除的是原来的第4个元素(2.5)，所以现在第4个元素是2.0
    CHECK_EQ(*it, 2.0);     // 修正：返回的迭代器指向下一个元素

    // 测试erase范围
    it = buf.erase(buf.begin() + 1, buf.begin() + 3);
    CHECK_EQ(buf.size(), 4);
    CHECK_EQ(buf[1], 2.0);  // 修正：删除[1.5, 2.5]后，第2个元素是2.0
    CHECK_EQ(*it, 2.0);     // 修正：返回的迭代器指向被删除范围后的第一个元素

    // 测试pop_back
    buf.pop_back();
    CHECK_EQ(buf.size(), 3);
    CHECK_EQ(buf.back(), 3.0);

    // 测试clear
    buf.clear();
    CHECK_UNARY(buf.empty());
    CHECK_EQ(buf.size(), 0);
}

TEST_CASE("test_IndicatorImpBuffer_algorithms") {
    IndicatorImpBuffer buf = {3.0, 1.0, 4.0, 1.0, 5.0};

    // 测试sort
    buf.sort();
    CHECK_EQ(buf[0], 1.0);
    CHECK_EQ(buf[1], 1.0);
    CHECK_EQ(buf[4], 5.0);

    // 测试erase_if
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
    // 测试new操作符
    IndicatorImpBuffer* buf = new IndicatorImpBuffer(5, 2.0);
    CHECK_EQ(buf->size(), 5);
    CHECK_EQ((*buf)[0], 2.0);
    delete buf;

    // 测试new[]操作符
    IndicatorImpBuffer* bufs = new IndicatorImpBuffer[2];
    bufs[0] = IndicatorImpBuffer(3, 1.0);
    bufs[1] = IndicatorImpBuffer(2, 2.0);
    CHECK_EQ(bufs[0].size(), 3);
    CHECK_EQ(bufs[1].size(), 2);
    delete[] bufs;
}

TEST_CASE("test_IndicatorImpBuffer_missing_coverage") {
    // 测试resize方法的各种情况
    IndicatorImpBuffer buf;

    // resize扩大容量
    buf.resize(5);
    CHECK_EQ(buf.size(), 5);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[4], 0.0);

    // resize缩小容量
    buf.resize(3);
    CHECK_EQ(buf.size(), 3);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[2], 0.0);

    // resize带默认值扩大
    buf.resize(6, 2.5);
    CHECK_EQ(buf.size(), 6);
    CHECK_EQ(buf[3], 2.5);
    CHECK_EQ(buf[5], 2.5);

    // 测试max_size
    CHECK_GT(buf.max_size(), 0);

    // 测试at的边界检查
    CHECK_THROWS_AS(buf.at(10), std::out_of_range);

    // 测试空容器的边界情况（注意：front/back在空容器上是未定义行为，不抛异常）
    IndicatorImpBuffer empty_buf;
    // 不测试empty_buf.front()和empty_buf.back()，因为这是未定义行为
    CHECK_UNARY(empty_buf.empty());
    CHECK_EQ(empty_buf.size(), 0);
}

TEST_CASE("test_IndicatorImpBuffer_move_semantics") {
    // 测试移动语义的push_back
    IndicatorImpBuffer buf;
    double value = 3.14;
    buf.push_back(std::move(value));
    CHECK_EQ(buf.size(), 1);
    CHECK_EQ(buf[0], doctest::Approx(3.14));

    // 测试移动语义的insert
    double insert_value = 2.71;
    buf.insert(buf.begin(), std::move(insert_value));
    CHECK_EQ(buf.size(), 2);
    CHECK_EQ(buf[0], doctest::Approx(2.71));
}

TEST_CASE("test_IndicatorImpBuffer_iterator_operations") {
    IndicatorImpBuffer buf = {1.0, 2.0, 3.0, 4.0, 5.0};

    // 测试反向迭代器
    auto rit = buf.rbegin();
    CHECK_EQ(*rit, 5.0);
    ++rit;
    CHECK_EQ(*rit, 4.0);

    // 测试const迭代器
    const IndicatorImpBuffer& const_buf = buf;
    auto cit = const_buf.cbegin();
    CHECK_EQ(*cit, 1.0);

    // 测试迭代器算术运算
    auto it1 = buf.begin();
    auto it2 = buf.begin() + 2;
    CHECK_EQ(it2 - it1, 2);
    CHECK_EQ(*(it1 + 2), 3.0);

    // 测试迭代器比较
    CHECK_FALSE(it1 == it2);
    CHECK_UNARY(it1 < it2);
    CHECK_UNARY(it2 > it1);
}

TEST_CASE("test_IndicatorImpBuffer_range_insert") {
    IndicatorImpBuffer buf = {1.0, 2.0, 3.0};
    std::vector<double> source = {4.0, 5.0, 6.0};

    // 从vector插入范围
    buf.insert(buf.begin() + 1, source.begin(), source.end());
    CHECK_EQ(buf.size(), 6);
    CHECK_EQ(buf[1], 4.0);
    CHECK_EQ(buf[2], 5.0);
    CHECK_EQ(buf[3], 6.0);

    // 在开头插入范围
    std::vector<double> prefix = {0.0, 0.5};
    buf.insert(buf.begin(), prefix.begin(), prefix.end());
    CHECK_EQ(buf.size(), 8);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[1], 0.5);

    // 在末尾插入范围
    std::vector<double> suffix = {7.0, 8.0};
    buf.insert(buf.end(), suffix.begin(), suffix.end());
    CHECK_EQ(buf.size(), 10);
    CHECK_EQ(buf[8], 7.0);
    CHECK_EQ(buf[9], 8.0);
}

TEST_CASE("test_IndicatorImpBuffer_initializer_list") {
    // 测试初始化列表构造函数
    IndicatorImpBuffer buf1{1.0, 2.0, 3.0, 4.0};
    CHECK_EQ(buf1.size(), 4);
    CHECK_EQ(buf1[0], 1.0);
    CHECK_EQ(buf1[3], 4.0);

    // 测试空初始化列表
    IndicatorImpBuffer buf2{};
    CHECK_EQ(buf2.size(), 0);
    CHECK_UNARY(buf2.empty());
}

TEST_CASE("test_IndicatorImpBuffer_comparison_operators") {
    IndicatorImpBuffer buf1{1.0, 2.0, 3.0};
    IndicatorImpBuffer buf2{1.0, 2.0, 3.0};
    IndicatorImpBuffer buf3{1.0, 2.0, 4.0};
    IndicatorImpBuffer buf4{1.0, 2.0};

    // 测试相等性
    CHECK_UNARY(buf1 == buf2);
    CHECK_FALSE(buf1 == buf3);
    CHECK_FALSE(buf1 == buf4);

    // 测试不等性
    CHECK_FALSE(buf1 != buf2);
    CHECK_UNARY(buf1 != buf3);
    CHECK_UNARY(buf1 != buf4);

    // 测试小于比较
    CHECK_FALSE(buf1 < buf2);  // 相等
    CHECK_UNARY(buf1 < buf3);  // 字典序小于
    CHECK_FALSE(buf1 < buf4);  // 长度大于

    // 测试其他比较操作符
    CHECK_UNARY(buf1 <= buf2);  // 相等
    CHECK_FALSE(buf3 < buf1);   // 大于
    CHECK_UNARY(buf3 > buf1);   // 大于
    CHECK_UNARY(buf3 >= buf1);  // 大于等于
}

TEST_CASE("test_IndicatorImpBuffer_edge_cases") {
    // 测试大量元素操作
    IndicatorImpBuffer buf;
    for (int i = 0; i < 1000; ++i) {
        buf.push_back(static_cast<double>(i));
    }
    CHECK_EQ(buf.size(), 1000);
    CHECK_EQ(buf[0], 0.0);
    CHECK_EQ(buf[999], 999.0);

    // 测试频繁的插入删除操作
    for (int i = 0; i < 100; ++i) {
        buf.insert(buf.begin() + i, static_cast<double>(i + 1000));
        buf.erase(buf.begin() + i + 1);
    }
    CHECK_EQ(buf.size(), 1000);

    // 测试内存重新分配场景
    buf.clear();
    buf.reserve(10);
    for (int i = 0; i < 20; ++i) {
        buf.push_back(static_cast<double>(i));
    }
    CHECK_GE(buf.capacity(), 20);

    // 测试极端容量操作
    buf.clear();
    buf.resize(1000000, 1.0);  // 一百万个元素
    CHECK_EQ(buf.size(), 1000000);
    CHECK_EQ(buf[500000], 1.0);
}

TEST_CASE("test_IndicatorImpBuffer_exception_safety") {
    // 测试异常安全性的基本场景
    IndicatorImpBuffer buf1{1.0, 2.0, 3.0};
    IndicatorImpBuffer buf2{4.0, 5.0};

    // 正常swap操作
    buf1.swap(buf2);
    CHECK_EQ(buf1.size(), 2);
    CHECK_EQ(buf2.size(), 3);

    // 测试非成员swap函数
    swap(buf1, buf2);
    CHECK_EQ(buf1.size(), 3);
    CHECK_EQ(buf2.size(), 2);
}
