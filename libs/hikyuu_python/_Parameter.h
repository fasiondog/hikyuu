/*
 * _Parameter.h
 *
 *  Created on: 2015年3月11日
 *      Author: fasiondog
 */

#ifndef PYTHON_PARAMETER_H_
#define PYTHON_PARAMETER_H_

#include <boost/python.hpp>
#include <hikyuu/utilities/Parameter.h>
#include <hikyuu/Log.h>
#include "pickle_support.h"

namespace hku {

using namespace boost::python;

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
inline boost::any Parameter::get<boost::any>(const std::string& name) const {
    param_map_t::const_iterator iter;
    iter = m_params.find(name);
    if (iter == m_params.end()) {
        throw std::out_of_range("out_of_range in Parameter::get : " + name);
    }
    return iter->second;
}


template <>
inline void Parameter::set<object>(const string& name, const object& o) {
    if( !have(name)){
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

#endif /* PYTHON_PARAMETER_H_ */
