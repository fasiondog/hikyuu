/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#include "../osdef.h"
#include "GlobalStealThreadPool.h"

namespace hku {

#if HKU_OS_WINDOWS
WorkStealQueue* GlobalStealThreadPool::m_local_work_queue = nullptr;
int GlobalStealThreadPool::m_index = -1;
InterruptFlag GlobalStealThreadPool::m_thread_need_stop;
std::thread::id GlobalStealThreadPool::m_thread_id;

#else

#if CPP_STANDARD < CPP_STANDARD_17 || defined(__clang__)
thread_local WorkStealQueue* GlobalStealThreadPool::m_local_work_queue = nullptr;
thread_local int GlobalStealThreadPool::m_index = -1;
thread_local InterruptFlag GlobalStealThreadPool::m_thread_need_stop;
thread_local std::thread::id GlobalStealThreadPool::m_thread_id;
#endif

#endif

}  // namespace hku