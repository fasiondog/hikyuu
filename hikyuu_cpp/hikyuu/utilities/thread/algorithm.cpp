/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-16
 *      Author: fasiondog
 */

// pthread_getattr_np is a GNU extension and needs the feature macro before any system header
#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "algorithm.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <sys/resource.h>
#endif

namespace hku {

std::unique_ptr<GlobalStealThreadPool> global_steal_thread_pool;

void HKU_UTILS_API init_global_task_group(size_t work_num) {
    size_t cpu_num = std::thread::hardware_concurrency();
    if (work_num == 0) {
        work_num = cpu_num;
    }
    if (!global_steal_thread_pool) {
        global_steal_thread_pool = std::make_unique<GlobalStealThreadPool>(work_num, false);
    }
}

void HKU_UTILS_API release_global_task_group() {
    if (global_steal_thread_pool) {
        global_steal_thread_pool.reset();
    }
}

namespace detail {

namespace {

struct ThreadStackInfo {
    uintptr_t low = 0;   // The lowest address the stack may grow down to
    size_t size = 0;     // Total stack size of the thread
    bool valid = false;  // false when the stack range cannot be determined
};

ThreadStackInfo queryThreadStack() {
    ThreadStackInfo info;
#if defined(_WIN32)
    // StackBase is the top of the reserved stack, while StackLimit only follows the committed
    // part of it, so the reserved range is taken by walking the whole stack allocation
    const auto* tib = reinterpret_cast<const NT_TIB*>(NtCurrentTeb());
    uintptr_t base = reinterpret_cast<uintptr_t>(tib->StackBase);
    MEMORY_BASIC_INFORMATION region;
    if (base > 0 && VirtualQuery(reinterpret_cast<LPCVOID>(base - 1), &region, sizeof(region))) {
        uintptr_t alloc = reinterpret_cast<uintptr_t>(region.AllocationBase);
        if (alloc != 0 && alloc < base) {
            uintptr_t cur = alloc;
            size_t total = 0;
            bool complete = false;
            while (cur < base) {
                if (!VirtualQuery(reinterpret_cast<LPCVOID>(cur), &region, sizeof(region)) ||
                    reinterpret_cast<uintptr_t>(region.AllocationBase) != alloc ||
                    region.RegionSize == 0) {
                    break;
                }
                total += region.RegionSize;
                cur += region.RegionSize;
                if (cur >= base) {
                    complete = true;
                }
            }
            if (complete) {
                info.low = alloc;
                info.size = total;
                info.valid = true;
            }
        }
    }
#elif defined(__APPLE__)
    void* base = pthread_get_stackaddr_np(pthread_self());
    size_t size = pthread_get_stacksize_np(pthread_self());
    if (base != nullptr && size > 0) {
        info.low = reinterpret_cast<uintptr_t>(base);
        info.size = size;
        info.valid = true;
    }
#else
    pthread_attr_t attr;
    if (pthread_getattr_np(pthread_self(), &attr) == 0) {
        void* base = nullptr;
        size_t size = 0;
        if (pthread_attr_getstack(&attr, &base, &size) == 0 && base != nullptr && size > 0) {
            info.low = reinterpret_cast<uintptr_t>(base);
            info.size = size;
            info.valid = true;
        }
        pthread_attr_destroy(&attr);
    }
    if (!info.valid) {
        struct rlimit limit;
        if (getrlimit(RLIMIT_STACK, &limit) == 0 && limit.rlim_cur > 0) {
            info.size = static_cast<size_t>(limit.rlim_cur);
            info.valid = true;
        }
    }
#endif
    // The probed address must fall inside the reported range; platforms disagree on which end the
    // stack address APIs return, so the window is shifted down a size when it was the top end
    if (info.valid) {
        volatile char probe;
        uintptr_t cur = reinterpret_cast<uintptr_t>(&probe);
        if (cur <= info.low && info.low - cur <= info.size) {
            info.low -= info.size;
        }
        if (cur < info.low || cur >= info.low + info.size) {
            info.valid = false;
        }
    }
    return info;
}

}  // namespace

size_t HKU_UTILS_API remaining_stack_size(size_t* total_size) {
    static thread_local ThreadStackInfo info = queryThreadStack();
    if (total_size != nullptr) {
        *total_size = info.size;
    }
    HKU_IF_RETURN(!info.valid || info.low == 0, 0);
    // The address of a local is a good enough approximation of the current stack pointer
    volatile char probe;
    uintptr_t cur = reinterpret_cast<uintptr_t>(&probe);
    return cur > info.low ? static_cast<size_t>(cur - info.low) : 0;
}

}  // namespace detail

}  // namespace hku
