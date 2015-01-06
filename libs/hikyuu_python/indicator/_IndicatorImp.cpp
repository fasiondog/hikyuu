/*
 * _IndicatorImp.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/indicator/Indicator.h>

using namespace boost::python;
using namespace hku;

class IndicatorImpWrap: public IndicatorImp, public wrapper<IndicatorImp> {
public:
    IndicatorImpWrap(): IndicatorImp() {}
    IndicatorImpWrap(int discard, size_t result_num): IndicatorImp(discard, result_num) {}
    IndicatorImpWrap(const Indicator& indicator, int discard, size_t result_num)
               : IndicatorImp(indicator, discard, result_num) {}

    string name() const {
        if (override name = this->get_override("name"))
#if defined(BOOST_WINDOWS)
            return call<char const*>(name.ptr());
#else
            return name();
#endif
        return IndicatorImp::name();
    }

    string default_name() const {
        return this->IndicatorImp::name();
    }

    IndicatorImpPtr operator()(const Indicator& ind) {
        if (override call = get_override("__call__")) {
            return call(ind);
        }
        return IndicatorImp::operator()(ind);
    }

    IndicatorImpPtr default_call(const Indicator& ind) {
        return this->IndicatorImp::operator()(ind);
    }
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(_set_overloads, _set, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(_append_overloads, _append, 1, 2)

void export_IndicatorImp() {
    class_<IndicatorImpWrap, boost::noncopyable>("IndicatorImp", init<>())
            .def(init<size_t, size_t>())
            .def(init<const Indicator&, size_t, size_t>())
            .def(self_ns::str(self))
            .add_property("discard", &IndicatorImp::discard)
            .def("name", &IndicatorImp::name, &IndicatorImpWrap::default_name)
            .def("_set", &IndicatorImp::_set, _set_overloads())
            .def("_append", &IndicatorImp::_append, _append_overloads())
            .def("getResultNumber", &IndicatorImp::getResultNumber)
            .def("getResultAsPriceList", &IndicatorImp::getResultAsPriceList)
            .def("__call__", &IndicatorImp::operator(), &IndicatorImpWrap::default_call)
            ;

    register_ptr_to_python<IndicatorImpPtr>();
}

