/*
 * _StockManager.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <hikyuu/StockManager.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_StockManager(py::module& m) {
    py::class_<StockManager>(m, "StockManager", "The security information management class")
      .def_static("instance", &StockManager::instance, py::return_value_policy::reference,
                  "Get the StockManager singleton instance")

      .def(
        "init", &StockManager::init, py::arg("base_info_param"), py::arg("block_param"),
        py::arg("kdata_param"), py::arg("preload_param"), py::arg("hikyuu_param"),
        py::arg("context") = StrategyContext({"all"}),
        // The initialization (including the IPC negotiation, waiting for the data server readiness and the preloading) may take a long time; the GIL must be released,
        // otherwise all the other threads of this process are frozen; waiting for the master process to load as an IPC client appears as a hang;
        // as the master process, the service thread also needs the GIL to write the logs to sys.stdout.
        py::call_guard<py::gil_scoped_release>(),
        R"(init(self, base_info_param, block_param, kdata_param, preload_param, hikyuu_param, context)
              
    The initialization function, which must be called at the program entry
    
    :param base_info_param the basic information driver parameters
     param block_param the block information driver parameters
     param kdata_param the K-line data driver parameters
     param preload_param the preloading parameters
     param hikyuu_param the other parameters
     param StrategyContext context the strategy context, loading all the securities by default)")

      .def_property_readonly("data_ready", &StockManager::dataReady,
                             "Whether all the data is ready (loaded)")

      .def("wait_data_ready", &StockManager::waitDataReady,
           "A simple block, waiting for all the data to be ready (loaded)")

      .def("cancel_load", &StockManager::cancelLoad, "Cancel all the data loading")

      .def("reload", &StockManager::reload, py::call_guard<py::gil_scoped_release>(),
           "Reload all the security data")

      .def("reload_with", &StockManager::reloadWith, py::arg("context"),
           py::call_guard<py::gil_scoped_release>(),
           R"(reload_with(self, context)

    Reload with the strategy context parameter; if the security list in the context is empty, the original context will be used
    
    :param StrategyContext context: the strategy context)")

      .def("tmpdir", &StockManager::tmpdir, py::return_value_policy::copy, R"(tmpdir(self) -> str

    Get the temporary directory used to save the temporary variables, etc.; if not configured, it is the current directory, specified by 'tmpdir' in m_config)")

      .def("datadir", &StockManager::datadir, py::return_value_policy::copy,
           R"(datadir(self) -> str

    Get the finance data storage path)")

      .def("get_base_info_parameter", &StockManager::getBaseInfoDriverParameter,
           py::return_value_policy::copy, "Get the current basic information driver parameters")
      .def("get_block_parameter", &StockManager::getBlockDriverParameter,
           py::return_value_policy::copy, "Get the current block information driver parameters")
      .def("get_kdata_parameter", &StockManager::getKDataDriverParameter,
           py::return_value_policy::copy, "Get the current K-line data driver parameters")
      .def("get_preload_parameter", &StockManager::getPreloadParameter,
           py::return_value_policy::copy, "Get the current preloading parameters")
      .def("get_hikyuu_parameter", &StockManager::getHikyuuParameter, py::return_value_policy::copy,
           "Get the current other parameters")
      .def("get_context", &StockManager::getStrategyContext, py::return_value_policy::copy,
           "Get the current context")

      .def("set_plugin_path", &StockManager::setPluginPath, R"(set_plugin_path(self, path)
        
    Set the plugin path, which is valid only when set before the initialization)")

      .def("get_plugin_path", &StockManager::getPluginPath, py::return_value_policy::copy,
           R"(get_plugin_path(self)

    Get the plugin path)")

      .def("set_language_path", &StockManager::setLanguagePath, R"(set_language_path(self, path)
      
    Set the path of the translation files for the multi-language support, which is valid only when set before the initialization)")

      .def("get_market_list", &StockManager::getAllMarket, R"(get_market_list(self)

    Get the list of the market abbreviations

    :rtype: StringList)")

      .def("get_market_info", &StockManager::getMarketInfo, R"(get_market_info(self, market)

    Get the corresponding market information

    :param string market: the specified market identifier (the market abbreviation)
    :return: the corresponding market information; if the corresponding market information does not exist, return Null<MarketInfo>()
    :rtype: MarketInfo)")

      .def("get_market_stock", &StockManager::getMarketStock, R"(get_market_stock(self, market)

    Get the representative index of the specified market (may be empty)

    :param string market: the specified market identifier (the market abbreviation)
    :return: the corresponding market representative index; if the corresponding market information does not exist, return Null<Stock>()
    :rtype: Stock)")

      .def("get_stock_type_info", &StockManager::getStockTypeInfo,
           R"(get_stock_type_info(self, stk_type)

    Get the detailed information of the corresponding security type

    :param int stk_type: the security type, see: :py:data:`constant`
    :return: the corresponding security type information; if it does not exist, return Null<StockTypeInfo>()
    :rtype: StockTypeInfo)")

      .def(
        "get_stock_type_info_list",
        [](const StockManager& self) {
            auto stk_type_info_list = self.getStockTypeInfoList();
            size_t total = stk_type_info_list.size();
            if (total == 0) {
                return py::module_::import("pandas").attr("DataFrame")();
            }

            // Create the array
            py::array_t<uint32_t> type_arr(total);
            py::array_t<double> tick_arr(total);
            py::array_t<double> tickvalue_arr(total);
            py::array_t<double> unit_arr(total);
            py::array_t<int> precision_arr(total);
            py::array_t<double> minTradeNumber_arr(total);
            py::array_t<double> maxTradeNumber_arr(total);

            // Get the buffer and fill the data
            auto type_buf = type_arr.request();
            auto tick_buf = tick_arr.request();
            auto tickvalue_buf = tickvalue_arr.request();
            auto unit_buf = unit_arr.request();
            auto precision_buf = precision_arr.request();
            auto minTradeNumber_buf = minTradeNumber_arr.request();
            auto maxTradeNumber_buf = maxTradeNumber_arr.request();

            uint32_t* type_ptr = static_cast<uint32_t*>(type_buf.ptr);
            double* tick_ptr = static_cast<double*>(tick_buf.ptr);
            double* tickvalue_ptr = static_cast<double*>(tickvalue_buf.ptr);
            double* unit_ptr = static_cast<double*>(unit_buf.ptr);
            int* precision_ptr = static_cast<int*>(precision_buf.ptr);
            double* minTradeNumber_ptr = static_cast<double*>(minTradeNumber_buf.ptr);
            double* maxTradeNumber_ptr = static_cast<double*>(maxTradeNumber_buf.ptr);

            py::list desc_list(total);

            auto* ts = stk_type_info_list.data();
            for (size_t i = 0; i < total; i++) {
                type_ptr[i] = ts[i].type();
                tick_ptr[i] = ts[i].tick();
                tickvalue_ptr[i] = ts[i].tickValue();
                unit_ptr[i] = ts[i].unit();
                precision_ptr[i] = ts[i].precision();
                minTradeNumber_ptr[i] = ts[i].minTradeNumber();
                maxTradeNumber_ptr[i] = ts[i].maxTradeNumber();
                desc_list[i] = ts[i].description();
            }

            auto pandas = py::module_::import("pandas");
            py::dict columns;
            columns[htr("type").c_str()] = type_arr;
            columns[htr("description").c_str()] =
              pandas.attr("Series")(desc_list, py::arg("dtype") = "string");
            columns["tick"] = tick_arr;
            columns[htr("tick_value").c_str()] = tickvalue_arr;
            columns[htr("unit").c_str()] = unit_arr;
            columns[htr("precision").c_str()] = precision_arr;
            columns[htr("minTradeNumber").c_str()] = minTradeNumber_arr;
            columns[htr("maxTradeNumber").c_str()] = maxTradeNumber_arr;

            return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
        },
        R"(get_stock_type_info_list(self)

    Get the detailed information of all the security types

    :return: the detailed information of all the security types
    :rtype: DataFrame)")

      .def("get_stock", &StockManager::getStock, R"(get_stock(self, querystr)

    Get the corresponding security instance by "market abbreviation + security code"

    :param str querystr: the format: "market abbreviation + security code", e.g. "sh000001"
    :return: the corresponding security instance; if the instance does not exist, return Null<Stock>() without raising an exception
    :rtype: Stock)")

      .def(
        "get_stock_list",
        [](const StockManager& self, py::object filter) {
            StockList ret;
            if (filter.is_none()) {
                ret = self.getStockList();
            } else {
                HKU_CHECK(py::hasattr(filter, "__call__"), "filter not callable!");
                py::object filter_func = filter.attr("__call__");
                ret = self.getStockList(
                  [&](const Stock& stk) { return filter_func(stk).cast<bool>(); });
            }
            return ret;
        },
        py::arg("filter") = py::none(), R"(get_stock_list(self[, filter=None])
        
    Get the security list

    :param func filter: a filter function whose input parameter is the stock and which returns True | False)")

      .def("get_category_list", &StockManager::getAllCategory, R"(get_category_list(self)

    Get all the block categories

    :return: all the block categories
    :rtype: StringList)")

      .def("get_block", &StockManager::getBlock, R"(get_block(self, category, name)

    Get the predefined block

    :param str category: the block category
    :param str name: the block name
    :return: the block; if it cannot be found, return an empty Block
    :rtype: Block)")

      .def("add_block", &StockManager::addBlock, R"(add_block(self, block)
      
    Add the independent block to the database; the blocks are distinguished by category+name, and the same block in the database will be overwritten. Note, if the block has changed, you need to call save_block to save it again.
      
    :param Block block: the block instance)")

      .def("save_block", &StockManager::saveBlock, R"(save_block(self, block)
      
    Save the changed block to the database

    :param Block block: the block instance)")

      .def("remove_block",
           py::overload_cast<const string&, const string&>(&StockManager::removeBlock),
           py::arg("category"), py::arg("name"))
      .def("remove_block", py::overload_cast<const Block&>(&StockManager::removeBlock),
           py::arg("block"), R"(remove_block(self, block)
           
    Delete the block from the database
    
    :param Block block: the block instance)")

      .def("get_block_list", py::overload_cast<const string&>(&StockManager::getBlockList),
           py::arg("category") = "",
           R"(get_block_list(self[, category])

    Get the block list of the specified category

    :param str category: the block category; when it is an empty string, return all the block lists
    :return: the block list
    :rtype: BlockList)")

      .def("get_block_list_by_index_stock", &StockManager::getBlockListByIndexStock,
           py::arg("index_stk"),
           R"(get_block_list_by_index_stock(self, index_stk)

    Get the block list of the specified index

    :param Stock index_stk: the index
    :return: the block list
    :rtype: BlockList)")

      .def("get_trading_calendar",
           py::overload_cast<const KQuery&, const string&>(&StockManager::getTradingCalendar),
           py::arg("query"), py::arg("market") = "SH",
           R"(get_trading_calendar(self, query[, market='SH'])

    Get the trading calendar of the specified market

    :param KQuery query: the Query condition
    :param str market: the market abbreviation
    :return: the date list
    :rtype: DatetimeList)")

      .def("get_trading_calendar",
           py::overload_cast<const StockList&, const KQuery&>(&StockManager::getTradingCalendar),
           py::arg("stk_list"), py::arg("query"),
           R"(get_trading_calendar(self, stk_list, query)

    Get the superimposed trading calendar according to the specified security list (mainly used when the securities of the different markets are included)

    :param StockList stk_list: the stock list
    :param KQuery query: the Query condition
    :return: the date list
    :rtype: DatetimeList)")

      .def("add_temp_csv_stock", &StockManager::addTempCsvStock, py::arg("code"),
           py::arg("day_filename"), py::arg("min_filename"), py::arg("tick") = 0.01,
           py::arg("tick_value") = 0.01, py::arg("precision") = 2, py::arg("min_trade_num") = 1,
           py::arg("max_trade_num") = 1000000,
           R"(add_temp_csv_stock(code, day_filename, min_filename[, tick=0.01, tick_value=0.01,
        precision=2, min_trade_num = 1, max_trade_num=1000000])

    Add a temporary Stock from the CSV files (the K-line data), which can be used for the temporary testing when there are only the K-line data in the CSV format.

    The market of the added stock is "TMP"; if it needs to be obtained through sm, you need to add tmp, e.g.: sm['tmp0001']

    The first line of the CSV file is the header, which needs to contain
    Datetime (or Date, 日期), OPEN (or 开盘价), HIGH (or 最高价), LOW (or 最低价), CLOSE (or 收盘价), AMOUNT (or 成交金额), VOLUME (or VOL, COUNT, 成交量).

    Note: please make sure the csv is stored in the utf8 format, otherwise the Chinese cannot be recognized

    :param str code: the security code numbered by yourself; it cannot be the same as the existing Stock, otherwise it will return Null<Stock>
    :param str day_filename: the daily-line CSV file name
    :param str min_filename: the minute-line CSV file name
    :param float tick: the minimum tick, defaulting to 0.01
    :param float tick_value: the minimum tick value, defaulting to 0.01
    :param int precision: the price precision, defaulting to 2
    :param int min_trade_num: the minimum trading quantity per order, defaulting to 1
    :param int max_trade_num: the maximum trading quantity per order, defaulting to 1000000
    :return: the added Stock
    :rtype: Stock)",
           py::keep_alive<1, 2>())

      .def("remove_temp_csv_stock", &StockManager::removeTempCsvStock,
           R"(remove_temp_csv_stock(self, code)

    Remove the added temporary Stock

    :param str code: the custom code at the creation)")

      .def("is_holiday", &StockManager::isHoliday, R"(is_holiday(self, d)

    Judge whether the date corresponding to the specified time is a holiday (only using the A-share market)

    :param Datetime d: the time to judge)")

      .def("is_trading_hours", &StockManager::isTradingHours, py::arg("d"),
           py::arg("market") = "SH", R"(is_trading_hours(self, d)

    Judge whether the date corresponding to the specified time is the trading time

    :param Datetime d: the time to judge
    :param str market: the market abbreviation
    :return: whether it is the trading time
    :rtype: bool)")

      .def("get_history_finance_field_name", &StockManager::getHistoryFinanceFieldName,
           py::return_value_policy::copy, R"(get_history_finance_field_name(self, index)
           
    Get the corresponding field name of the historical finance information by the field index)")

      .def("get_history_finance_field_index", &StockManager::getHistoryFinanceFieldIndex,
           R"(get_history_finance_field_index(self, name)
    
    Get the index of the corresponding field of the historical finance information by the field name)")

      .def(
        "get_history_finance_all_fields",
        [](const StockManager& sm) {
            auto fields = sm.getHistoryFinanceAllFields();
            py::list ret;
            for (const auto& f : fields) {
                ret.append(py::make_tuple(f.first, f.second));
            }
            return ret;
        },
        R"(get_history_finance_all_fields(self)
    Get all the historical finance information fields and their indexes)")

      .def("add_stock", &StockManager::addStock, R"(add_stock(self, stock)
      
    Call with caution!!! It is only used to add some temporary external Stocks
    @return True | False)")

      .def("remove_stock", &StockManager::removeStock, R"(remove_stock(self, market_code)
    
    Remove the security represented by the market_code from sm; use with caution!!! It is usually used to remove the temporary external Stocks added
    
    :param str market_code: the security market identifier)")

      .def("__len__", &StockManager::size, "Return the number of the securities")
      .def("__getitem__", &StockManager::getStock, "The same as get_stock")
      .def(
        "__iter__",
        [](const StockManager& sm) {
            return py::make_iterator<py::return_value_policy::reference_internal, StockMapIterator,
                                     StockMapIterator, const Stock&>(sm.begin(), sm.end());
        },
        py::keep_alive<0, 1>());
}
