/*
 * StealTaskBase.cpp
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#include <iostream>
#include "../../Log.h"
#include "StealTaskBase.h"
#include "StealTaskRunner.h"
#include "StealTaskGroup.h"

namespace hku {

StealTaskBase::StealTaskBase() : m_done(false), m_group(nullptr) {}

StealTaskBase::~StealTaskBase() {}

void StealTaskBase::join() {
    if (m_done) {
        return;
    }

    if (StealTaskRunner::m_local_runner) {
        // 当前在子线程中
        StealTaskRunner::m_local_runner->taskJoin(shared_from_this());
    } else {
        // 当前在主线程中
        HKU_CHECK(m_group, "This taks had not be added to any task group!");
        m_group->taskJoinInMaster(shared_from_this());
    }
}

void StealTaskBase::invoke() {
    try {
        run();
    } catch (std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR("Unknown error in task running!");
    }
    m_done = true;
}

void StealTaskBase::run() {
    HKU_WARN("This is empty task!");
}

}  // namespace hku
