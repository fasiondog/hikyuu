/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <hikyuu/plugin/factor.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_Factor(py::module& m);
void export_FactorSet(py::module& m);

void export_factor_main(py::module& m) {
    export_Factor(m);
    export_FactorSet(m);

    m.def("get_factor", &getFactor, py::arg("name"), py::arg("ktype") = KQuery::DAY,
          R"(get_factor(name[, ktype=KQuery.DAY])
    
    获取因子元数据

    :param str name: 因子名称
    :param KQuery.KType ktype: K线类型，默认为日线
    :return: 因子对象，如果不存在则返回空因子
    :rtype: Factor)");

    m.def("save_factor", &saveFactor, py::arg("factor"),
          R"(save_factor(factor)
    
    保存因子到数据库

    :param Factor factor: 要保存的因子对象
    :note: 以 name + ktype 作为唯一标识)");

    m.def("remove_factor", &removeFactor, py::arg("name"), py::arg("ktype"),
          R"(remove_factor(name, ktype)
    
    从数据库中删除因子

    :param str name: 因子名称
    :param KQuery.KType ktype: K线类型
    :note: 以 name + ktype 作为唯一标识)");

    m.def("get_all_factors", &getAllFactors,
          R"(get_all_factors()
    
    获取所有因子元数据

    :return: 所有因子对象列表
    :rtype: list)");

    m.def("update_all_factors_values", &updateAllFactorsValues, py::arg("ktype") = KQuery::DAY,
          R"(update_all_factors_values([ktype=KQuery.DAY])
    
    更新所有因子值

    :param KQuery.KType ktype: K线类型，默认为日线)");

    m.def("save_factorset", &saveFactorSet, py::arg("set"),
          R"(save_factorset(set)
    
    保存因子集到数据库

    :param FactorSet set: 要保存的因子集对象
    :note: 以 name + ktype 作为唯一标识)");

    m.def("get_factorset", &getFactorSet, py::arg("name"), py::arg("ktype") = KQuery::DAY,
          R"(get_factorset(name[, ktype=KQuery.DAY])
    
    获取因子集

    :param str name: 因子集名称
    :param KQuery.KType ktype: K线类型，默认为日线
    :return: 因子集对象，如果不存在则返回空因子集
    :rtype: FactorSet)");

    m.def("remove_factorset", &removeFactorSet, py::arg("name"), py::arg("ktype"),
          R"(remove_factorset(name, ktype)
    
    从数据库中删除因子集

    :param str name: 因子集名称
    :param KQuery.KType ktype: K线类型
    :note: 以 name + ktype 作为唯一标识)");

    m.def("get_all_factorsets", &getAllFactorSets,
          R"(get_all_factorsets()
    
    获取所有因子集

    :return: 所有因子集对象列表
    :rtype: list)");
}