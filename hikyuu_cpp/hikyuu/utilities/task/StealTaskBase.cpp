/*
 * StealTaskBase.cpp
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#include <iostream>
#include "../../Log.h"
#include "../exception.h"
#include "StealTaskBase.h"
#include "StealTaskRunner.h"
#include "StealTaskGroup.h"

namespace hku {

StealTaskBase::StealTaskBase() {
    m_done = false;
    m_runner = NULL;
}

StealTaskBase::~StealTaskBase() {}

void StealTaskBase::join() {
    HKU_CHECK(m_group, "This taks had not be added to any task group!");
    auto runner = m_group->getRunnerByThreadId(std::this_thread::get_id());
    runner->taskJoin(shared_from_this());
    /*if (m_runner) {
        m_runner->taskJoin(shared_from_this());
    } else {
        HKU_ERROR("Invalid runner!");
    }*/
}

void StealTaskBase::invoke() {
    run();
    m_done = true;
}

void StealTaskBase::run() {
    HKU_WARN("This is empty task!");
}

}  // namespace hku
