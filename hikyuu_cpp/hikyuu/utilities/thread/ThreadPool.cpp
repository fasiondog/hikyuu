/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#include "ThreadPool.h"
#include "MQThreadPool.h"
#include "StealThreadPool.h"
#include "MQStealThreadPool.h"

namespace hku {

#if CPP_STANDARD < CPP_STANDARD_17
thread_local InterruptFlag ThreadPool::m_thread_need_stop;
thread_local int ThreadPool::m_index = -1;

thread_local ThreadSafeQueue<MQThreadPool::task_type>* MQThreadPool::m_local_work_queue = nullptr;
thread_local int MQThreadPool::m_index = -1;
thread_local InterruptFlag MQThreadPool::m_thread_need_stop;

thread_local WorkStealQueue* StealThreadPool::m_local_work_queue = nullptr;
thread_local int StealThreadPool::m_index = -1;
thread_local InterruptFlag StealThreadPool::m_thread_need_stop;

thread_local MQStealQueue<MQStealThreadPool::task_type>* MQStealThreadPool::m_local_work_queue =
  nullptr;
thread_local int MQStealThreadPool::m_index = -1;
thread_local InterruptFlag MQStealThreadPool::m_thread_need_stop;
#endif

}  // namespace hku