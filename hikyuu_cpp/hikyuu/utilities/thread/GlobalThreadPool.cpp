/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#include "GlobalThreadPool.h"
#include "GlobalMQThreadPool.h"
#include "GlobalStealThreadPool.h"
#include "GlobalMQStealThreadPool.h"

namespace hku {

#if CPP_STANDARD < CPP_STANDARD_17 || defined(__clang__)
thread_local InterruptFlag GlobalThreadPool::m_thread_need_stop;

thread_local ThreadSafeQueue<GlobalMQThreadPool::task_type>*
  GlobalMQThreadPool::m_local_work_queue = nullptr;
thread_local InterruptFlag GlobalMQThreadPool::m_thread_need_stop;

thread_local WorkStealQueue* GlobalStealThreadPool::m_local_work_queue = nullptr;
thread_local int GlobalStealThreadPool::m_index = -1;
thread_local InterruptFlag GlobalStealThreadPool::m_thread_need_stop;
thread_local std::thread::id GlobalStealThreadPool::m_thread_id;

thread_local MQStealQueue<GlobalMQStealThreadPool::task_type>*
  GlobalMQStealThreadPool::m_local_work_queue = nullptr;
thread_local int GlobalMQStealThreadPool::m_index = -1;
thread_local InterruptFlag GlobalMQStealThreadPool::m_thread_need_stop;
#endif

}  // namespace hku