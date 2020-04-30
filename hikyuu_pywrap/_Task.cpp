/*
 * _log.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/utilities/task/StealTaskGroup.h>

using namespace boost::python;
using namespace hku;

class TaskWrap : public StealTaskBase, public wrapper<StealTaskBase> {
public:
    TaskWrap() : StealTaskBase() {}

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

    class_<StealTaskGroup>("TaskGroup", init<>((arg("group_size") = 0)))
      .def("done", &StealTaskGroup::done)
      .def("join", &StealTaskGroup::join)
      .def("size", &StealTaskGroup::size)
      .def("stop", &StealTaskGroup::stop)
      .def("addTask", &StealTaskGroup::addTask);

    register_ptr_to_python<TaskPtr>();
    register_ptr_to_python<TaskGroupPtr>();
}
