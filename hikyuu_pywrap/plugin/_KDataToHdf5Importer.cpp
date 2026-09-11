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
      .def("set_config", &KDataToHdf5Importer::setConfig, py::arg("datapath"),
           py::arg("markets") = vector<string>({"SH", "SZ", "BJ"}),
           py::arg("ktypes") = vector<string>({"DAY", "MIN", "MIN5", "TIMELINE", "TRANSDATA"}),
           py::arg("baseinfo_path") = "",
           "设置数据保存路径和数据源列表；baseinfo_path 为 baseinfo sqlite3 库路径"
           "（用于 add_market/add_stock_type）")
      .def("get_last_datetime", &KDataToHdf5Importer::getLastDatetime,
           "获取指定市场指定证券最后K线时间")
      .def("add_krecord_list", &KDataToHdf5Importer::addKRecordList, "添加K线数据")
      .def("add_timeline_list", &KDataToHdf5Importer::addTimeLineList, "添加分时数据")
      .def("add_trans_list", &KDataToHdf5Importer::addTransList, "添加分笔数据")
      .def("update_index", &KDataToHdf5Importer::updateIndex, "更新索引")
      .def("remove", &KDataToHdf5Importer::remove, "删除指定时间及其之后的K线数据")
      .def("add_market", &KDataToHdf5Importer::addMarket, py::arg("market"), py::arg("name"),
           py::arg("description"), py::arg("index_code"), py::arg("open1") = 930,
           py::arg("close1") = 1130, py::arg("open2") = 1300, py::arg("close2") = 1500,
           "向 baseinfo 库注册新市场（注册后须重启 hikyuu_init 生效）")
      .def("add_stock_type", &KDataToHdf5Importer::addStockType, py::arg("type_id"),
           py::arg("description"), py::arg("precision") = 2, py::arg("tick") = 0.01,
           py::arg("tick_value") = 0.01, py::arg("min_trade") = 1.0,
           py::arg("max_trade") = 1000000.0,
           "向 baseinfo 库注册证券类型（type_id 须为 10 或 >=12；注册后须重启 hikyuu_init 生效）");
}