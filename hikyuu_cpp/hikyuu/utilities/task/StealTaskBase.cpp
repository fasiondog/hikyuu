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

namespace hku {

StealTaskBase::StealTaskBase() {
    m_done = false;
    m_runner = NULL;
}

StealTaskBase::~StealTaskBase() {}

void StealTaskBase::join() {
    if (m_runner) {
        m_runner->taskJoin(shared_from_this());
    } else {
        HKU_ERROR("Invalid runner!");
    }
}

void StealTaskBase::invoke() {
    run();
    m_done = true;
}

void StealTaskBase::run() {
    HKU_WARN("This is empty task!");
}

}  // namespace hku
