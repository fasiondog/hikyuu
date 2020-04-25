/*
 * StealTaskBase.cpp
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#include <iostream>
#include "StealTaskBase.h"
#include "StealTaskRunner.h"

namespace hku {

StealTaskBase::StealTaskBase() {
    _done = false;
    _runner = NULL;
}

StealTaskBase::~StealTaskBase() {}

void StealTaskBase::fork(StealTaskRunner* runner) {
    setTaskRunner(runner);
    if (runner) {
        runner->putTask(shared_from_this());
    } else {
        std::cerr << "[TaskBase::fork] Invalid Runner!" << std::endl;
    }
}

void StealTaskBase::join() {
    if (_runner) {
        _runner->taskJoin(shared_from_this());
    } else {
        std::cerr << "[TaskBase::join] Invalid Runner!" << std::endl;
    }
}

void StealTaskBase::invoke() {
    run();
    _done = true;
}

}  // namespace hku
