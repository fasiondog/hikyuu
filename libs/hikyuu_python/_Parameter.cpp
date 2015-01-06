/*
 * _Parameter.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/utilities/Parameter.h>
#include <hikyuu/Log.h>
#include "pickle_support.h"

using namespace boost::python;

namespace hku {

/*
 * boost::any To Python转换器
 */
struct AnyToPython{
    static PyObject* convert(boost::any x) {
        if (x.type() == typeid(bool)) {
            bool tmp = boost::any_cast<bool>(x);
            return tmp ? Py_True : Py_False;
            //object *o = new object(boost::any_cast<bool>(x));
            //return (*o).ptr();

        } else if (x.type() == typeid(int)) {
            return Py_BuildValue("n", boost::any_cast<int>(x));

        } else if (x.type() == typeid(double)){
            return Py_BuildValue("d", boost::any_cast<double>(x));

        } else if (x.type() == typeid(string)) {
            string s(boost::any_cast<string>(x));
            return Py_BuildValue("s", s.c_str());

        } else {
            HKU_ERROR("convert failed! Unkown type! Will return None!"
                    " [AnyToPython::convert]");
            return Py_BuildValue("s", (char *)0);
        }
    }
};

template <>
boost::any Parameter::get<boost::any>(const std::string& name) const {
    param_map_t::const_iterator iter;
    iter = m_params.find(name);
    if (iter == m_params.end()) {
        throw std::out_of_range("out_of_range in Parameter::get : " + name);
    }
    return iter->second;
}

template <>
void Parameter::add<object>(const string& name, const object& o) {
    if( have(name)){
        throw std::logic_error("Duplicate parameter! " + name);
        return;
    }

    if (PyBool_Check(o.ptr())) {
        m_params[name] = bool(extract<bool>(o));
        return;
    }

    extract<int> x2(o);
    if (x2.check()) {
        m_params[name] = x2();
        return;
    }

    extract<double> x3(o);
    if (x3.check()) {
        m_params[name] = x3();
        return;
    }

    extract<string> x4(o);
    if (x4.check()) {
        m_params[name] = x4();
        return;
    }

    throw std::logic_error("Unsuport Type! " + name);
    return;
}


template <>
void Parameter::set<object>(const string& name, const object& o) {
    if( !have(name)){
        std::logic_error("The parameter not exists! " + name);
        return;
    }

    string mismatch("Mismatch Type! " + name);
    if (m_params[name].type() == typeid(bool)) {
        extract<bool> x1(o);
        if (x1.check()) {
            m_params[name] = x1();
            return;
        }
        throw std::logic_error(mismatch);
        return;
    }

    if (m_params[name].type() == typeid(int)) {
        extract<int> x2(o);
        if (x2.check()) {
            m_params[name] = x2();
            return;
        }
        throw std::logic_error(mismatch);
        return;
    }

    if (m_params[name].type() == typeid(double)) {
        extract<double> x3(o);
        if (x3.check()) {
            m_params[name] = x3();
            return;
        }
        throw std::logic_error(mismatch);
        return;
    }

    if (m_params[name].type() == typeid(string)) {
        extract<string> x4(o);
        if (x4.check()) {
            m_params[name] = x4();
            return;
        }
        throw std::logic_error(mismatch);
        return;
    }

    throw std::logic_error("Unsupported type! " + name);
}

} /* namespace hku */


using namespace hku;

void export_Parameter() {

    to_python_converter<boost::any, AnyToPython>();

    class_<Parameter>("Parameter")
            .def(self_ns::str(self))
            .def("getNameList", &Parameter::getNameList)
            .def("have", &Parameter::have)
            .def("add", &Parameter::add<object>)
            .def("set", &Parameter::set<object>)
            .def("get", &Parameter::get<boost::any>)
            .def("getNameList", &Parameter::getNameList)
            .def("getValueList", &Parameter::getValueList)
            .def("getNameValueList", &Parameter::getNameValueList)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<Parameter>())
#endif
            ;
}

