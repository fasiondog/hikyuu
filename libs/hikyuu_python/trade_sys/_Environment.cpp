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

using namespace boost::python;
using namespace hku;

class EnvironmentWrap: public EnvironmentBase, public wrapper<EnvironmentBase>{
public:
    EnvironmentWrap(const string& name): EnvironmentBase(name) { }

    bool isValid(const string& market, const Datetime& datetime) {
        return this->get_override("isValid")(market, datetime);
    }

    void _reset() {
        this->get_override("_reset")();
    }

    EnvironmentPtr _clone() {
        return this->get_override("_clone")();
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
            .def("reset", &EnvironmentBase::reset)
            .def("clone", &EnvironmentBase::clone)
            .def("isValid", pure_virtual(&EnvironmentBase::isValid))
            .def("_reset", pure_virtual(&EnvironmentBase::_reset))
            .def("_clone", pure_virtual(&EnvironmentBase::_clone))
            ;
    register_ptr_to_python<EnvironmentPtr>();
}


