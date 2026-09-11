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

    K线数据导入器（ClickHouse 存储），用于将外部数据源取得的 K线 / 分时 / 分笔数据写入 ClickHouse，
    并支持向 baseinfo 库注册新的市场与证券类型。

    实际写入由 import2ch 插件完成（VIP 插件，需有效的设备授权）。插件缺失或授权无效时，构造阶段
    仅输出错误日志，后续调用表现为：set_config / add_market / add_stock_type 返回 False，
    get_last_datetime 返回空 Datetime，各数据写入接口被直接忽略。

    数据组织方式与 hikyuu 的 ClickHouse 数据驱动一致：K线按周期写入 hku_data 库的 day_k、min_k、
    min5_k、week_k 等表，分时写入 hku_data.timeline、分笔写入 hku_data.transdata，表内以 market +
    code 列区分证券；数据表自动创建，无需预先建表。

    仅需写入基础周期：周线、月线、季线、半年线、年线由日线派生，15/30/60分钟线、2小时线由5分钟线
    派生，故 add_krecord_list 通常只配合 Query.DAY、Query.MIN、Query.MIN5 使用。

    典型用法::

        >>> im = KDataToClickHouseImporter()
        >>> if not im.set_config('127.0.0.1', 9000, 'default', 'pwd'):  # 返回 False 通常表示无有效授权
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

    设置 ClickHouse 连接信息，须在其余接口之前调用。

    :param str host: ClickHouse 主机地址（必填）
    :param int port: ClickHouse TCP 端口
    :param str usr: 登录用户名
    :param str pwd: 登录密码
    :param str baseinfo_db: baseinfo 库名，add_market、add_stock_type 写入该库；须与 hikyuu 配置中使用的 baseinfo 库一致。K线数据固定写入 hku_data 库，不受本参数影响
    :return: 配置成功返回 True；插件缺失或授权无效返回 False
    :rtype: bool)")
      .def("get_last_datetime", &KDataToClickHouseImporter::getLastDatetime,
           R"(get_last_datetime(market: str, code: str, ktype: Query.KType) -> Datetime

    获取指定市场、指定证券在指定周期下已导入数据的最后时间，常用于增量导入时确定起始位置。

    :param str market: 市场简称，如 'SH'
    :param str code: 证券代码（不含市场简称），如 '600000'
    :param Query.KType ktype: K线周期，如 Query.DAY、Query.MIN、Query.MIN5
    :return: 最后一条数据的时间；无数据、插件缺失或授权无效时返回空 Datetime（is_null 为 True）
    :rtype: Datetime)")
      .def("add_krecord_list", &KDataToClickHouseImporter::addKRecordList,
           R"(add_krecord_list(market: str, code: str, krecords: list, ktype: Query.KType) -> None

    为指定市场、指定证券追加写入 K 线数据，写入后需调用 update_index 更新索引。

    :param str market: 市场简称，如 'SH'
    :param str code: 证券代码（不含市场简称），如 '600000'
    :param list krecords: KRecord 列表，字段含义参见 KRecord（datetime、open、high、low、close、amount、volume）
    :param Query.KType ktype: K线周期，如 Query.DAY、Query.MIN、Query.MIN5
    :return: None

    注意：旧版 hku_data 表以 FixedString 存储 market / code（market 最长 2 字符、code 最长 6 字符），
    超出的市场简称或证券代码无法写入，需将表结构迁移为 String 类型。)")
      .def("add_timeline_list", &KDataToClickHouseImporter::addTimeLineList,
           R"(add_timeline_list(market: str, code: str, timeline: list) -> None

    为指定市场、指定证券追加写入分时数据（写入 hku_data.timeline 表）。

    :param str market: 市场简称，如 'SH'
    :param str code: 证券代码（不含市场简称），如 '600000'
    :param list timeline: TimeLineRecord 列表，字段含义参见 TimeLineRecord（date、price、vol）
    :return: None)")
      .def("add_trans_list", &KDataToClickHouseImporter::addTransList,
           R"(add_trans_list(market: str, code: str, translist: list) -> None

    为指定市场、指定证券追加写入分笔数据（写入 hku_data.transdata 表）。

    :param str market: 市场简称，如 'SH'
    :param str code: 证券代码（不含市场简称），如 '600000'
    :param list translist: TransRecord 列表，字段含义参见 TransRecord（date、price、vol、direct）
    :return: None)")
      .def("update_index", &KDataToClickHouseImporter::updateIndex,
           R"(update_index(market: str, code: str, ktype: Query.KType) -> None

    更新指定市场、指定证券、指定周期的数据索引（记录该证券数据的起止范围），通常在批量写入数据后调用。

    :param str market: 市场简称，如 'SH'
    :param str code: 证券代码（不含市场简称），如 '600000'
    :param Query.KType ktype: K线周期，如 Query.DAY、Query.MIN、Query.MIN5
    :return: None)")
      .def("remove", &KDataToClickHouseImporter::remove,
           R"(remove(market: str, code: str, ktype: Query.KType, start: Datetime) -> None

    删除指定市场、指定证券、指定周期中 start 及其之后的全部数据，常用于清除错误数据后重新导入。

    :param str market: 市场简称，如 'SH'
    :param str code: 证券代码（不含市场简称），如 '600000'
    :param Query.KType ktype: K线周期，如 Query.DAY、Query.MIN、Query.MIN5
    :param Datetime start: 起始时间（含），该时间及其之后的数据一并删除
    :return: None

    注意：ClickHouse 的删除为 mutation 操作，异步执行，短时间内仍可能查询到待删除的数据。)")
      .def(
        "add_market", &KDataToClickHouseImporter::addMarket, py::arg("market"), py::arg("name"),
        py::arg("description"), py::arg("index_code"), py::arg("open1") = 930,
        py::arg("close1") = 1130, py::arg("open2") = 1300, py::arg("close2") = 1500,
        R"(add_market(market: str, name: str, description: str, index_code: str, open1: int = 930, close1: int = 1130, open2: int = 1300, close2: int = 1500) -> bool

    向 baseinfo 库注册新市场，幂等操作：市场已存在时跳过写入并返回 True。

    :param str market: 市场简称（自动转为大写），如 'US'
    :param str name: 市场名称
    :param str description: 市场描述，建议注明时区，如 'NASDAQ/UTC-5'
    :param str index_code: 市场代表指数代码，get_market_stock 与交易日历依赖 {market}{index_code}
    :param int open1: 上午开盘时间，HHMM 格式，如 930
    :param int close1: 上午收盘时间，HHMM 格式，如 1130
    :param int open2: 下午开盘时间，HHMM 格式，如 1300
    :param int close2: 下午收盘时间，HHMM 格式，如 1500
    :return: 注册成功或市场已存在返回 True；授权无效或写入失败返回 False
    :rtype: bool

    注意：注册后须重启（重新执行 hikyuu_init）方生效；市场代表指数的 K 线须另行导入。)")
      .def(
        "add_stock_type", &KDataToClickHouseImporter::addStockType, py::arg("type_id"),
        py::arg("description"), py::arg("precision") = 2, py::arg("tick") = 0.01,
        py::arg("tick_value") = 0.01, py::arg("min_trade") = 1.0, py::arg("max_trade") = 1000000.0,
        R"(add_stock_type(type_id: int, description: str, precision: int = 2, tick: float = 0.01, tick_value: float = 0.01, min_trade: float = 1.0, max_trade: float = 1000000.0) -> bool

    向 baseinfo 库注册证券类型，幂等操作：type_id 已存在时跳过写入并返回 True。

    :param int type_id: 类型编号，约定 id 与 type 一致，仅允许 10（复用 CRYPTO）或大于等于 12 的自定义编号
    :param str description: 类型描述
    :param int precision: 价格精度（小数位数）
    :param float tick: 最小跳动量
    :param float tick_value: 每个 tick 的价值
    :param float min_trade: 每笔最小交易量
    :param float max_trade: 每笔最大交易量
    :return: 注册成功或类型已存在返回 True；授权无效、type_id 非法或写入失败返回 False
    :rtype: bool

    注意：注册后须重启（重新执行 hikyuu_init）方生效。
)");
}
