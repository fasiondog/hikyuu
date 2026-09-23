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
          
    Register the extended K-line type, implementing the custom dynamic period K-lines.

    Way 1: compose strictly based on the number of the Bars, e.g.: register_extra_ktype("DAY4, "DAY", 4)
    Way 2: compose based on the time periods; you need to define the conversion function that calculates the period end time from the current time, e.g. to define a 7-minute K-line:

      def get_min3_phase_end(datetime):
          m = datetime.minute
          if m % 3 == 0:
              return datetime
          m = (m / 3 + 1) * 3
          return Datetime(datetime.year, datetime.month, datetime.day, datetime.hour, m)

      register_extra_ktype("MIN3", "MIN", 3, get_min3_phase_end)

    Note:
    1. Hikyuu has built in the DAY3, DAY5, DAY7 based on the number of the Bars, and the MIN3 extended K-line based on the time conversion
    2. It is recommended to create the custom K-line type period end point calculation conversion function in the way of the hub c++ part, because python has a GIL lock, 
       and creating the conversion function may make it impossible to calculate with multiple threads
    3. Registering the dynamic K-lines is not thread-safe; please perform the other operations after the registration is completed
    
    :param str ktype: the extended K-line type name
    :param str basetype: the basic K-line type name
    :param int nbars|minutes: the number of the basic periods or the minutes corresponding to the basic K-line type
    :param func get_phase_end: the date conversion function, whose parameter is a date and which returns the period end date corresponding to that date
    :return: None)");

    m.def("release_extra_ktype", releaseExtraKType, R"(release_ktype_extra()
        
    Release the extended K-line type, used internally. It is called when python exits, preventing the defined python conversion functions from causing an exit crash)");

    m.def("enable_kdata_cache", enableKDataCache, py::arg("enable"),
          R"(enable_kdata_cache(enable)

    Enable or disable the K-line data cache

    :param bool enable: whether to enable the K-line data cache
    :return: None)");
}