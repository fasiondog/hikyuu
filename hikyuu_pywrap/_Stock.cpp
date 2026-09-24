/*
 * _Stock.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <hikyuu/serialization/Stock_serialization.h>
#include <hikyuu/KData.h>
#include "df_to_ks.h"

using namespace hku;
namespace py = pybind11;

// BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getIndex_overloads, getIndex, 1, 2)

KRecord (Stock::*getKRecord1)(size_t pos, const KQuery::KType& kType) const = &Stock::getKRecord;
KRecord (Stock::*getKRecord2)(const Datetime&,
                              const KQuery::KType& kType) const = &Stock::getKRecord;

void export_Stock(py::module& m) {
    py::class_<Stock>(m, "Stock", "The security object")
      .def(py::init<>())
      .def(py::init<const string&, const string&, const string&>(), py::arg("market"),
           py::arg("code"), py::arg("name"))
      .def(py::init<const Stock&>())

      .def("__str__", &Stock::toString)
      .def("__repr__", &Stock::toString)

      .def_property_readonly("id", &Stock::id, "The internal id")
      .def_property("market", py::overload_cast<>(&Stock::market, py::const_),
                    py::overload_cast<const string&>(&Stock::market), py::return_value_policy::copy,
                    "The market abbreviation it belongs to; the market abbreviation is the unique identifier of the market")
      .def_property("code", py::overload_cast<>(&Stock::code, py::const_),
                    py::overload_cast<const string&>(&Stock::code), py::return_value_policy::copy,
                    "The security code")
      .def_property_readonly("market_code", py::overload_cast<>(&Stock::market_code, py::const_),
                             "The market abbreviation + the security code, e.g.: sh000001")
      .def_property("name", py::overload_cast<>(&Stock::name, py::const_),
                    py::overload_cast<const string&>(&Stock::name), py::return_value_policy::copy,
                    "The security name")
      .def_property("type", py::overload_cast<>(&Stock::type, py::const_),
                    py::overload_cast<uint32_t>(&Stock::type), "The security type, see: constant")
      .def_property("valid", py::overload_cast<>(&Stock::valid, py::const_),
                    py::overload_cast<bool>(&Stock::valid), "Whether the security is currently valid")
      .def_property("start_datetime", py::overload_cast<>(&Stock::startDatetime, py::const_),
                    py::overload_cast<const Datetime&>(&Stock::startDatetime),
                    py::return_value_policy::copy, "The start date of the security")
      .def_property("last_datetime", py::overload_cast<>(&Stock::lastDatetime, py::const_),
                    py::overload_cast<const Datetime&>(&Stock::lastDatetime),
                    py::return_value_policy::copy, "The last date of the security")
      .def_property("tick", py::overload_cast<>(&Stock::tick, py::const_),
                    py::overload_cast<price_t>(&Stock::tick), "The minimum tick")
      .def_property("tick_value", py::overload_cast<>(&Stock::tickValue, py::const_),
                    py::overload_cast<price_t>(&Stock::tickValue), "The minimum tick value")
      .def_property_readonly("unit", &Stock::unit, "The per-unit value = tickValue / tick")
      .def_property("precision", py::overload_cast<>(&Stock::precision, py::const_),
                    py::overload_cast<int>(&Stock::precision), "The price precision")
      .def_property("atom", py::overload_cast<>(&Stock::atom, py::const_),
                    py::overload_cast<double>(&Stock::atom), "The minimum trading quantity, the same as min_tradeNumber")
      .def_property("min_trade_number", py::overload_cast<>(&Stock::minTradeNumber, py::const_),
                    py::overload_cast<double>(&Stock::minTradeNumber), "The minimum trading quantity")
      .def_property("max_trade_number", py::overload_cast<>(&Stock::maxTradeNumber, py::const_),
                    py::overload_cast<double>(&Stock::maxTradeNumber), "The maximum trading quantity")

      .def("is_null", &Stock::isNull, R"(is_null(self)

        Whether it is Null

        :rtype: bool)")

      .def("is_buffer", &Stock::isBuffer, R"(Whether the K-line data of the specified type is cached)")

      .def(
        "get_index_range",
        [](const Stock& self, const KQuery& query) {
            size_t start, end;
            self.getIndexRange(query, start, end);
            return py::make_tuple(start, end);
        },
        R"(get_index_range(self, query) -> (size_t, size_t)

        Get the corresponding K-line position range [start_pos, end_pos) according to the condition specified by the KQuery
        
        :param query [in] the specified query condition
        :return (start_pos, end_pos)")

      .def("get_kdata", &Stock::getKData, R"(get_kdata(self, query)

        Get the K-line data

        :param Query query: the query condition
        :return: the K-line data satisfying the query condition
        :rtype: KData)")

      .def("get_timeline_list", &Stock::getTimeLineList, R"(get_timeline_list(self, query)

        Get the time-line

        :param Query query: the query condition(the K-line type and the recovery type parameters in the query condition are useless at this time)
        :rtype: TimeLineList)")

      .def("get_trans_list", &Stock::getTransList, R"(get_trans_list(self, query)

        Get the historical tick data

        :param Query query: the query condition(the K-line type and the recovery type parameters in the query condition are useless at this time)
        :rtype: TransList)")

      .def("get_count", &Stock::getCount, py::arg("ktype") = KQuery::DAY,
           R"(get_count(self, [ktype=Query.DAY])

        Get the amount of the K-line data of the different types

        :param Query.KType ktype: the K-line data category
        :return: the number of the K-line records
        :rtype: int)")

      .def("get_market_value", &Stock::getMarketValue, R"(get_market_value(self, date, ktype)

        Get the market value at the specified moment, i.e. the close price of the last record less than or equal to the specified moment

        :param Datetime date: the specified moment
        :param Query.KType ktype: the K-line data category
        :return: the market value at the specified moment
        :rtype: float)")

      .def("get_krecord", getKRecord1, py::arg("pos"), py::arg("ktype") = KQuery::DAY,
           R"(get_krecord(self, pos[, ktype=Query.DAY])

        Get the K-line data record at the specified index, without the out-of-bounds check

        :param int pos: the specified index position
        :param Query.KType ktype: the K-line data category
        :return: the K-line record
        :rtype: KRecord)")

      .def("get_krecord", getKRecord2, py::arg("date"), py::arg("ktype") = KQuery::DAY,
           R"(get_krecord(self, date[, ktype=Query.DAY])

        Get the KRecord at the specified moment according to the data type (the daily line, the weekly line, etc.)

        :param Datetime date: the specified date-time
        :param Query.KType ktype: the K-line data category
        :return: the K-line record
        :rtype: KRecord)")

      .def("get_krecord_list", &Stock::getKRecordList, R"(get_krecord_list(self, start, end,
          ktype)

        Get the K-line records [start, end); it is generally not used directly.

        :param int start: the start position
        :param int end: the end position
        :param Query.KType ktype: the K-line category
        :return: the K-line record list
        :rtype: KRecordList)")

      .def("get_datetime_list", &Stock::getDatetimeList, R"(get_datetime_list(self, query)

        Get the date list

        :param Query query: the query condition
        :rtype: DatetimeList)")

      .def("get_finance_info", &Stock::getFinanceInfo, R"(get_finance_info(self)

        Get the current finance information

        :rtype: Parameter)")

      .def("realtime_update", &Stock::realtimeUpdate, py::arg("krecord"),
           py::arg("ktype") = KQuery::DAY,
           R"(realtime_update(self, krecord)

        Only used to update the daily-line data in the cache

        :param KRecord krecord: the newly added real-time K-line record
        :param KQuery.KType ktype: the K-line type)")

      .def("get_last_update_time", &Stock::getLastUpdateTime, py::arg("ktype") = KQuery::DAY,
           R"(get_last_update_time(self, [ktype=Query.DAY])

        Get the last update moment of the specified type of the K-line data. In the client mode, the ordinary securities are forwarded to the master process
        to take its buffer refresh moment; the temporary securities (setKRecordList) return the local writing moment.

        :param KQuery.KType ktype: the K-line type
        :rtype: Datetime)")

      .def("get_weight", &Stock::getWeight, py::arg("start") = Datetime::min(),
           py::arg("end") = Datetime(),
           R"(get_weight(self, [start, end])

        Get the dividend information within the specified time range [start, end). When the start and the end moments are not specified, get all the dividend records.

        :param Datetime start: the start moment
        :param Datetime end: the end moment
        :rtype: StockWeightList)")

      .def(
        "get_belong_to_block_list",
        [](Stock& stk, const py::object& category) {
            string c_category;
            if (!category.is_none()) {
                c_category = category.cast<string>();
            }
            return stk.getBelongToBlockList(c_category);
        },
        py::arg("category") = py::none(), R"(get_belong_to_block_list(self[, category=None])
      
      Get the list of the belonging blocks

      :param str category: the specified block category; when it is None, return the belonging blocks under all the block categories
      :rtype: list)")

      .def(
        "get_history_finance",
        [](const Stock& stk) {
            auto finances = stk.getHistoryFinance();
            py::list ret;
            for (const auto& f : finances) {
                ret.append(py::make_tuple(f.fileDate, f.reportDate, f.values));
            }
            return ret;
        },
        R"(get_history_finance(self)
        
        Get all the historical finance records; for the field information, refer to the related methods in StockManager: the get_history_finance_all_fields/get_history_finance_field_index/get_history_finance_field_name methods
        For the daily use, it is recommended to use the FINANCE indicator directly to get the finance data)")

      .def("get_trading_calendar", &Stock::getTradingCalendar, py::arg("query"),
           R"(get_trading_calendar(self, query)

        Get the trading calendar of its own market (not its own trading dates)

        :param KQuery query: the Query condition
        :return: the date list
        :rtype: DatetimeList)")

      .def("load_kdata_to_buffer", &Stock::loadKDataToBuffer, R"(load_kdata_to_buffer(self,
          ktype)

        Load the K-line data of the specified category into the memory cache; if the cache already exists, you need to release_kdata_buffer first

        :param Query.KType ktype: the K-line type)")

      .def("release_kdata_buffer", &Stock::releaseKDataBuffer, R"(release_kdata_buffer(self,
          ktype)

        Release the memory K-line data of the specified category

        :param Query.KType ktype: the K-line type)")

      .def(
        "set_krecord_list",
        [](Stock& self, const py::object& obj, const KQuery::KType& ktype) {
            if (py::isinstance<KRecordList>(obj)) {
                const auto& ks = obj.cast<const KRecordList&>();
                self.setKRecordList(ks, ktype);
            } else if (py::isinstance<py::sequence>(obj)) {
                auto seq = obj.cast<py::sequence>();
                auto ks = python_list_to_vector<KRecord>(seq);
                self.setKRecordList(std::move(ks), ktype);
            } else {
                HKU_THROW("Unusable input data type");
            }
        },
        py::arg("krecord_list"), py::arg("ktype") = KQuery::DAY,
        R"(set_krecord_list(self, krecord_list[, ktype=Query.DAY])

      "Call with caution!!! Set the current memory KRecordList directly; it is only used to set the K-line data for the external Stocks that need to be added temporarily

      :param krecord_list: KRecordList or list of KRecord
      :param Query.KType ktype: the K-line category)")

      .def(
        "set_kdata_from_df",
        [](Stock& self, const py::object& df, const StringList& cols, const KQuery::KType& ktype) {
            auto ks = df_to_krecords(df, cols);
            self.setKRecordList(std::move(ks), ktype);
        },
        py::arg("df"),
        py::arg("cols") =
          StringList{"datetime", "open", "high", "low", "close", "amount", "volume"},
        py::arg("ktype") = KQuery::DAY,
        R"(set_kdata_from_df(self, df, cols, [ktype=Query.DAY])

      Call with caution!!! Set the current memory data directly, which means the basic data of the Stock is changed.
      Get the KRecordList from the DataFrame and set it to the current Stock. df must specify the column names in order, defaulting to: ("datetime", "open", "high", "low", "close", "amount", "volume"))")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)

        DEF_PICKLE(Stock);
}
