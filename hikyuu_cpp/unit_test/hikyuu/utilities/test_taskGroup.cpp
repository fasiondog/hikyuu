/*
 * test_taskGroup.cpp
 *
 *  Created on: 2020-4-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/Log.h>
#include <hikyuu/utilities/SpendTimer.h>
#include <hikyuu/utilities/task/StealTaskBase.h>
#include <hikyuu/utilities/task/StealTaskGroup.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_TaskGroup test_hikyuu_TaskGroup
 * @ingroup test_hikyuu_utilities
 * @{
 */

class TestTask : public TaskBase {
public:
    TestTask(int i) : m_i(i) {}
    virtual ~TestTask() = default;

    virtual void run() {
#if FMT_VERSION >= 90000
        HKU_INFO("{}: ------------------- [{}]", m_i, fmt::streamed(std::this_thread::get_id()));
        // fmt::print("{}: ----------------------\n", m_i);
#else
        HKU_INFO("{}: ------------------- [{}]", m_i, std::this_thread::get_id());
#endif
    }

private:
    int m_i;
};

/** @par 检测点 */
TEST_CASE("test_TaskGroup") {
    {
        std::vector<TaskPtr> task_list;
        {
            SPEND_TIME(test_TaskGroup);
            TaskGroup tg(8);
            HKU_INFO("worker_num: {}", tg.size());

#if FMT_VERSION >= 90000
            HKU_INFO("main thread: {}", fmt::streamed(std::this_thread::get_id()));
#else
            HKU_INFO("main thread: {}", std::this_thread::get_id());
#endif
            for (int i = 0; i < 10; i++) {
                task_list.push_back(tg.addTask(std::make_shared<TestTask>(i)));
                // CHECK(!task_list[i]->done());
            }
            tg.join();
        }
        CHECK(task_list.size() == 10);
        for (auto& task : task_list) {
            CHECK(task->done());
        }
    }
    // tg.run();
}

/** @} */