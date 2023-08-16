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
#include <hikyuu/KData.h>
#include "pybind_utils.h"
#include "pickle_support.h"

namespace hku {

using namespace boost::python;

/*
 * boost::any To Python转换器
 */
struct AnyToPython {
    static PyObject* convert(boost::any x) {
        if (x.type() == typeid(bool)) {
            bool tmp = boost::any_cast<bool>(x);
            return tmp ? Py_True : Py_False;
            // object *o = new object(boost::any_cast<bool>(x));
            // return (*o).ptr();

        } else if (x.type() == typeid(int)) {
            return Py_BuildValue("i", boost::any_cast<int>(x));

        } else if (x.type() == typeid(int64_t)) {
            return Py_BuildValue("L", boost::any_cast<int64_t>(x));

        } else if (x.type() == typeid(double)) {
            return Py_BuildValue("d", boost::any_cast<double>(x));

        } else if (x.type() == typeid(string)) {
            string s(boost::any_cast<string>(x));
            return Py_BuildValue("s", s.c_str());

        } else if (x.type() == typeid(Stock)) {
            const Stock& stk = boost::any_cast<Stock>(x);
            string cmd;
            if (stk.isNull()) {
                cmd = "Stock()";
            } else {
                cmd = "getStock('" + stk.market_code() + "')";
            }
            object o = eval(cmd.c_str());
            return boost::python::incref(o.ptr());

        } else if (x.type() == typeid(KQuery)) {
            const KQuery& query = boost::any_cast<KQuery>(x);
            std::stringstream cmd(std::stringstream::out);
            if (query.queryType() == KQuery::INDEX) {
                cmd << "QueryByIndex(" << query.start() << "," << query.end() << ", Query."
                    << KQuery::getKTypeName(query.kType()) << ", Query."
                    << KQuery::getRecoverTypeName(query.recoverType()) << ")";
            } else {
                cmd << "QueryByDate(Datetime(" << query.startDatetime() << "), Datetime("
                    << query.endDatetime() << "), "
                    << "Query." << KQuery::getKTypeName(query.kType()) << "Query."
                    << KQuery::getRecoverTypeName(query.recoverType()) << ")";
            }
            object o = eval(cmd.str().c_str());
            return boost::python::incref(o.ptr());

        } else if (x.type() == typeid(KData)) {
            KData kdata = boost::any_cast<KData>(x);
            Stock stock = kdata.getStock();
            KQuery query = kdata.getQuery();
            std::stringstream cmd;
            if (stock.isNull()) {
                cmd << "KData()";
            } else {
                cmd << "getStock('" << stock.market_code() << "').getKData(";
                if (query.queryType() == KQuery::INDEX) {
                    cmd << "QueryByIndex(" << query.start() << "," << query.end() << ", Query."
                        << KQuery::getKTypeName(query.kType()) << ", Query."
                        << KQuery::getRecoverTypeName(query.recoverType()) << ")";
                } else {
                    cmd << "QueryByDate(Datetime(" << query.startDatetime() << "), Datetime("
                        << query.endDatetime() << "), "
                        << "Query." << KQuery::getKTypeName(query.kType()) << "Query."
                        << KQuery::getRecoverTypeName(query.recoverType()) << ")";
                }
            }
            std::cout << cmd.str() << std::endl;
            object o = eval(cmd.str().c_str());
            return boost::python::incref(o.ptr());

        } else if (x.type() == typeid(PriceList)) {
            const PriceList& price_list = boost::any_cast<PriceList>(x);
            boost::python::list o;
            for (auto iter = price_list.begin(); iter != price_list.end(); ++iter) {
                o.append(*iter);
            }
            return boost::python::incref(o.ptr());

        } else if (x.type() == typeid(DatetimeList)) {
            const DatetimeList& date_list = boost::any_cast<DatetimeList>(x);
            boost::python::list o;
            for (auto iter = date_list.begin(); iter != date_list.end(); ++iter) {
                o.append(*iter);
            }
            return boost::python::incref(o.ptr());

        } else {
            HKU_ERROR("convert failed! Unkown type! Will return None!");
            return Py_BuildValue("s", (char*)0);
        }
    }
};

template <>
inline void Parameter::set<object>(const string& name, const object& o) {
    if (o.is_none()) {
        HKU_THROW_EXCEPTION(std::logic_error, "Not support None!");
    }

    if (!have(name)) {
        if (PyBool_Check(o.ptr())) {
            m_params[name] = bool(extract<bool>(o));
            return;
        }

        extract<int> x2(o);
        if (x2.check()) {
            int overflow;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
            long val = PyLong_AsLongAndOverflow(o.ptr(), &overflow);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
            if (overflow == 0) {
                m_params[name] = x2();
            } else {
                int64_t long_val = PyLong_AsLongLong(o.ptr());
                m_params[name] = long_val;
            }
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

        extract<Stock> x5(o);
        if (x5.check()) {
            m_params[name] = x5();
            return;
        }

        extract<KQuery> x6(o);
        if (x6.check()) {
            m_params[name] = x6();
            return;
        }

        extract<KData> x7(o);
        if (x7.check()) {
            m_params[name] = x7();
            return;
        }

        try {
            boost::python::list pylist(o);
            size_t total = len(pylist);
            if (total > 0) {
                extract<price_t> x8(pylist[0]);
                if (x8.check()) {
                    m_params[name] = python_list_to_vector<PriceList>(pylist);
                    return;
                }

                extract<Datetime> x9(pylist[0]);
                if (x9.check()) {
                    m_params[name] = python_list_to_vector<DatetimeList>(pylist);
                    return;
                }
            }

        } catch (...) {
            // do nothing
        }

        throw std::logic_error("Unsuport Type! " + name);
    }

    string mismatch("Mismatch Type! " + name);
    if (m_params[name].type() == typeid(bool)) {
        extract<bool> x1(o);
        if (x1.check()) {
            m_params[name] = x1();
            return;
        }
        throw std::logic_error(mismatch);
    }

    if (m_params[name].type() == typeid(int)) {
        extract<int> x2(o);
        if (x2.check()) {
            m_params[name] = x2();
            return;
        }
        throw std::logic_error(mismatch);
    }

    if (m_params[name].type() == typeid(int64_t)) {
        int64_t long_val = PyLong_AsLongLong(o.ptr());
        m_params[name] = long_val;
        return;
    }

    if (m_params[name].type() == typeid(double)) {
        extract<double> x3(o);
        if (x3.check()) {
            m_params[name] = x3();
            return;
        }
        throw std::logic_error(mismatch);
    }

    if (m_params[name].type() == typeid(string)) {
        extract<string> x4(o);
        if (x4.check()) {
            m_params[name] = x4();
            return;
        }
        throw std::logic_error(mismatch);
    }

    if (m_params[name].type() == typeid(Stock)) {
        extract<Stock> x5(o);
        if (x5.check()) {
            m_params[name] = x5();
            return;
        }
        throw std::logic_error(mismatch);
    }

    if (m_params[name].type() == typeid(KQuery)) {
        extract<KQuery> x6(o);
        if (x6.check()) {
            m_params[name] = x6();
            return;
        }
        throw std::logic_error(mismatch);
    }

    if (m_params[name].type() == typeid(KData)) {
        extract<KData> x7(o);
        if (x7.check()) {
            m_params[name] = x7();
            return;
        }
        throw std::logic_error(mismatch);
    }

    if (m_params[name].type() == typeid(PriceList)) {
        m_params[name] = python_list_to_vector<PriceList>(boost::python::list(o));
        return;
    }

    if (m_params[name].type() == typeid(DatetimeList)) {
        m_params[name] = python_list_to_vector<DatetimeList>(boost::python::list(o));
        return;
    }

    throw std::logic_error("Unsupported type! " + name);
}

} /* namespace hku */

#endif /* PYTHON_PARAMETER_H_ */
