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

    m.def("has_factor", &hasFactor, py::arg("name"), py::arg("ktype") = KQuery::DAY,
          R"(has_factor(name[, ktype=KQuery.DAY])
    
    Check whether the factor exists

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :return: if the factor exists, return True; otherwise, return False
    :rtype: bool)");

    m.def("get_factor", &getFactor, py::arg("name"), py::arg("ktype") = KQuery::DAY,
          R"(get_factor(name[, ktype=KQuery.DAY])
    
    Get the factor metadata

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :return: the factor object; if it does not exist, return an empty factor
    :rtype: Factor)");

    m.def("save_factor", &saveFactor, py::arg("factor"), py::arg("update_before") = true,
          R"(save_factor(factor[, update_before=True])
    
    Save the factor to the database

    :param Factor factor: the factor object to save
    :param bool update_before: whether to check and update the existing factor before saving, defaulting to True). Note: it usually must be true, otherwise it will cause the data errors, unless you are certain that all the factor values have been updated
    :note: with name + ktype as the unique identifier)");

    m.def("remove_factor", &removeFactor, py::arg("name"), py::arg("ktype"),
          R"(remove_factor(name, ktype)
    
    Delete the factor from the database

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type
    :note: with name + ktype as the unique identifier)");

    m.def("get_all_factors", &getAllFactors,
          R"(get_all_factors()
    
    Get all the factor metadata

    :return: the list of all the factor objects
    :rtype: list)");

    m.def("update_all_factors_values", &updateAllFactorsValues, py::arg("ktype") = KQuery::DAY,
          R"(update_all_factors_values([ktype=KQuery.DAY])
    
    Update all the factor values

    :param KQuery.KType ktype: the K-line type, defaulting to the daily line)");

    m.def("save_factorset", &saveFactorSet, py::arg("set"),
          R"(save_factorset(set)
    
    Save the factor set to the database

    :param FactorSet set: the factor set object to save
    :note: with name + ktype as the unique identifier)");

    m.def("get_factorset", &getFactorSet, py::arg("name"), py::arg("ktype") = KQuery::DAY,
          R"(get_factorset(name[, ktype=KQuery.DAY])
    
    Get the factor set

    :param str name: the factor set name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :return: the factor set object; if it does not exist, return an empty factor set
    :rtype: FactorSet)");

    m.def("remove_factorset", &removeFactorSet, py::arg("name"), py::arg("ktype"),
          R"(remove_factorset(name, ktype)
    
    Delete the factor set from the database

    :param str name: the factor set name
    :param KQuery.KType ktype: the K-line type
    :note: with name + ktype as the unique identifier)");

    m.def("get_all_factorsets", &getAllFactorSets,
          R"(get_all_factorsets()
    
    Get all the factor sets

    :return: the list of all the factor set objects
    :rtype: list)");
}