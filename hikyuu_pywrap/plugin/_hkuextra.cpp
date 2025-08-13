/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-04
 *      Author: fasiondog
 */

#include <hikyuu/plugin/hkuextra.h>
#include <pybind11/functional.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_hkuextra(py::module& m) {
    m.def("register_extra_ktype",
          py::overload_cast<const string&, const string&, int32_t,
                            std::function<Datetime(const Datetime&)>>(registerExtraKType),
          py::arg("ktype"), py::arg("basetype"), py::arg("minutes"), py::arg("get_phase_end"));
    m.def("register_extra_ktype",
          py::overload_cast<const string&, const string&, int32_t>(registerExtraKType),
          py::arg("ktype"), py::arg("basetype"), py::arg("nbars"),
          R"(register_extra_ktype(ktype, basetype, nbars|minutes[, get_phase_end])
          
    注册扩展K线类型, 实现自定义动态周期K线
    
    :param str ktype: 扩展K线类型名称
    :param str basetype: 基础K线类型名称
    :param int nbars|minutes: 基础K线类型对应的基础周期数或分钟数
    :param func get_phase_end: 日期转换函数，参数为日期，返回该日期对应的周期结束日期
    :return: None)");

    m.def("release_extra_ktype", releaseExtraKType, R"(release_ktype_extra()
        
    释放扩展K线类型)");
}