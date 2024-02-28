/*
 *  Copyright (c) 2023 hikyuu.org
 *HKU_
 *  Created on: 2023-01-13
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/Log.h>
#include <hikyuu/utilities/FilterNode.h>

using namespace hku;

class TestFilterNode : public FilterNode {
public:
    TestFilterNode() = default;
    virtual ~TestFilterNode() = default;

    TestFilterNode(int flag) : FilterNode(), m_flag(flag) {}

    virtual bool filter(const any_t& data) override {
        return m_flag % 2;
    }

    virtual void process(const any_t& data) override {
        HKU_TRACE("TestFilterNode process {}", m_flag);
        int val = any_cast<int>(data);
        m_value = m_flag + val;
    }

private:
    int m_flag = 0;
};

class TestFilterNodeExceptFilter : public TestFilterNode {
    virtual bool filter(const any_t& data) override {
        throw std::runtime_error("error");
    }
};

class TestFilterNodeExceptProcess : public TestFilterNode {
    virtual void process(const any_t& data) override {
        throw std::runtime_error("error");
    }
};

TEST_CASE("test_FilterNode_no_group") {
    auto root = std::make_shared<FilterNode>(false);
    CHECK_UNARY(!root->exclusive());
    CHECK_THROWS(root->addChild(FilterNodePtr()));

    auto node1 = std::make_shared<TestFilterNode>(1);
    auto node11 = std::make_shared<TestFilterNode>(11);
    auto node_except_filter = std::make_shared<TestFilterNodeExceptFilter>();
    auto node_except_process = std::make_shared<TestFilterNodeExceptProcess>();
    node1->addChild(node11);
    node1->addChild(node_except_filter);
    node1->addChild(node_except_process);
    root->addChild(node1);

    auto node2 = std::make_shared<TestFilterNode>(2);
    auto node23 = std::make_shared<TestFilterNode>(23);
    auto node22 = std::make_shared<TestFilterNode>(22);
    node2->addChild(node23);
    node2->addChild(node22);
    root->addChild(node2);

    auto node3 = std::make_shared<TestFilterNode>(3);
    auto node33 = std::make_shared<TestFilterNode>(33);
    auto node34 = std::make_shared<TestFilterNode>(34);
    auto node35 = std::make_shared<TestFilterNode>(35);
    node3->addChild(node33);
    node3->addChild(node34);
    node3->addChild(node35);
    root->addChild(node3);

    auto bind_node1 = std::make_shared<BindFilterNode>(
      [](FilterNode* self, const any_t& data) -> bool { return true; },
      [](FilterNode* self, const any_t& data) { HKU_DEBUG("bind filter node"); });
    auto bind_node2 = std::make_shared<BindFilterNode>();
    root->addChild(bind_node1);
    root->addChild(bind_node2);

    root->run(1);
    CHECK_UNARY(!node2->has_value());
    CHECK_UNARY(!node22->has_value());
    CHECK_UNARY(!node34->has_value());
    CHECK_EQ(node1->value<int>(), 2);
    CHECK_EQ(node11->value<int>(), 12);
    CHECK_EQ(node3->value<int>(), 4);
    CHECK_EQ(node33->value<int>(), 34);
    CHECK_EQ(node35->value<int>(), 36);

    root->exclusive(true);
    CHECK_UNARY(root->exclusive());

    for (const auto& child : *root) {
        HKU_INFO("child value: {}", child->has_value() ? child->value<int>() : 0);
    }

    node1->value<int>(100);
    CHECK_EQ(node1->value<int>(), 100);
}

TEST_CASE("test_FilterNode_group") {
    auto root = std::make_shared<FilterNode>(true);

    auto node1 = std::make_shared<TestFilterNode>(1);
    auto node11 = std::make_shared<TestFilterNode>(11);
    node1->addChild(node11);
    root->addChild(node1);

    auto node_except_filter = std::make_shared<TestFilterNodeExceptFilter>();
    auto node_except_process = std::make_shared<TestFilterNodeExceptProcess>();
    root->addChild(node_except_filter);
    root->addChild(node_except_process);

    auto node2 = std::make_shared<TestFilterNode>(2);
    auto node23 = std::make_shared<TestFilterNode>(23);
    auto node22 = std::make_shared<TestFilterNode>(22);
    node2->addChild(node23);
    node2->addChild(node22);
    root->addChild(node2);

    auto node3 = std::make_shared<TestFilterNode>(3);
    auto node33 = std::make_shared<TestFilterNode>(33);
    auto node34 = std::make_shared<TestFilterNode>(34);
    auto node35 = std::make_shared<TestFilterNode>(35);
    node3->addChild(node33);
    node3->addChild(node34);
    node3->addChild(node35);
    root->addChild(node3);

    root->run(1);
    CHECK_UNARY(!node2->has_value());
    CHECK_UNARY(!node22->has_value());
    CHECK_UNARY(!node34->has_value());
    CHECK_UNARY(!node1->has_value());
    CHECK_UNARY(!node11->has_value());
    CHECK_EQ(node3->value<int>(), 4);
    CHECK_EQ(node33->value<int>(), 34);
    CHECK_EQ(node35->value<int>(), 36);

    for (const auto& child : *root) {
        HKU_INFO("child value: {}", child->has_value() ? child->value<int>() : 0);
    }
}

TEST_CASE("test_AsyncSerialEventProcessor") {
    AsyncSerialEventProcessor<int> trees(true);
    CHECK_THROWS(trees.addAction(-1, FilterNodePtr()));

    for (int i = 0; i < 20; i++) {
        trees.addAction(i, std::make_shared<TestFilterNode>(i));
    }

    for (int i = 0; i < 20; i++) {
        trees.dispatch(i, any_t());
    }
}

TEST_CASE("test_any_to_string") {
    CHECK_EQ(any_to_string<int32_t>(any_t(int(1))), "1");
    CHECK_EQ(any_to_string<int32_t>(any_t(int(-1))), "-1");
    CHECK_EQ(any_to_string<int64_t>(any_t(int64_t(1))), "1");
    CHECK_EQ(any_to_string<int64_t>(any_t(int64_t(-1))), "-1");
    CHECK_EQ(any_to_string<uint32_t>(any_t(uint32_t(1))), "1");
    CHECK_EQ(any_to_string<uint64_t>(any_t(uint64_t(1))), "1");
    CHECK_EQ(any_to_string<size_t>(any_t(size_t(1))), "1");
    CHECK_EQ(any_to_string<float>(any_t(float(1.0))), std::to_string(float(1.0)));
    CHECK_EQ(any_to_string<double>(any_t(double(1.0))), std::to_string(double(1.0)));

    CHECK_EQ(any_to_string<Datetime>(any_t(Datetime(202301151217))), "2023-01-15 12:17:00");
    CHECK_EQ(any_to_string<Datetime>(any_t(Datetime())), Datetime().str());
    CHECK_EQ(any_to_string<TimeDelta>(any_t(TimeDelta())), "0 days, 00:00:0.000000");
    CHECK_EQ(any_to_string<TimeDelta>(any_t(TimeDelta(-1))), "-1 days, 00:00:0.000000");
    CHECK_EQ(any_to_string<TimeDelta>(any_t(TimeDelta(1, 2, 3, 4, 5, 6))),
             "1 days, 02:03:4.005006");
}

TEST_CASE("test_string_to_any") {
    CHECK_EQ(int32_t(1), any_cast<int32_t>(string_to_any<int>("1")));
    CHECK_EQ(int32_t(-1), any_cast<int32_t>(string_to_any<int>("-1")));
    CHECK_EQ(int64_t(1), any_cast<int64_t>(string_to_any<int64_t>("1")));
    CHECK_EQ(int64_t(-1), any_cast<int64_t>(string_to_any<int64_t>("-1")));
    CHECK_EQ(uint32_t(1), any_cast<uint32_t>(string_to_any<uint32_t>("1")));
    CHECK_EQ(uint64_t(1), any_cast<uint64_t>(string_to_any<uint64_t>("1")));
    CHECK_EQ(size_t(1), any_cast<size_t>(string_to_any<size_t>("1")));
    CHECK_EQ(float(1.0), any_cast<float>(string_to_any<float>("1.0")));
    CHECK_EQ(double(1.0), any_cast<double>(string_to_any<double>("1.0")));
    CHECK_EQ(double(1.23457e+08), any_cast<double>(string_to_any<double>("1.23457e+08")));

    CHECK_EQ(Datetime(202301151217),
             any_cast<Datetime>(string_to_any<Datetime>("2023-01-15 12:17:00")));
    CHECK_EQ(TimeDelta(), any_cast<TimeDelta>(string_to_any<TimeDelta>("0 days, 0:0:00.0")));
    // CHECK_EQ(TimeDelta(1, 2, 3, 4, 5, 6),
    //          any_cast<TimeDelta>(string_to_any<TimeDelta>("1 days, 2:3:4.005006")));
}