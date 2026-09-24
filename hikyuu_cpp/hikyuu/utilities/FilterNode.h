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
#include <shared_mutex>
#include "thread/ThreadPool.h"
#include "any_to_string.h"
#include "Log.h"

namespace hku {

/**
 * @brief Filter node
 */
class FilterNode {
public:
    FilterNode() = default;
    FilterNode(const FilterNode&) = default;
    virtual ~FilterNode() = default;

    /**
     * @brief Constructor
     * @param exclusive whether it is exclusive. When it is true only the first encountered child
     * node satisfying the filter condition is executed
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

    /**
     * @brief Set the exclusive mode
     * @param exclusive true: only the first matching child node is executed; false: all the
     * matching child nodes are executed
     */
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
            HKU_WARN("Exception in node filter: {}", e.what());
        } catch (...) {
            HKU_WARN("Unknown exception in node filter!");
        }
        return false;
    }

    void _process(const any_t& data) noexcept {
        try {
            process(data);
        } catch (const std::exception& e) {
            HKU_WARN("Exception in node process: {}", e.what());
        } catch (...) {
            HKU_WARN("Unknown exception in node process!");
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
 * @brief Bind a filter node; the custom filter and process handler functions are bound through
 * std::function
 * @note When it is created with the default constructor, m_filter and m_process are empty, filter()
 *       returns true and process()
 *       does not perform any operation
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
 * @brief Asynchronous serial event processor
 * @tparam EventT the event type, it needs to support hashing and equality comparison
 * @note All the event processing is executed serially in a single thread pool, guaranteeing that
 * only one event is being processed at the same moment
 */
template <class EventT>
class AsyncSerialEventProcessor {
public:
    /**
     * @brief Constructor
     * @param quit_wait wait for all the tasks to be finished on exit
     */
    explicit AsyncSerialEventProcessor(bool quit_wait = true) : m_quit_wait(quit_wait) {
        m_tg = std::unique_ptr<ThreadPool>(new ThreadPool(1));
    }

    /** Destructor */
    virtual ~AsyncSerialEventProcessor() {
        if (m_quit_wait) {
            m_tg->join();
        } else {
            m_tg->stop();
        }
    }

    /**
     * @brief Add an event processing node
     *
     * @param event event
     * @param action the corresponding processing node
     * @return the added node
     */
    FilterNodePtr addAction(const EventT& event, const FilterNodePtr& action) {
        HKU_CHECK(action, "Input action is null!");
        std::lock_guard<std::shared_mutex> lock(m_mutex);
        auto iter = m_trees.find(event);
        if (iter != m_trees.end()) {
            iter->second->addChild(action);
        } else {
            m_trees[event] = action;
        }
        return action;
    }

    /**
     * @brief Dispatch an event message
     *
     * @param event event
     * @param data the additional event information
     */
    void dispatch(const EventT& event, const any_t& data) {
        m_tg->submit([this, event, data] {
            try {
                std::shared_lock<std::shared_mutex> lock(m_mutex);
                auto iter = m_trees.find(event);
                HKU_WARN_IF_RETURN(iter == m_trees.end(), void(),
                                   "There is no matching handling method for the event({})!",
                                   event);
                iter->second->run(data);
            } catch (const std::exception& e) {
                HKU_WARN("Exception in event dispatch: {}", e.what());
            } catch (...) {
                HKU_WARN("Unknown exception in event dispatch!");
            }
        });
    }

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<EventT, FilterNodePtr> m_trees;
    std::unique_ptr<ThreadPool> m_tg;
    bool m_quit_wait = true;
};

}  // namespace hku