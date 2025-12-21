/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#include <hikyuu/plugin/KDataToClickHouseImporter.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_KDataToClickHouseImporter(py::module& m) {
    py::class_<KDataToClickHouseImporter>(m, "KDataToClickHouseImporter", "K线数据导入器")
      .def(py::init<>())
      .def("set_config", &KDataToClickHouseImporter::setConfig, py::arg("host"),
           py::arg("port") = 9000, py::arg("usr") = "default", py::arg("pwd") = "",
           "设置数据保存路径和数据源列表")
      //  .def("get_last_datetime", &KDataToClickHouseImporter::getLastDatetime,
      //       "获取指定市场指定证券最后K线时间")
      //  .def("add_krecord_list", &KDataToClickHouseImporter::addKRecordList, "添加K线数据")
      //  .def("add_timeline_list", &KDataToClickHouseImporter::addTimeLineList, "添加分时数据")
      //  .def("add_trans_list", &KDataToClickHouseImporter::addTransList, "添加分笔数据")
      //  .def("update_index", &KDataToClickHouseImporter::updateIndex, "更新索引")
      .def("remove", &KDataToClickHouseImporter::remove, "删除指定时间及其之后的K线数据");
}