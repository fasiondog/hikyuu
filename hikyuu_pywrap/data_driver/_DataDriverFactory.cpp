/*
 * _DataDriverFactory.cpp
 *
 *  Created on: 2017年10月7日
 *      Author: fasiondog
 */

#include <hikyuu/data_driver/DataDriverFactory.h>
#include "_BlockInfoDriver.h"

using namespace hku;
namespace py = pybind11;

void export_DataDriverFactory(py::module& m) {
    py::class_<DataDriverFactory>(m, "DataDriverFactory", "数据驱动工厂类")
      .def_static("getBaseInfoDriver", &DataDriverFactory::getBaseInfoDriver)
      .def_static("removeBaseInfoDriver", &DataDriverFactory::removeBaseInfoDriver)
      .def_static("getKDataDriverPool", &DataDriverFactory::getKDataDriverPool)
      .def_static("removeKDataDriver", &DataDriverFactory::removeKDataDriver)
      .def_static("getBlockDriver", &DataDriverFactory::getBlockDriver)
      .def_static("removeBlockDriver", &DataDriverFactory::removeBlockDriver)

      // .def_static("regBaseInfoDriver",
      //             [](py::object pydriver) {
      //                 auto keep_python_state_alive = std::make_shared<py::object>(pydriver);
      //                 auto ptr = pydriver.cast<PyBaseInfoDriver*>();
      //                 auto driver = BaseInfoDriverPtr(keep_python_state_alive, ptr);
      //                 DataDriverFactory::regBaseInfoDriver(driver);
      //             })

      .def_static("regBlockDriver",
                  [](py::object pydriver) {
                      auto keep_python_state_alive = std::make_shared<py::object>(pydriver);
                      auto ptr = pydriver.cast<PyBlockInfoDriver*>();
                      auto driver = BlockInfoDriverPtr(keep_python_state_alive, ptr);
                      DataDriverFactory::regBlockDriver(driver);
                  })

      // .def_static("regKDataDriver",
      //             [](py::object pydriver) {
      //                 auto keep_python_state_alive = std::make_shared<py::object>(pydriver);
      //                 auto ptr = pydriver.cast<PyKDataDriver*>();
      //                 auto driver = KDataDriverPtr(keep_python_state_alive, ptr);
      //                 DataDriverFactory::regKDataDriver(driver);
      //             })

      ;
}
