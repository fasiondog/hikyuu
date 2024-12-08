/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-5-24
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_PYTHON_CONVERT_ANY_H
#define HIKYUU_PYTHON_CONVERT_ANY_H

#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <hikyuu/hikyuu.h>
#include "bind_stl.h"

using namespace hku;

Datetime pydatetime_to_Datetime(const pybind11::object& source);

namespace pybind11 {
namespace detail {
template <>
struct type_caster<boost::any> {
public:
    PYBIND11_TYPE_CASTER(boost::any, _("any"));

    /**
     * Conversion part 1 (Python->C++)
     */
    bool load(handle source, bool) {
        if (source.is_none()) {
            return false;
        }

        /* Extract PyObject from handle */
        PyObject* src = source.ptr();

        object obj = reinterpret_borrow<object>(source);
        if (PyBool_Check(src)) {
            // value = bool(PyLong_AsLong(src));
            bool tmp = obj.cast<bool>();
            value = tmp;
            return true;
        }

        if (PyLong_Check(src)) {
            int overflow;
            long tmp = PyLong_AsLongAndOverflow(src, &overflow);
            if (overflow == 0) {
                value = static_cast<int>(tmp);
            } else {
                value = PyLong_AsLongLong(src);
            }
            return true;
        }

        if (PyFloat_Check(src)) {
            value = PyFloat_AsDouble(src);
            return true;
        }

        if (PyUnicode_Check(src)) {
            object temp = reinterpret_borrow<object>(PyUnicode_AsUTF8String(src));
            if (!temp)
                pybind11_fail("Unable to extract string contents! (encoding issue)");
            char* buffer;
            ssize_t length;
            if (PYBIND11_BYTES_AS_STRING_AND_SIZE(temp.ptr(), &buffer, &length))
                pybind11_fail("Unable to extract string contents! (invalid type)");
            value = std::string(buffer, (size_t)length);
            return true;
        }

        if (isinstance<Stock>(obj)) {
            value = obj.cast<Stock>();
            return true;

        } else if (isinstance<Block>(obj)) {
            value = obj.cast<Block>();
            return true;

        } else if (isinstance<KQuery>(obj)) {
            value = obj.cast<KQuery>();
            return true;

        } else if (isinstance<KData>(obj)) {
            value = obj.cast<KData>();
            return true;

        } else if (isinstance<sequence>(obj)) {
            sequence pyseq = obj.cast<sequence>();
            size_t total = pyseq.size();
            HKU_CHECK(total > 0, "Can't support empty sequence!");
            if (isinstance<Datetime>(pyseq[0])) {
                std::vector<Datetime> vect(total);
                for (size_t i = 0; i < total; i++) {
                    vect[i] = pyseq[i].cast<Datetime>();
                }
                value = vect;

            } else if (isinstance<price_t>(pyseq[0])) {
                std::vector<price_t> vect(total);
                for (size_t i = 0; i < total; i++) {
                    vect[i] = pyseq[i].cast<price_t>();
                }
                value = vect;

            } else {
                // 尝试 python 原生 datetime 的转换
                std::vector<Datetime> vect(total);
                for (size_t i = 0; i < total; i++) {
                    vect[i] = pydatetime_to_Datetime(pyseq[i]);
                }
                value = vect;
            }
            return true;
        }

        HKU_THROW_EXCEPTION(std::logic_error,
                            "Faile convert this value to boost::any, it may be not supported!");
        return false;
    }

    /**
     * Conversion part 2 (C++ -> Python)
     */
    static handle cast(boost::any x, return_value_policy /* policy */, handle /* parent */) {
        if (x.type() == typeid(bool)) {
            bool tmp = boost::any_cast<bool>(x);
            if (tmp) {
                Py_RETURN_TRUE;
            } else {
                Py_RETURN_FALSE;
            }
        } else if (x.type() == typeid(int)) {
            return Py_BuildValue("n", boost::any_cast<int>(x));
        } else if (x.type() == typeid(double)) {
            return Py_BuildValue("d", boost::any_cast<double>(x));
        } else if (x.type() == typeid(std::string)) {
            std::string s(boost::any_cast<std::string>(x));
            return Py_BuildValue("s", s.c_str());

        } else if (x.type() == typeid(KData)) {
            const KData& k = boost::any_cast<KData>(x);
            std::stringstream cmd;
            if (k == Null<KData>()) {
                cmd << "KData()";
            } else {
                auto stk = k.getStock();
                auto query = k.getQuery();
                std::stringstream q_cmd;
                if (query.queryType() == KQuery::INDEX) {
                    q_cmd << "Query(" << query.start() << "," << query.end() << ", Query."
                          << KQuery::getKTypeName(query.kType()) << ", Query."
                          << KQuery::getRecoverTypeName(query.recoverType()) << ")";
                } else {
                    q_cmd << "Query(Datetime('" << query.startDatetime() << "'), Datetime('"
                          << query.endDatetime() << "'), " << "Query."
                          << KQuery::getKTypeName(query.kType()) << ", Query."
                          << KQuery::getRecoverTypeName(query.recoverType()) << ")";
                }
                cmd << "KData(get_stock('" << stk.market_code() << "'), " << q_cmd.str() << ")";
            }
            object o = eval(cmd.str());
            o.inc_ref();
            return o;

        } else if (x.type() == typeid(Stock)) {
            const Stock& stk = boost::any_cast<Stock>(x);
            std::stringstream cmd;
            if (stk.isNull()) {
                cmd << "Stock()";
            } else {
                cmd << "get_stock('" << stk.market_code() << "')";
            }
            object o = eval(cmd.str());
            o.inc_ref();
            return o;

        } else if (x.type() == typeid(Block)) {
            const Block& blk = boost::any_cast<const Block&>(x);
            std::stringstream cmd;
            object o;
            if (blk == Null<Block>()) {
                cmd << "Block()";
                o = eval(cmd.str());
                o.inc_ref();
            } else {
                cmd << "Block('" << blk.category() << "','" << blk.name() << "')";
                o = eval(cmd.str());
                o.inc_ref();
                Block out = o.cast<Block>();
                out.add(blk.getStockList());
            }
            return o;

        } else if (x.type() == typeid(KQuery)) {
            const KQuery& query = boost::any_cast<KQuery>(x);
            std::stringstream cmd;
            if (query.queryType() == KQuery::INDEX) {
                cmd << "Query(" << query.start() << "," << query.end() << ", Query."
                    << KQuery::getKTypeName(query.kType()) << ", Query."
                    << KQuery::getRecoverTypeName(query.recoverType()) << ")";
            } else {
                cmd << "Query(Datetime('" << query.startDatetime() << "'), Datetime('"
                    << query.endDatetime() << "'), " << "Query."
                    << KQuery::getKTypeName(query.kType()) << ", Query."
                    << KQuery::getRecoverTypeName(query.recoverType()) << ")";
            }
            object o = eval(cmd.str());
            o.inc_ref();
            return o;

        } else if (x.type() == typeid(PriceList)) {
            const PriceList& price_list = boost::any_cast<PriceList>(x);
            list o;
            for (auto iter = price_list.begin(); iter != price_list.end(); ++iter) {
                o.append(*iter);
            }
            // o.inc_ref();
            return o;

        } else if (x.type() == typeid(DatetimeList)) {
            const DatetimeList& date_list = boost::any_cast<DatetimeList>(x);
            list o;
            for (auto iter = date_list.begin(); iter != date_list.end(); ++iter) {
                o.append(*iter);
            }
            // o.inc_ref();
            return o;
        }

        HKU_THROW_EXCEPTION(std::runtime_error, "convert failed! Unkown type!!");
    }
};
}  // namespace detail
}  // namespace pybind11

#endif /* HIKYUU_PYTHON_CONVERT_ANY_H */