/*
 * _Environment.cpp
 *
 *  Created on: 2013-3-2
 *      Author: fasiondog
 */

/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/environment/build_in.h>

using namespace boost::python;
using namespace hku;

class EnvironmentWrap: public EnvironmentBase, public wrapper<EnvironmentBase>{
public:
    EnvironmentWrap(const string& name): EnvironmentBase(name) { }

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        }
        EnvironmentBase::_reset();
    }

    void default_reset() {
        this->EnvironmentBase::_reset();
    }

    EnvironmentPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }
};

string (EnvironmentBase::*get_name)() const = &EnvironmentBase::name;
void (EnvironmentBase::*set_name)(const string&) = &EnvironmentBase::name;

void export_Environment() {
    class_<EnvironmentWrap, boost::noncopyable>("EnvironmentBase", init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", get_name, set_name)
            .def("getParam", &EnvironmentBase::getParam<boost::any>)
            .def("setParam", &EnvironmentBase::setParam<object>)
            .def("setQuery", &EnvironmentBase::setQuery)
            .def("getQuery", &EnvironmentBase::getQuery)
            .def("isValid", &EnvironmentBase::isValid)
            .def("_addValid", &EnvironmentBase::_addValid)
            .def("reset", &EnvironmentBase::reset)
            .def("clone", &EnvironmentBase::clone)
            .def("_reset", &EnvironmentBase::_reset, &EnvironmentWrap::default_reset)
            .def("_clone", pure_virtual(&EnvironmentBase::_clone))
            .def("_calculate", pure_virtual(&EnvironmentBase::_calculate))
            ;
    register_ptr_to_python<EnvironmentPtr>();

    def("EV_TwoLine", EV_TwoLine, (arg("fast"), arg("slow"), arg("market") = "SH"));
}


