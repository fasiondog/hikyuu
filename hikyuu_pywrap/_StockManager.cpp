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
    py::class_<StockManager>(m, "StockManager", "证券信息管理类")
      .def_static("instance", &StockManager::instance, py::return_value_policy::reference,
                  "获取StockManager单例实例")

      .def(
        "init", &StockManager::init, py::arg("base_info_param"), py::arg("block_param"),
        py::arg("kdata_param"), py::arg("preload_param"), py::arg("hikyuu_param"),
        py::arg("context") = StrategyContext({"all"}),
        R"(init(self, base_info_param, block_param, kdata_param, preload_param, hikyuu_param, context)
              
    初始化函数，必须在程序入口调用
    
    :param base_info_param 基础信息驱动参数
     param block_param 板块信息驱动参数
     param kdata_param K线数据驱动参数
     param preload_param 预加载参数
     param hikyuu_param 其他参数
     param StrategyContext context 策略上下文, 默认加载全部证券)")

      .def_property_readonly("data_ready", &StockManager::dataReady,
                             "是否所有数据已准备就绪（加载完毕）")

      .def("cancel_load", &StockManager::cancelLoad, "取消所有数据加载")

      .def("reload", &StockManager::reload, "重新加载所有证券数据")

      .def("tmpdir", &StockManager::tmpdir, R"(tmpdir(self) -> str

    获取用于保存零时变量等的临时目录，如未配置则为当前目录 由m_config中的“tmpdir”指定)")

      .def("datadir", &StockManager::datadir, R"(datadir(self) -> str

    获取财务数据存放路径)")

      .def("get_base_info_parameter", &StockManager::getBaseInfoDriverParameter,
           py::return_value_policy::copy, "获取当前基础信息驱动参数")
      .def("get_block_parameter", &StockManager::getBlockDriverParameter,
           py::return_value_policy::copy, "获取当前板块信息驱动参数")
      .def("get_kdata_parameter", &StockManager::getKDataDriverParameter,
           py::return_value_policy::copy, "获取当前K线数据驱动参数")
      .def("get_preload_parameter", &StockManager::getPreloadParameter,
           py::return_value_policy::copy, "获取当前预加载参数")
      .def("get_hikyuu_parameter", &StockManager::getHikyuuParameter, py::return_value_policy::copy,
           "获取当前其他参数")
      .def("get_context", &StockManager::getStrategyContext, py::return_value_policy::copy,
           "获取当前上下文")

      .def("set_plugin_path", &StockManager::setPluginPath, R"(set_plugin_path(self, path)
        
    设置插件路径，仅在初始化之前设置有效)")

      .def("get_plugin_path", &StockManager::getPluginPath, py::return_value_policy::copy,
           R"(get_plugin_path(self)

    获取插件路径)")

      .def("set_language_path", &StockManager::setLanguagePath, R"(set_language_path(self, path)
      
    设置多语言支持的翻译文件所在路径，仅在初始化之前设置有效)")

      .def("get_market_list", &StockManager::getAllMarket, R"(get_market_list(self)

    获取市场简称列表

    :rtype: StringList)")

      .def("get_market_info", &StockManager::getMarketInfo, R"(get_market_info(self, market)

    获取相应的市场信息

    :param string market: 指定的市场标识（市场简称）
    :return: 相应的市场信息，如果相应的市场信息不存在，则返回Null<MarketInfo>()
    :rtype: MarketInfo)")

      .def("get_market_stock", &StockManager::getMarketStock, R"(get_market_stock(self, market)

    获取指定市场的代表指数（可能为空）

    :param string market: 指定的市场标识（市场简称）
    :return: 相应的市场代表指数，如果相应的市场信息不存在，则返回Null<Stock>()
    :rtype: Stock)")

      .def("get_stock_type_info", &StockManager::getStockTypeInfo,
           R"(get_stock_type_info(self, stk_type)

    获取相应的证券类型详细信息

    :param int stk_type: 证券类型，参见： :py:data:`constant`
    :return: 对应的证券类型信息，如果不存在，则返回Null<StockTypeInfo>()
    :rtype: StockTypeInfo)")

      .def(
        "get_stock_type_info_list",
        [](const StockManager& self) {
            auto stk_type_info_list = self.getStockTypeInfoList();
            size_t total = stk_type_info_list.size();
            if (total == 0) {
                return py::module_::import("pandas").attr("DataFrame")();
            }

            // 创建数组
            py::array_t<uint32_t> type_arr(total);
            py::array_t<double> tick_arr(total);
            py::array_t<double> tickvalue_arr(total);
            py::array_t<double> unit_arr(total);
            py::array_t<int> precision_arr(total);
            py::array_t<double> minTradeNumber_arr(total);
            py::array_t<double> maxTradeNumber_arr(total);

            // 获取缓冲区并填充数据
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

    获取所有证券类型详细信息

    :return: 所有证券类型详细信息
    :rtype: DataFrame)")

      .def("get_stock", &StockManager::getStock, R"(get_stock(self, querystr)

    根据"市场简称证券代码"获取对应的证券实例

    :param str querystr: 格式：“市场简称证券代码”，如"sh000001"
    :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
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
        
    获取证券列表

    :param func filter: 输入参数为 stock, 返回 True | False 的过滤函数)")

      .def("get_category_list", &StockManager::getAllCategory, R"(get_category_list(self)

    获取所有板块分类

    :return: 所有板块分类
    :rtype: StringList)")

      .def("get_block", &StockManager::getBlock, R"(get_block(self, category, name)

    获取预定义的板块

    :param str category: 板块分类
    :param str name: 板块名称
    :return: 板块，如找不到返回空Block
    :rtype: Block)")

      .def("add_block", &StockManager::addBlock, R"(add_block(self, block)
      
    将独立的板块加入到数据库中， 板块通过 category+name 区分， 数据库中相同板块将被覆盖。注意，如果板块发生变化，需要调用 save_block 重新保存。
      
    :param Block block: 板块实例)")

      .def("save_block", &StockManager::saveBlock, R"(save_block(self, block)
      
    保存发生变化后的板块保存至数据库

    :param Block block: 板块实例)")

      .def("remove_block",
           py::overload_cast<const string&, const string&>(&StockManager::removeBlock),
           py::arg("category"), py::arg("name"))
      .def("remove_block", py::overload_cast<const Block&>(&StockManager::removeBlock),
           py::arg("block"), R"(remove_block(self, block)
           
    从数据库中删除板块
    
    :param Block block: 板块实例)")

      .def("get_block_list", py::overload_cast<>(&StockManager::getBlockList))
      .def("get_block_list", py::overload_cast<const string&>(&StockManager::getBlockList),
           R"(get_block_list(self[, category])

    获取指定分类的板块列表

    :param str category: 板块分类
    :return: 板块列表
    :rtype: BlockList)")

      .def("get_block_list_by_index_stock", &StockManager::getBlockListByIndexStock,
           py::arg("index_stk"),
           R"(get_block_list_by_index_stock(self, index_stk)

    获取指定指数的板块列表

    :param Stock index_stk: 指数
    :return: 板块列表
    :rtype: BlockList)")

      .def("get_trading_calendar", &StockManager::getTradingCalendar, py::arg("query"),
           py::arg("market") = "SH",
           R"(get_trading_calendar(self, query[, market='SH'])

    获取指定市场的交易日日历

    :param KQuery query: Query查询条件
    :param str market: 市场简称
    :return: 日期列表
    :rtype: DatetimeList)")

      .def("add_temp_csv_stock", &StockManager::addTempCsvStock, py::arg("code"),
           py::arg("day_filename"), py::arg("min_filename"), py::arg("tick") = 0.01,
           py::arg("tick_value") = 0.01, py::arg("precision") = 2, py::arg("min_trade_num") = 1,
           py::arg("max_trade_num") = 1000000,
           R"(add_temp_csv_stock(code, day_filename, min_filename[, tick=0.01, tick_value=0.01,
        precision=2, min_trade_num = 1, max_trade_num=1000000])

    从CSV文件（K线数据）增加临时的Stock，可用于只有CSV格式的K线数据时，进行临时测试。

    添加的 stock 对应的 market 为 "TMP", 如需通过 sm 获取，需加入 tmp，如：sm['tmp0001']

    CSV文件第一行为标题，需含有
    Datetime（或Date、日期）、OPEN（或开盘价）、HIGH（或最高价）、LOW（或最低价）、CLOSE（或收盘价）、AMOUNT（或成交金额）、VOLUME（或VOL、COUNT、成交量）。

    注意：请确保 csv 使用 utf8 格式存储，否则无法识别中文

    :param str code: 自行编号的证券代码，不能和已有的Stock相同，否则将返回Null<Stock>
    :param str day_filename: 日线CSV文件名
    :param str min_filename: 分钟线CSV文件名
    :param float tick: 最小跳动量，默认0.01
    :param float tick_value: 最小跳动量价值，默认0.01
    :param int precision: 价格精度，默认2
    :param int min_trade_num: 单笔最小交易量，默认1
    :param int max_trade_num: 单笔最大交易量，默认1000000
    :return: 加入的Stock
    :rtype: Stock)",
           py::keep_alive<1, 2>())

      .def("remove_temp_csv_stock", &StockManager::removeTempCsvStock,
           R"(remove_temp_csv_stock(self, code)

    移除增加的临时Stock

    :param str code: 创建时自定义的编码)")

      .def("is_holiday", &StockManager::isHoliday, R"(is_holiday(self, d)

    判断指定时间对应的日期是否为节假日(仅使用A股市场)

    :param Datetime d: 待判断的时间)")

      .def("is_trading_hours", &StockManager::isTradingHours, py::arg("d"),
           py::arg("market") = "SH", R"(is_trading_hours(self, d)

    判断指定时间对应的日期是否为交易时间

    :param Datetime d: 待判断的时间
    :param str market: 市场简称
    :return: 是否为交易时间
    :rtype: bool)")

      .def("get_history_finance_field_name", &StockManager::getHistoryFinanceFieldName,
           py::return_value_policy::copy, R"(get_history_finance_field_name(self, index)
           
    根据字段索引，获取历史财务信息相应字段名)")

      .def("get_history_finance_field_index", &StockManager::getHistoryFinanceFieldIndex,
           R"(get_history_finance_field_index(self, name)
    
    根据字段名称，获取历史财务信息相应字段索引)")

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
    获取所有历史财务信息字段及其索引)")

      .def("add_stock", &StockManager::addStock, R"(add_stock(self, stock)
      
    谨慎调用！！！仅供增加某些临时的外部 Stock
    @return True | False)")

      .def("remove_stock", &StockManager::removeStock, R"(remove_stock(self, market_code)
    
    从 sm 中移除 market_code 代表的证券，谨慎使用！！！通常用于移除临时增加的外部 Stock
    
    :param str market_code: 证券市场标识)")

      .def("save_indicator", &StockManager::saveIndicator)
      .def("load_indicator", &StockManager::loadIndicator)
      .def("save_factor_meta", &StockManager::saveFactorMeta)

      .def("__len__", &StockManager::size, "返回证券数量")
      .def("__getitem__", &StockManager::getStock, "同 get_stock")
      .def(
        "__iter__",
        [](const StockManager& sm) {
            return py::make_iterator<py::return_value_policy::reference_internal, StockMapIterator,
                                     StockMapIterator, const Stock&>(sm.begin(), sm.end());
        },
        py::keep_alive<0, 1>());
}
