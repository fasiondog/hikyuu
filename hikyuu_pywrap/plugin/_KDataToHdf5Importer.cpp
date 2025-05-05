/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#include <hikyuu/plugin/KDataToHdf5Importer.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_KDataToHdf5Importer(py::module& m) {
    py::class_<KDataToHdf5Importer>(m, "KDataToHdf5Importer", "K线数据导入器")
      .def(py::init<>())
      .def("set_config", &KDataToHdf5Importer::setConfig, "设置数据保存路径和数据源列表")
      .def("get_last_datetime", &KDataToHdf5Importer::getLastDatetime,
           "获取指定市场指定证券最后K线时间")
      .def("add_krecord_list", &KDataToHdf5Importer::addKRecordList, "添加K线数据")
      .def("update_index", &KDataToHdf5Importer::updateIndex, "更新索引");
}