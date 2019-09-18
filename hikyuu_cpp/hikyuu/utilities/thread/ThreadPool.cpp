#include "ThreadPool.h"

namespace hku {

thread_local WorkStealQueue* ThreadPool::m_local_work_queue = nullptr;
thread_local unsigned ThreadPool:: m_index = 0;
thread_local bool ThreadPool::m_thread_need_stop = false;

} /* namespace hku */