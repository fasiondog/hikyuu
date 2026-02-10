/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-10
 *      Author: fasiondog
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "hikyuu/config.h"

namespace hku {

/**
 * IndicatorImpBuffer类，完全自定义内存管理，兼容malloc和mi_malloc
 * 对外提供类似std::vector的接口
 */
class HKU_API IndicatorImpBuffer {
public:
// 类型定义
#if HKU_USE_LOW_PRECISION
    typedef float value_type;
#else
    typedef double value_type;
#endif
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

private:
    // 内部数据结构
    struct Buffer {
        pointer data;        // 数据指针
        size_type size;      // 当前元素数量
        size_type capacity;  // 分配的容量

        Buffer() : data(nullptr), size(0), capacity(0) {}
        ~Buffer() {
            deallocate();
        }

        void allocate(size_type new_capacity);
        void deallocate();
        void reallocate(size_type new_capacity);
    };

    Buffer m_buffer;

public:
    /** 默认构造函数 */
    IndicatorImpBuffer() = default;

    /** 指定容量构造函数 */
    explicit IndicatorImpBuffer(size_type count) {
        if (count > 0) {
            m_buffer.allocate(count);
            m_buffer.size = count;
            std::uninitialized_value_construct_n(m_buffer.data, count);
        }
    }

    /** 带初始值的构造函数 */
    IndicatorImpBuffer(size_type count, const value_type& value) {
        if (count > 0) {
            m_buffer.allocate(count);
            m_buffer.size = count;
            std::uninitialized_fill_n(m_buffer.data, count, value);
        }
    }

    /** 拷贝构造函数 */
    IndicatorImpBuffer(const IndicatorImpBuffer& other) {
        if (other.m_buffer.size > 0) {
            m_buffer.allocate(other.m_buffer.capacity);
            m_buffer.size = other.m_buffer.size;
            std::uninitialized_copy(other.m_buffer.data, other.m_buffer.data + other.m_buffer.size,
                                    m_buffer.data);
        }
    }

    /** 移动构造函数 */
    IndicatorImpBuffer(IndicatorImpBuffer&& other) noexcept {
        // 直接转移资源
        m_buffer.data = other.m_buffer.data;
        m_buffer.size = other.m_buffer.size;
        m_buffer.capacity = other.m_buffer.capacity;

        // 清空源对象
        other.m_buffer.data = nullptr;
        other.m_buffer.size = 0;
        other.m_buffer.capacity = 0;
    }

    /** 从迭代器范围构造 */
    template <typename InputIterator>
    IndicatorImpBuffer(InputIterator first, InputIterator last) {
        // 计算距离
        size_type count = static_cast<size_type>(std::distance(first, last));
        if (count > 0) {
            reserve(count);
            std::uninitialized_copy(first, last, m_buffer.data);
            m_buffer.size = count;
        }
    }

    /** 初始化列表构造函数 */
    IndicatorImpBuffer(std::initializer_list<value_type> init_list) {
        if (init_list.size() > 0) {
            reserve(init_list.size());
            std::uninitialized_copy(init_list.begin(), init_list.end(), m_buffer.data);
            m_buffer.size = init_list.size();
        }
    }

    /** 赋值操作符 */
    IndicatorImpBuffer& operator=(const IndicatorImpBuffer& other) {
        if (this != &other) {
            if (other.m_buffer.size > m_buffer.capacity) {
                // 需要重新分配内存
                Buffer new_buffer;
                new_buffer.allocate(other.m_buffer.capacity);
                std::uninitialized_copy(other.m_buffer.data,
                                        other.m_buffer.data + other.m_buffer.size, new_buffer.data);
                new_buffer.size = other.m_buffer.size;

                // 清理旧资源并替换
                m_buffer.deallocate();
                m_buffer.data = new_buffer.data;
                m_buffer.size = new_buffer.size;
                m_buffer.capacity = new_buffer.capacity;

                // 清空new_buffer以避免双重释放
                new_buffer.data = nullptr;
                new_buffer.size = 0;
                new_buffer.capacity = 0;
            } else {
                // 销毁现有元素
                destroy_elements(m_buffer.data, m_buffer.data + m_buffer.size);

                // 复制新元素
                std::copy(other.m_buffer.data, other.m_buffer.data + other.m_buffer.size,
                          m_buffer.data);
                m_buffer.size = other.m_buffer.size;
            }
        }
        return *this;
    }

    /** 移动赋值操作符 */
    IndicatorImpBuffer& operator=(IndicatorImpBuffer&& other) noexcept {
        if (this != &other) {
            // 清理当前对象的资源
            clear();
            m_buffer.deallocate();

            // 转移资源
            m_buffer.data = other.m_buffer.data;
            m_buffer.size = other.m_buffer.size;
            m_buffer.capacity = other.m_buffer.capacity;

            // 清空源对象
            other.m_buffer.data = nullptr;
            other.m_buffer.size = 0;
            other.m_buffer.capacity = 0;
        }
        return *this;
    }

    /** 析构函数 */
    ~IndicatorImpBuffer() {
        clear();
    }

    /** 重载new操作符 */
    static void* operator new(size_t size);

    /** 重载delete操作符 */
    static void operator delete(void* ptr) noexcept;

    /** 重载new[]操作符 */
    static void* operator new[](size_t size);

    /** 重载delete[]操作符 */
    static void operator delete[](void* ptr) noexcept;

    // 容量相关接口
    size_type size() const noexcept {
        return m_buffer.size;
    }
    size_type capacity() const noexcept {
        return m_buffer.capacity;
    }
    bool empty() const noexcept {
        return m_buffer.size == 0;
    }
    size_type max_size() const noexcept {
        return static_cast<size_type>(-1) / sizeof(value_type);
    }

    // 修改器接口
    void resize(size_type count) {
        if (count < m_buffer.size) {
            // 缩小
            destroy_elements(m_buffer.data + count, m_buffer.data + m_buffer.size);
            m_buffer.size = count;
        } else if (count > m_buffer.size) {
            // 扩大
            if (count > m_buffer.capacity) {
                reserve(count);
            }
            std::uninitialized_value_construct(m_buffer.data + m_buffer.size,
                                               m_buffer.data + count);
            m_buffer.size = count;
        }
    }

    void resize(size_type count, const value_type& value) {
        if (count < m_buffer.size) {
            // 缩小
            destroy_elements(m_buffer.data + count, m_buffer.data + m_buffer.size);
            m_buffer.size = count;
        } else if (count > m_buffer.size) {
            // 扩大
            if (count > m_buffer.capacity) {
                reserve(count);
            }
            std::uninitialized_fill(m_buffer.data + m_buffer.size, m_buffer.data + count, value);
            m_buffer.size = count;
        }
    }

    void reserve(size_type new_cap) {
        if (new_cap > m_buffer.capacity) {
            m_buffer.reallocate(calculate_growth(new_cap));
        }
    }

    void shrink_to_fit() {
        if (m_buffer.capacity > m_buffer.size) {
            m_buffer.reallocate(m_buffer.size);
        }
    }

    void clear() noexcept {
        destroy_elements(m_buffer.data, m_buffer.data + m_buffer.size);
        m_buffer.size = 0;
    }

    // 元素访问接口
    reference at(size_type pos) {
        if (pos >= m_buffer.size) {
            throw std::out_of_range("IndicatorImpBuffer::at: position out of range");
        }
        return m_buffer.data[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= m_buffer.size) {
            throw std::out_of_range("IndicatorImpBuffer::at: position out of range");
        }
        return m_buffer.data[pos];
    }

    reference operator[](size_type pos) {
        return m_buffer.data[pos];
    }
    const_reference operator[](size_type pos) const {
        return m_buffer.data[pos];
    }
    reference front() {
        return m_buffer.data[0];
    }
    const_reference front() const {
        return m_buffer.data[0];
    }
    reference back() {
        return m_buffer.data[m_buffer.size - 1];
    }
    const_reference back() const {
        return m_buffer.data[m_buffer.size - 1];
    }
    pointer data() noexcept {
        return m_buffer.data;
    }
    const_pointer data() const noexcept {
        return m_buffer.data;
    }

    // 迭代器接口
    iterator begin() noexcept {
        return m_buffer.data;
    }
    const_iterator begin() const noexcept {
        return m_buffer.data;
    }
    const_iterator cbegin() const noexcept {
        return m_buffer.data;
    }
    iterator end() noexcept {
        return m_buffer.data + m_buffer.size;
    }
    const_iterator end() const noexcept {
        return m_buffer.data + m_buffer.size;
    }
    const_iterator cend() const noexcept {
        return m_buffer.data + m_buffer.size;
    }
    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crend() const noexcept {
        return rend();
    }

    // 修改器接口
    void push_back(const value_type& value) {
        if (m_buffer.size >= m_buffer.capacity) {
            reserve(m_buffer.size + 1);
        }
        new (m_buffer.data + m_buffer.size) value_type(value);
        ++m_buffer.size;
    }

    void push_back(value_type&& value) {
        if (m_buffer.size >= m_buffer.capacity) {
            reserve(m_buffer.size + 1);
        }
        new (m_buffer.data + m_buffer.size) value_type(std::move(value));
        ++m_buffer.size;
    }

    template <class... Args>
    reference emplace_back(Args&&... args) {
        if (m_buffer.size >= m_buffer.capacity) {
            reserve(calculate_growth(m_buffer.size + 1));
        }

        pointer pos = m_buffer.data + m_buffer.size;
        new (pos) value_type(std::forward<Args>(args)...);
        ++m_buffer.size;
        return *pos;
    }

    void pop_back() {
        if (m_buffer.size > 0) {
            --m_buffer.size;
            std::destroy_at(m_buffer.data + m_buffer.size);
        }
    }

    iterator insert(const_iterator pos, const value_type& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, value_type&& value) {
        return emplace(pos, std::move(value));
    }

    iterator insert(const_iterator pos, size_type count, const value_type& value) {
        difference_type pos_offset = pos - m_buffer.data;

        if (count == 0)
            return const_cast<iterator>(pos);

        if (m_buffer.size + count > m_buffer.capacity) {
            reserve(calculate_growth(m_buffer.size + count));
            // 重新计算位置，因为reserve可能导致内存重新分配
            pos = m_buffer.data + pos_offset;
        }

        iterator pos_it = const_cast<iterator>(pos);
        if (pos_offset < static_cast<difference_type>(m_buffer.size)) {
            move_elements_backward(pos_it, m_buffer.data + m_buffer.size, pos_it + count);
        }

        std::uninitialized_fill_n(pos_it, count, value);
        m_buffer.size += count;

        return pos_it;
    }

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        difference_type pos_offset = pos - m_buffer.data;
        difference_type count = std::distance(first, last);

        if (count == 0)
            return const_cast<iterator>(pos);

        if (m_buffer.size + count > m_buffer.capacity) {
            reserve(calculate_growth(m_buffer.size + count));
            // 重新计算位置，因为reserve可能导致内存重新分配
            pos = m_buffer.data + pos_offset;
        }

        iterator pos_it = const_cast<iterator>(pos);
        if (pos_offset < static_cast<difference_type>(m_buffer.size)) {
            move_elements_backward(pos_it, m_buffer.data + m_buffer.size, pos_it + count);
        }

        std::uninitialized_copy(first, last, pos_it);
        m_buffer.size += count;

        return pos_it;
    }

    template <class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        difference_type pos_offset = pos - m_buffer.data;

        if (m_buffer.size >= m_buffer.capacity) {
            reserve(calculate_growth(m_buffer.size + 1));
            // 重新计算位置，因为reserve可能导致内存重新分配
            pos = m_buffer.data + pos_offset;
        }

        iterator pos_it = const_cast<iterator>(pos);
        if (pos_offset < static_cast<difference_type>(m_buffer.size)) {
            move_elements_backward(pos_it, m_buffer.data + m_buffer.size, pos_it + 1);
        }

        new (pos_it) value_type(std::forward<Args>(args)...);
        ++m_buffer.size;

        return pos_it;
    }

    iterator erase(const_iterator pos) {
        iterator pos_it = const_cast<iterator>(pos);
        // 先销毁要删除的元素
        std::destroy_at(pos_it);
        // 然后移动后面的元素
        std::move(pos_it + 1, m_buffer.data + m_buffer.size, pos_it);
        --m_buffer.size;
        return pos_it;
    }

    iterator erase(const_iterator first, const_iterator last) {
        if (first == last)
            return const_cast<iterator>(first);

        iterator first_it = const_cast<iterator>(first);
        iterator last_it = const_cast<iterator>(last);

        // 销毁被删除范围内的元素
        destroy_elements(first_it, last_it);

        // 移动后面的元素
        std::move(last_it, m_buffer.data + m_buffer.size, first_it);
        size_type count = last - first;
        m_buffer.size -= count;

        return first_it;
    }

    void swap(IndicatorImpBuffer& other) noexcept {
        std::swap(m_buffer.data, other.m_buffer.data);
        std::swap(m_buffer.size, other.m_buffer.size);
        std::swap(m_buffer.capacity, other.m_buffer.capacity);
    }

    // STL算法兼容接口
    template <typename UnaryPredicate>
    iterator erase_if(UnaryPredicate p) {
        iterator it = std::remove_if(begin(), end(), p);
        size_type new_size = it - begin();
        destroy_elements(m_buffer.data + new_size, m_buffer.data + m_buffer.size);
        m_buffer.size = new_size;
        return it;
    }

    template <typename Compare>
    void sort(Compare comp) {
        std::sort(begin(), end(), comp);
    }

    void sort() {
        sort(std::less<value_type>());
    }

private:
    void construct_elements(pointer first, pointer last, const value_type& value) {
        std::uninitialized_fill(first, last, value);
    }

    void destroy_elements(pointer first, pointer last) {
        std::destroy(first, last);
    }

    void move_elements_backward(pointer first, pointer last, pointer dest) {
        // 手动实现元素向后移动，避免std::move_backward的复杂性
        difference_type count = last - first;
        if (count <= 0)
            return;

        // 从后往前移动元素
        for (difference_type i = count - 1; i >= 0; --i) {
            *(dest + i) = std::move(*(first + i));
        }
    }

    size_type calculate_growth(size_type new_size) const {
        size_type current_capacity = m_buffer.capacity;
        if (current_capacity == 0) {
            return std::max(new_size, static_cast<size_type>(16));
        }
        return std::max(new_size, current_capacity * 2);
    }
};

// 非成员函数
inline void swap(IndicatorImpBuffer& lhs, IndicatorImpBuffer& rhs) noexcept {
    lhs.swap(rhs);
}

inline bool operator==(const IndicatorImpBuffer& lhs, const IndicatorImpBuffer& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

inline bool operator!=(const IndicatorImpBuffer& lhs, const IndicatorImpBuffer& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(const IndicatorImpBuffer& lhs, const IndicatorImpBuffer& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

inline bool operator<=(const IndicatorImpBuffer& lhs, const IndicatorImpBuffer& rhs) {
    return !(rhs < lhs);
}

inline bool operator>(const IndicatorImpBuffer& lhs, const IndicatorImpBuffer& rhs) {
    return rhs < lhs;
}

inline bool operator>=(const IndicatorImpBuffer& lhs, const IndicatorImpBuffer& rhs) {
    return !(lhs < rhs);
}

}  // namespace hku