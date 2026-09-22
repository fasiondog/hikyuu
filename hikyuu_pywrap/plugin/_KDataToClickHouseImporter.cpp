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
    py::class_<KDataToClickHouseImporter>(m, "KDataToClickHouseImporter",
                                          R"(KDataToClickHouseImporter()

    The K-line data importer (the ClickHouse storage), used to write the K-line / time-line / tick data obtained from the external data sources to ClickHouse,
    and supports registering the new markets and the security types to the baseinfo database.

    The actual writing is done by the import2ch plugin (a VIP plugin, requiring a valid device license). When the plugin is missing or the license is invalid, at the construction stage
    only the error logs are output; the subsequent calls behave as: set_config / add_market / add_stock_type return False,
    get_last_datetime returns an empty Datetime, and the data writing interfaces are ignored directly.

    The data organization is consistent with the hikyuu ClickHouse data driver: the K-lines are written by period to the day_k, min_k,
    min5_k, week_k, etc. tables of the hku_data database; the time-line is written to hku_data.timeline and the tick data to hku_data.transdata; within the tables, the market +
    code columns distinguish the securities; the data tables are created automatically, without needing to create the tables in advance.

    Only the basic periods need to be written: the weekly, the monthly, the quarterly, the half-year and the yearly lines are derived from the daily line, and the 15/30/60-minute lines and the 2-hour line are derived from the 5-minute line,
    therefore, add_krecord_list is usually only used together with Query.DAY, Query.MIN, Query.MIN5.

    Typical usage::

        >>> im = KDataToClickHouseImporter()
        >>> if not im.set_config('127.0.0.1', 9000, 'default', 'pwd'):  # returning False usually means there is no valid license
        ...     raise RuntimeError('KDataToClickHouseImporter set config error! Maybe no license!')
        >>> last = im.get_last_datetime('SH', '600000', Query.DAY)
        >>> ks = [KRecord(Datetime(20250102), 10.0, 10.5, 9.8, 10.2, 1200.0, 100000.0)]
        >>> im.add_krecord_list('SH', '600000', ks, Query.DAY)
        >>> im.update_index('SH', '600000', Query.DAY)
)")
      .def(py::init<>())
      .def(
        "set_config", &KDataToClickHouseImporter::setConfig, py::arg("host"),
        py::arg("port") = 9000, py::arg("usr") = "default", py::arg("pwd") = "",
        py::arg("baseinfo_db") = "hku_base",
        R"(set_config(host: str, port: int = 9000, usr: str = 'default', pwd: str = '', baseinfo_db: str = 'hku_base') -> bool

    Set the ClickHouse connection information; it must be called before the other interfaces.

    :param str host: the ClickHouse host address (required)
    :param int port: the ClickHouse TCP port
    :param str usr: the login user name
    :param str pwd: the login password
    :param str baseinfo_db: the baseinfo database name; add_market and add_stock_type write to this database; it must be consistent with the baseinfo database used in the hikyuu configuration. The K-line data is always written to the hku_data database, unaffected by this parameter
    :return: return True when the configuration succeeds; return False when the plugin is missing or the license is invalid
    :rtype: bool)")
      .def("get_last_datetime", &KDataToClickHouseImporter::getLastDatetime,
           R"(get_last_datetime(market: str, code: str, ktype: Query.KType) -> Datetime

    Get the last time of the imported data of the specified market and the specified security under the specified period, usually used to determine the start position during the incremental import.

    :param str market: the market abbreviation, e.g. 'SH'
    :param str code: the security code (excluding the market abbreviation), e.g. '600000'
    :param Query.KType ktype: the K-line period, e.g. Query.DAY, Query.MIN, Query.MIN5
    :return: the time of the last record; when there is no data, the plugin is missing or the license is invalid, return an empty Datetime (is_null is True)
    :rtype: Datetime)")
      .def("add_krecord_list", &KDataToClickHouseImporter::addKRecordList,
           R"(add_krecord_list(market: str, code: str, krecords: list, ktype: Query.KType) -> None

    Append and write the K-line data for the specified market and the specified security; after writing, you need to call update_index to update the index.

    :param str market: the market abbreviation, e.g. 'SH'
    :param str code: the security code (excluding the market abbreviation), e.g. '600000'
    :param list krecords: a list of the KRecords; for the field meanings, see KRecord (datetime, open, high, low, close, amount, volume)
    :param Query.KType ktype: the K-line period, e.g. Query.DAY, Query.MIN, Query.MIN5
    :return: None

    Note: the old hku_data tables store the market / code with FixedString (the market is at most 2 characters, and the code is at most 6 characters),
    and the market abbreviations or the security codes exceeding that cannot be written; you need to migrate the table structures to the String type.)")
      .def("add_timeline_list", &KDataToClickHouseImporter::addTimeLineList,
           R"(add_timeline_list(market: str, code: str, timeline: list) -> None

    Append and write the time-line data for the specified market and the specified security (written to the hku_data.timeline table).

    :param str market: the market abbreviation, e.g. 'SH'
    :param str code: the security code (excluding the market abbreviation), e.g. '600000'
    :param list timeline: a list of the TimeLineRecords; for the field meanings, see TimeLineRecord (date, price, vol)
    :return: None)")
      .def("add_trans_list", &KDataToClickHouseImporter::addTransList,
           R"(add_trans_list(market: str, code: str, translist: list) -> None

    Append and write the tick data for the specified market and the specified security (written to the hku_data.transdata table).

    :param str market: the market abbreviation, e.g. 'SH'
    :param str code: the security code (excluding the market abbreviation), e.g. '600000'
    :param list translist: a list of the TransRecords; for the field meanings, see TransRecord (date, price, vol, direct)
    :return: None)")
      .def("update_index", &KDataToClickHouseImporter::updateIndex,
           R"(update_index(market: str, code: str, ktype: Query.KType) -> None

    Update the data index of the specified market, the specified security and the specified period (recording the start and the end range of this security's data), usually called after writing the data in batches.

    :param str market: the market abbreviation, e.g. 'SH'
    :param str code: the security code (excluding the market abbreviation), e.g. '600000'
    :param Query.KType ktype: the K-line period, e.g. Query.DAY, Query.MIN, Query.MIN5
    :return: None)")
      .def("remove", &KDataToClickHouseImporter::remove,
           R"(remove(market: str, code: str, ktype: Query.KType, start: Datetime) -> None

    Delete all the data of the specified market, the specified security and the specified period from start onwards, usually used to re-import after clearing the erroneous data.

    :param str market: the market abbreviation, e.g. 'SH'
    :param str code: the security code (excluding the market abbreviation), e.g. '600000'
    :param Query.KType ktype: the K-line period, e.g. Query.DAY, Query.MIN, Query.MIN5
    :param Datetime start: the start time (inclusive); the data at and after this time is deleted together
    :return: None

    Note: the ClickHouse deletion is a mutation operation, executed asynchronously; the data to be deleted may still be queried within a short time.)")
      .def(
        "add_market", &KDataToClickHouseImporter::addMarket, py::arg("market"), py::arg("name"),
        py::arg("description"), py::arg("index_code"), py::arg("open1") = 930,
        py::arg("close1") = 1130, py::arg("open2") = 1300, py::arg("close2") = 1500,
        R"(add_market(market: str, name: str, description: str, index_code: str, open1: int = 930, close1: int = 1130, open2: int = 1300, close2: int = 1500) -> bool

    Register a new market to the baseinfo database, an idempotent operation: when the market already exists, skip the writing and return True.

    :param str market: the market abbreviation (automatically converted to uppercase), e.g. 'US'
    :param str name: the market name
    :param str description: the market description; it is recommended to note the time zone, e.g. 'NASDAQ/UTC-5'
    :param str index_code: the market representative index code; get_market_stock and the trading calendar depend on {market}{index_code}
    :param int open1: the morning open time, in the HHMM format, e.g. 930
    :param int close1: the morning close time, in the HHMM format, e.g. 1130
    :param int open2: the afternoon open time, in the HHMM format, e.g. 1300
    :param int close2: the afternoon close time, in the HHMM format, e.g. 1500
    :return: return True when the registration succeeds or the market already exists; return False when the license is invalid or the writing fails
    :rtype: bool

    Note: a restart (re-executing hikyuu_init) is required after the registration for it to take effect; the K-lines of the market representative index need to be imported separately.)")
      .def(
        "add_stock_type", &KDataToClickHouseImporter::addStockType, py::arg("type_id"),
        py::arg("description"), py::arg("precision") = 2, py::arg("tick") = 0.01,
        py::arg("tick_value") = 0.01, py::arg("min_trade") = 1.0, py::arg("max_trade") = 1000000.0,
        R"(add_stock_type(type_id: int, description: str, precision: int = 2, tick: float = 0.01, tick_value: float = 0.01, min_trade: float = 1.0, max_trade: float = 1000000.0) -> bool

    Register the security type to the baseinfo database, an idempotent operation: when the type_id already exists, skip the writing and return True.

    :param int type_id: the type number; by convention, the id is consistent with the type; only 10 (reusing CRYPTO) or a custom number greater than or equal to 12 is allowed
    :param str description: the type description
    :param int precision: the price precision (the number of the decimal places)
    :param float tick: the minimum tick
    :param float tick_value: the value of each tick
    :param float min_trade: the minimum trading quantity per order
    :param float max_trade: the maximum trading quantity per order
    :return: return True when the registration succeeds or the type already exists; return False when the license is invalid, the type_id is illegal or the writing fails
    :rtype: bool

    Note: a restart (re-executing hikyuu_init) is required after the registration for it to take effect.
)");
}
