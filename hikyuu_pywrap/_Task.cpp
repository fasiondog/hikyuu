/*
 * _log.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/Log.h>
#include "task/StealTaskGroup.h"

using namespace boost::python;
using namespace hku;

class TaskWrap : public StealTaskBase, public wrapper<StealTaskBase> {
public:
    TaskWrap() : StealTaskBase() {}

    /*void invoke() {
        auto gstate = PyGILState_Ensure();
        Py_BEGIN_ALLOW_THREADS;
        Py_BLOCK_THREADS;
        try {
            run();
        } catch (std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR("Unknown error in task running!");
        }
        m_done = true;
        Py_UNBLOCK_THREADS;
        Py_END_ALLOW_THREADS;
        PyGILState_Release(gstate);
    }*/

    void run() {
        if (override call = get_override("run")) {
            call();
        } else {
            StealTaskBase::run();
        }
    }

    void default_run() {
        this->StealTaskBase::run();
    }
};

void export_task() {
    class_<TaskWrap, boost::noncopyable>("Task", init<>())
      .def("join", &StealTaskBase::join)
      .def("done", &StealTaskBase::done)
      .def("run", &StealTaskBase::run, &TaskWrap::default_run);

    class_<StealTaskGroup>("TaskGroup", init<>())
      .def(init<int>())
      .def("done", &StealTaskGroup::done)
      .def("join", &StealTaskGroup::join)
      .def("size", &StealTaskGroup::size)
      .def("stop", &StealTaskGroup::stop)
      .def("addTask", &StealTaskGroup::addTask, (arg("task"), arg("inMain") = true));

    register_ptr_to_python<TaskPtr>();
    register_ptr_to_python<TaskGroupPtr>();
}
