/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-01-13
 *      Author: fasiondog
 */

#pragma once

#include <memory>
#include <functional>
#include <forward_list>
#include <unordered_map>
#include "thread/ThreadPool.h"
#include "any_to_string.h"
#include "Log.h"

namespace hku {

/**
 * @brief 过滤节点
 */
class FilterNode {
public:
    FilterNode() = default;
    FilterNode(const FilterNode&) = default;
    virtual ~FilterNode() = default;

    /**
     * @brief 构造函数
     * @param exclusive 是否排他。为 true 时，只执行第一个遇到的满足过滤条件的子节点
     */
    explicit FilterNode(bool exclusive) : m_exclusive(exclusive) {}

    FilterNode(FilterNode&& rv)
    : m_value(std::move(rv.m_value)),
      m_children(std::move(rv.m_children)),
      m_exclusive(rv.m_exclusive) {}

    FilterNode& operator=(const FilterNode& rv) {
        if (this == &rv)
            return *this;
        m_value = rv.m_value;
        m_children = rv.m_children;
        m_exclusive = rv.m_exclusive;
        return *this;
    }

    FilterNode& operator=(FilterNode&& rv) {
        if (this == &rv)
            return *this;
        m_value = std::move(rv.m_value);
        m_children = std::move(rv.m_children);
        m_exclusive = rv.m_exclusive;
        return *this;
    }

    using ptr_t = std::shared_ptr<FilterNode>;

    ptr_t addChild(const ptr_t& child) {
        HKU_CHECK(child, "Invalid input child! child is null!");
        m_children.push_front(child);
        return child;
    }

    bool exclusive() const {
        return m_exclusive;
    }

    void exclusive(bool exclusive) {
        m_exclusive = exclusive;
    }

    using const_iterator = std::forward_list<ptr_t>::const_iterator;
    using iterator = std::forward_list<ptr_t>::iterator;
    const_iterator cbegin() const {
        return m_children.cbegin();
    }

    const_iterator cend() const {
        return m_children.cend();
    }

    iterator begin() {
        return m_children.begin();
    }

    iterator end() {
        return m_children.end();
    }

    bool run(const any_t& data) noexcept {
        if (_filter(data)) {
            _process(data);
            for (auto& node : m_children) {
                if (node->run(data) && m_exclusive) {
                    return true;
                }
            }
            return true;
        }
        return false;
    }

    virtual bool filter(const any_t& data) {
        return true;
    }

    virtual void process(const any_t& data) {}

    template <typename ValueT>
    ValueT value() const {
        return any_cast<ValueT>(m_value);
    }

    template <typename ValueT>
    void value(const ValueT& value) {
        m_value = value;
    }

    bool has_value() const {
#if !HKU_OS_IOS && CPP_STANDARD >= CPP_STANDARD_17
        return m_value.has_value();
#else
        return !m_value.empty();
#endif
    }

private:
    bool _filter(const any_t& data) noexcept {
        try {
            return filter(data);
        } catch (const std::exception& e) {
            HKU_WARN("Node filter exist error! {}", e.what());
        } catch (...) {
            HKU_WARN("Node filter exist unknown error!");
        }
        return false;
    }

    void _process(const any_t& data) noexcept {
        try {
            process(data);
        } catch (const std::exception& e) {
            HKU_WARN("Node process exist error! {}", e.what());
        } catch (...) {
            HKU_WARN("Node process exist unknown error!");
        }
    }

protected:
    any_t m_value;

private:
    std::forward_list<ptr_t> m_children;
    bool m_exclusive = false;
};

template <>
inline const any_t& FilterNode::value() const {
    return m_value;
}

typedef std::shared_ptr<FilterNode> FilterNodePtr;

/**
 * @brief 绑定过滤节点，通过 std::function 绑定自定义的 filter 和 process 处理函数
 */
class BindFilterNode : public FilterNode {
public:
    BindFilterNode() = default;
    virtual ~BindFilterNode() = default;

    using filter_func = std::function<bool(FilterNode*, const any_t&)>;
    using process_func = std::function<void(FilterNode*, const any_t&)>;

    explicit BindFilterNode(const process_func& process) : FilterNode(false), m_process(process) {}
    explicit BindFilterNode(process_func&& process)
    : FilterNode(false), m_process(std::move(process)) {}

    BindFilterNode(const filter_func& filter, const process_func& process, bool exclusive = false)
    : FilterNode(exclusive), m_filter(filter), m_process(process) {}

    BindFilterNode(filter_func&& filter, process_func&& process, bool exclusive = false)
    : FilterNode(exclusive), m_filter(std::move(filter)), m_process(std::move(process)) {}

    virtual bool filter(const any_t& data) {
        return m_filter ? m_filter(this, data) : true;
    }

    virtual void process(const any_t& data) {
        if (m_process) {
            m_process(this, data);
        }
    }

private:
    filter_func m_filter;
    process_func m_process;
};

/**
 * @brief 异步串行事件处理器
 * @tparam EventT
 */
template <class EventT>
class AsyncSerialEventProcessor {
public:
    /**
     * @brief 构造函数
     * @param quit_wait 退出时等待所有任务完成
     */
    explicit AsyncSerialEventProcessor(bool quit_wait = true) : m_quit_wait(quit_wait) {
        m_tg = std::unique_ptr<ThreadPool>(new ThreadPool(1));
    }

    /** 析构函数 */
    virtual ~AsyncSerialEventProcessor() {
        if (m_quit_wait) {
            m_tg->join();
        } else {
            m_tg->stop();
        }
    }

    /**
     * @brief 添加事件处理节点
     *
     * @param event 事件
     * @param action 对应的处理节点
     * @return 返回加入的节点
     */
    FilterNodePtr addAction(const EventT& event, const FilterNodePtr& action) {
        HKU_CHECK(action, "Input action is null!");
        std::lock_guard<std::mutex> lock(m_mutex);
        auto iter = m_trees.find(event);
        if (iter != m_trees.end()) {
            iter->second->addChild(action);
        } else {
            m_trees[event] = action;
        }
        return action;
    }

    /**
     * @brief 分派事件消息
     *
     * @param event 事件
     * @param data 事件附加信息
     */
    void dispatch(const EventT& event, const any_t& data) {
        m_tg->submit([=] {
            auto iter = m_trees.find(event);
            HKU_WARN_IF_RETURN(iter == m_trees.end(), void(),
                              "There is no matching handling method for the event({})!", event);
            iter->second->run(data);
        });
    }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<EventT, FilterNodePtr> m_trees;
    std::unique_ptr<ThreadPool> m_tg;
    bool m_quit_wait = true;
};

}  // namespace hku