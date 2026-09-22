/*
 * _KData.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/KData_serialization.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

const KRecord& (KData::*KData_getKRecord1)(size_t pos) const = &KData::getKRecord;
const KRecord& (KData::*KData_getKRecord2)(Datetime datetime) const = &KData::getKRecord;

void export_KData(py::module& m) {
    int64_t null_int64 = Null<int64_t>();
    py::class_<KData>(
      m, "KData",
      "The K-line data obtained through Stock.getKData; it is an array composed of KRecords and can be traversed like a list")
      .def(py::init<>())
      .def("__str__", &KData::toString)
      .def("__repr__", &KData::toString)

      .def_property_readonly("start_pos", &KData::startPos,
                             "Get the corresponding start position in the original K-line records; if the KData is empty, return 0")
      .def_property_readonly(
        "end_pos", &KData::endPos,
        "Get the position of the next record after the range in the original K-line records; if it is empty, return 0, otherwise it equals lastPos + 1")
      .def_property_readonly(
        "last_pos", &KData::lastPos,
        "Get the position of the last record in the original K-line records; if it is empty, return 0, otherwise it equals endPos - 1")

      .def_property_readonly("open", &KData::open,
                             "Return the Indicator instance containing the open prices, equivalent to OPEN(k)")
      .def_property_readonly("close", &KData::close,
                             "Return the Indicator instance containing the close prices, equivalent to CLOSE(k)")
      .def_property_readonly("high", &KData::high,
                             "Return the Indicator instance containing the high prices, equivalent to HIGH(k)")
      .def_property_readonly("low", &KData::low, "Return the Indicator instance containing the low prices, equivalent to LOW(k)")
      .def_property_readonly("amo", &KData::amo, "Return the Indicator instance containing the amounts, equivalent to AMO(k)")
      .def_property_readonly("vol", &KData::vol, "Return the Indicator instance containing the volumes, equivalent to VOL(k)")

      .def("get_datetime_list", &KData::getDatetimeList, R"(get_datetime_list(self)

        Return the trading date list

        :rtype: DatetimeList)")

      .def("get", KData_getKRecord1, py::return_value_policy::copy, R"(get(self, pos)

        Get the K-line record at the specified index position

        :param int pos: the position index
        :rtype: KRecord)")

      .def("get_by_datetime", KData_getKRecord2, py::return_value_policy::copy,
           R"(get_by_datetime(self, datetime)

        Get the K-line record at the specified time.

        :param Datetime datetime: the specified date
        :rtype: KRecord)")

      .def(
        "get_pos",
        [](const KData& self, const Datetime& d) {
            size_t pos = self.getPos(d);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
                ret = py::int_(pos);
            }
            return ret;
        },
        R"(get_pos(self, datetime)

        Get the index position of the K-line record at the specified time; if it is out of the data range, return None
        
        :param Datetime datetime: the specified date
        :rtype: int)")

      .def(
        "get_pos_in_stock",
        [](const KData& self, Datetime datetime) {
            size_t pos = self.getPosInStock(datetime);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
                ret = py::int_(pos);
            }
            return ret;
        },
        R"(get_pos_in_stock(self, datetime) 
        
        Get the index position in the original K-line corresponding to the specified time

        :param Datetime datetime: the specified time
        :return: the corresponding index position; if it is out of the data range, return None)")

      .def("empty", &KData::empty, R"(empty(self)

        Judge whether it is empty

        :rtype: bool)")

      .def("get_query", &KData::getQuery, py::return_value_policy::copy, R"(get_query(self)

        Get the associated query condition

        :rtype: KQuery)")

      .def("get_stock", &KData::getStock, py::return_value_policy::copy, R"(get_stock(self)

        Get the associated Stock

        :rtype: Stock)")

      .def("get_kdata", py::overload_cast<const KQuery::KType&>(&KData::getKData, py::const_),
           py::arg("ktype"), R"(get_kdata(self, ktype
           
        Get the K-line data of the other type within the same time range, e.g. the minute-line data corresponding under the daily line

        :param KQuery::KType ktype: the specified needed K-line type)")

      .def("get_kdata",
           py::overload_cast<const Datetime&, const Datetime&>(&KData::getKData, py::const_),
           R"(get_kdata(self, start_date, end_date)
      
        Get a new KData that keeps the data type and the recovery type unchanged through the current KData (note that it is not a subset of the original KData)

        :param Datetime start: the new start date
        :param Datetime end: the new end date
        :rtype: KData)")

      .def("get_kdata", py::overload_cast<const KQuery&>(&KData::getKData, py::const_),
           R"(get_kdata(query)

        Get another KData through the current KData, which is not necessarily a subset of it

        :rtype: KData)")

      .def("get_sub_kdata", &KData::getSubKData, py::arg("start"), py::arg("end") = null_int64,
           R"(get_sub_kdata(start, end = Null<int64_t>)

        Get a subset of itself by the index

        :param int start: the start index
        :param int end: the end index
        :rtype: KData)")

      .def("tocsv", &KData::tocsv, R"(tocsv(self, filename)

        Save the data to a CSV file

        :param str filename: the specified file name to save)")

      .def("__len__", &KData::size)

      .def(py::self == py::self)
      .def(py::self != py::self)

      .def("__getitem__",
           [](const KData& self, py::object obj) {
               py::object ret;
               if (py::isinstance<py::int_>(obj)) {
                   int64_t i = obj.cast<int64_t>();
                   int64_t length = self.size();
                   int64_t index = i < 0 ? length + i : i;
                   if (index < 0 || index >= length)
                       throw std::out_of_range(fmt::format("index out of range: {}", i));
                   ret = py::cast(self.getKRecord(index));
                   return ret;
               } else if (py::isinstance<Datetime>(obj)) {
                   Datetime dt = py::cast<Datetime>(obj);
                   auto krecord = self.getKRecord(dt);
                   if (!krecord.isValid()) {
                       throw std::out_of_range(fmt::format("datetime out of range: {}", dt));
                   }
                   ret = py::cast(krecord);
                   return ret;
               } else if (py::isinstance<py::str>(obj)) {
                   Datetime dt = Datetime(py::cast<std::string>(obj));
                   auto krecord = self.getKRecord(dt);
                   if (!krecord.isValid()) {
                       throw std::out_of_range(fmt::format("datetime out of range: {}", dt));
                   }
                   ret = py::cast(krecord);
                   return ret;
               } else if (py::isinstance<py::slice>(obj)) {
                   py::slice slice = py::cast<py::slice>(obj);
                   size_t start, stop, step, length;

                   if (!slice.compute(self.size(), &start, &stop, &step, &length)) {
                       throw std::invalid_argument("Invalid slice parameters");
                   }

                   KRecordList result;
                   result.reserve(length);
                   for (size_t i = 0; i < length; ++i) {
                       size_t index = start + i * step;
                       result.push_back(self[static_cast<size_t>(index)]);
                   }

                   ret = py::cast(result);
                   return ret;
               }

               throw std::out_of_range("Error index type");
           })

      .def(
        "__iter__",
        [](const KData& self) {
            return py::make_iterator<py::return_value_policy::reference_internal>(self.cbegin(),
                                                                                  self.cend());
        },
        py::keep_alive<0, 1>())

      .def(
        "to_np",
        [](const KData& kdata) {
            size_t total = kdata.size();
            HKU_IF_RETURN(total == 0, py::array());

            struct RawData {
                int64_t datetime;  // The converted millisecond timestamp
                double open;
                double high;
                double low;
                double close;
                double amount;
                double volume;
            };

            RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
            for (size_t i = 0; i < total; i++) {
                const KRecord& k = kdata[i];
                data[i].datetime = k.datetime.timestamp() * 1000LL;
                data[i].open = k.openPrice;
                data[i].high = k.highPrice;
                data[i].low = k.lowPrice;
                data[i].close = k.closePrice;
                data[i].amount = k.transAmount;
                data[i].volume = k.transCount;
            }

            // Define the NumPy structured data type
            auto dtype = py::dtype(
              vector_to_python_list<string>(
                {"datetime", "open", "high", "low", "close", "amount", "volume"}),
              vector_to_python_list<string>({"datetime64[ns]", "d", "d", "d", "d", "d", "d"}),
              vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);

            return py::array(dtype, total, static_cast<RawData*>(data),
                             py::capsule(data, [](void* p) { std::free(p); }));
        },
        "Convert the KData to a NumPy array")

      .def(
        "to_df",
        [](const KData& self, bool with_stock) {
            size_t total = self.size();
            if (total == 0) {
                return py::module_::import("pandas").attr("DataFrame")();
            }

            // Create the array
            py::array_t<int64_t> datetime_arr(total);
            py::array_t<double> open_arr(total);
            py::array_t<double> high_arr(total);
            py::array_t<double> low_arr(total);
            py::array_t<double> close_arr(total);
            py::array_t<double> amount_arr(total);
            py::array_t<double> vol_arr(total);

            // Get the buffer and fill the data
            auto datetime_buf = datetime_arr.request();
            auto open_buf = open_arr.request();
            auto high_buf = high_arr.request();
            auto low_buf = low_arr.request();
            auto close_buf = close_arr.request();
            auto amount_buf = amount_arr.request();
            auto vol_buf = vol_arr.request();

            int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
            double* open_ptr = static_cast<double*>(open_buf.ptr);
            double* high_ptr = static_cast<double*>(high_buf.ptr);
            double* low_ptr = static_cast<double*>(low_buf.ptr);
            double* close_ptr = static_cast<double*>(close_buf.ptr);
            double* amount_ptr = static_cast<double*>(amount_buf.ptr);
            double* vol_ptr = static_cast<double*>(vol_buf.ptr);

            auto* ks = self.data();
            for (size_t i = 0; i < total; i++) {
                datetime_ptr[i] = ks[i].datetime.timestamp() * 1000LL;
                open_ptr[i] = ks[i].openPrice;
                high_ptr[i] = ks[i].highPrice;
                low_ptr[i] = ks[i].lowPrice;
                close_ptr[i] = ks[i].closePrice;
                amount_ptr[i] = ks[i].transAmount;
                vol_ptr[i] = ks[i].transCount;
            }

            // Build the DataFrame
            auto pandas = py::module_::import("pandas");
            py::dict columns;
            if (with_stock) {
                py::list code_list(total);
                py::list name_list(total);
                auto stk = self.getStock();
                auto code = py::str(stk.code());
                auto name = py::str(stk.name());
                for (size_t i = 0; i < total; i++) {
                    code_list[i] = code;
                    name_list[i] = name;
                }
                columns["market_code"] =
                  pandas.attr("Series")(code_list, py::arg("dtype") = "string");
                columns["name"] = pandas.attr("Series")(name_list, py::arg("dtype") = "string");
            }

            columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
            columns["open"] = open_arr;
            columns["high"] = high_arr;
            columns["low"] = low_arr;
            columns["close"] = close_arr;
            columns["amount"] = amount_arr;
            columns["volume"] = vol_arr;

            return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
        },
        py::arg("with_stock") = false, R"(to_df(self, with_stock=False) -> pandas.DataFrame

    Convert to a pandas DataFrame
        
    :param bool with_stock: include the code and the name of the Stock
    :rtype: pandas.DataFrame)")

        DEF_PICKLE(KData);
}
