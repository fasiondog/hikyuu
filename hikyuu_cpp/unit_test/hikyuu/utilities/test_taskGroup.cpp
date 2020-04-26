/*
 * test_Parameter.cpp
 *
 *  Created on: 2020-4-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/Log.h>
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
        HKU_INFO("{}: *****************", m_i);
    }

private:
    int m_i;
};

/** @par 检测点 */
TEST_CASE("test_TaskGroup") {
    TaskGroup tg;
    for (int i = 0; i < 30; i++) {
        tg.addTask(std::make_shared<TestTask>(i));
    }
    tg.run();
}

/** @} */