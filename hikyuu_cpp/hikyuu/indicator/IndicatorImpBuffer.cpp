/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-10
 *      Author: fasiondog
 */

#include <algorithm>
#include <stdexcept>
#include "IndicatorImpBuffer.h"

#if HKU_ENABLE_MIMALLOC
#include <mimalloc.h>
#endif

namespace hku {

// 重载new和delete操作符
void* IndicatorImpBuffer::operator new(size_t size) {
#if HKU_ENABLE_MIMALLOC
    void* ptr = mi_malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
#else
    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
#endif
}

void IndicatorImpBuffer::operator delete(void* ptr) noexcept {
#if HKU_ENABLE_MIMALLOC
    if (ptr) {
        mi_free(ptr);
    }
#else
    if (ptr) {
        std::free(ptr);
    }
#endif
}

void* IndicatorImpBuffer::operator new[](size_t size) {
#if HKU_ENABLE_MIMALLOC
    void* ptr = mi_malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
#else
    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
#endif
}

void IndicatorImpBuffer::operator delete[](void* ptr) noexcept {
#if HKU_ENABLE_MIMALLOC
    if (ptr) {
        mi_free(ptr);
    }
#else
    if (ptr) {
        std::free(ptr);
    }
#endif
}

// Buffer成员函数实现
void IndicatorImpBuffer::Buffer::allocate(IndicatorImpBuffer::size_type new_capacity) {
    if (new_capacity == 0) {
        data = nullptr;
        size = 0;
        capacity = 0;
        return;
    }

#if HKU_ENABLE_MIMALLOC
    data = static_cast<value_type*>(mi_malloc(new_capacity * sizeof(value_type)));
#else
    data = static_cast<value_type*>(std::malloc(new_capacity * sizeof(value_type)));
#endif

    if (!data) {
        throw std::bad_alloc();
    }
    capacity = new_capacity;
    size = 0;
}

void IndicatorImpBuffer::Buffer::deallocate() {
    if (data) {
#if HKU_ENABLE_MIMALLOC
        mi_free(data);
#else
        std::free(data);
#endif
        data = nullptr;
        size = 0;
        capacity = 0;
    }
}

void IndicatorImpBuffer::Buffer::reallocate(IndicatorImpBuffer::size_type new_capacity) {
    if (new_capacity == 0) {
        deallocate();
        return;
    }

    value_type* new_data;
#if HKU_ENABLE_MIMALLOC
    new_data = static_cast<value_type*>(mi_realloc(data, new_capacity * sizeof(value_type)));
#else
    new_data = static_cast<value_type*>(std::realloc(data, new_capacity * sizeof(value_type)));
#endif

    if (!new_data) {
        throw std::bad_alloc();
    }

    data = new_data;
    capacity = new_capacity;
    if (size > capacity) {
        size = capacity;
    }
}

// 私有辅助函数实现

}  // namespace hku