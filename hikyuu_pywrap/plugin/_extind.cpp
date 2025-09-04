/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#include "hikyuu/plugin/extind.h"
#include "../pybind_utils.h"

#define PY_AGG_FUNC_DEFINE(agg_func, doc)                                                  \
    m.def(#agg_func,                                                                       \
          py::overload_cast<const Indicator&, const KQuery::KType&, bool, int>(&agg_func), \
          py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,    \
          py::arg("unit") = 1, doc);

void export_extend_Indicator(py::module& m) {
    m.def("WITHKTYPE", py::overload_cast<const KQuery::KType&, bool>(WITHKTYPE), py::arg("ktype"),
          py::arg("fill_null") = false);
    m.def("WITHKTYPE", py::overload_cast<const Indicator&, const KQuery::KType&, bool>(WITHKTYPE),
          py::arg("ind"), py::arg("ktype"), py::arg("fill_null") = false,
          R"(WITHKTYPE([ind, ktype, fill_null])

    将指标数据转换到指定周期

    :param Indicator ind: 指标数据
    :param KQuery.KType ktype: 指标周期
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHDAY", py::overload_cast<bool>(WITHDAY), py::arg("fill_null") = false);
    m.def("WITHDAY", py::overload_cast<const Indicator&, bool>(WITHDAY), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHDAY([ind, fill_null])

    将指标数据转换到日线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHWEEK", py::overload_cast<bool>(WITHWEEK), py::arg("fill_null") = false);
    m.def("WITHWEEK", py::overload_cast<const Indicator&, bool>(WITHWEEK), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHWEEK([ind, fill_null])

    将指标数据转换到周线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHMONTH", py::overload_cast<bool>(WITHMONTH), py::arg("fill_null") = false);
    m.def("WITHMONTH", py::overload_cast<const Indicator&, bool>(WITHMONTH), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMONTH([ind, fill_null])

    将指标数据转换到月线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHQUARTER", py::overload_cast<bool>(WITHQUARTER), py::arg("fill_null") = false);
    m.def("WITHQUARTER", py::overload_cast<const Indicator&, bool>(WITHQUARTER), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHQUARTER([ind, fill_null])

    将指标数据转换到季线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHHALFYEAR", py::overload_cast<bool>(WITHHALFYEAR), py::arg("fill_null") = false);
    m.def("WITHHALFYEAR", py::overload_cast<const Indicator&, bool>(WITHHALFYEAR), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHALFYEAR([ind, fill_null])

    将指标数据转换到半年线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHYEAR", py::overload_cast<bool>(WITHYEAR), py::arg("fill_null") = false);
    m.def("WITHYEAR", py::overload_cast<const Indicator&, bool>(WITHYEAR), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHYEAR([ind, fill_null])

    将指标数据转换到年线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHMIN", py::overload_cast<bool>(WITHMIN), py::arg("fill_null") = false);
    m.def("WITHMIN", py::overload_cast<const Indicator&, bool>(WITHMIN), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN([ind, fill_null])

    将指标数据转换到分钟线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHMIN5", py::overload_cast<bool>(WITHMIN5), py::arg("fill_null") = false);
    m.def("WITHMIN5", py::overload_cast<const Indicator&, bool>(WITHMIN5), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN5([ind, fill_null])

    将指标数据转换到5分钟线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHMIN15", py::overload_cast<bool>(WITHMIN15), py::arg("fill_null") = false);
    m.def("WITHMIN15", py::overload_cast<const Indicator&, bool>(WITHMIN15), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN15([ind, fill_null])

    将指标数据转换到15分钟线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHMIN30", py::overload_cast<bool>(WITHMIN30), py::arg("fill_null") = false);
    m.def("WITHMIN30", py::overload_cast<const Indicator&, bool>(WITHMIN30), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN30([ind, fill_null])

    将指标数据转换到30分钟线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHMIN60", py::overload_cast<bool>(WITHMIN60), py::arg("fill_null") = false);
    m.def("WITHMIN60", py::overload_cast<const Indicator&, bool>(WITHMIN60), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN60([ind, fill_null])

    将指标数据转换到60分钟线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHHOUR", py::overload_cast<bool>(WITHHOUR), py::arg("fill_null") = false);
    m.def("WITHHOUR", py::overload_cast<const Indicator&, bool>(WITHHOUR), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHOUR([ind, fill_null])

    将指标数据转换到60分钟线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHHOUR2", py::overload_cast<bool>(WITHHOUR2), py::arg("fill_null") = false);
    m.def("WITHHOUR2", py::overload_cast<const Indicator&, bool>(WITHHOUR2), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHOUR2([ind, fill_null])

    将指标数据转换到2小时线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def("WITHHOUR4", py::overload_cast<bool>(WITHHOUR4), py::arg("fill_null") = false);
    m.def("WITHHOUR4", py::overload_cast<const Indicator&, bool>(WITHHOUR4), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHOUR4([ind, fill_null])

    将指标数据转换到4小时线

    :param Indicator ind: 指标数据
    :param bool fill_null: 是否填充空值
    :rtype: Indicator)");

    m.def(
      "RANK",
      [](const py::sequence stks, int mode, bool fill_null, const string& market) {
          Block blk;
          if (py::isinstance<Block>(stks)) {
              blk = py::cast<Block>(stks);
          } else if (py::isinstance<StockManager>(stks)) {
              auto& sm = py::cast<StockManager&>(stks);
              blk.add(sm.getStockList());
          } else {
              StockList sl = python_list_to_vector<Stock>(stks);
              blk.add(sl);
          }
          return RANK(blk, mode, fill_null, market);
      },
      py::arg("stks"), py::arg("mode") = 0, py::arg("fill_null") = true, py::arg("market") = "SH");
    m.def(
      "RANK",
      [](const py::sequence stks, const Indicator& ref_ind, int mode, bool fill_null,
         const string& market) {
          Block blk;
          if (py::isinstance<Block>(stks)) {
              blk = py::cast<Block>(stks);
          } else if (py::isinstance<StockManager>(stks)) {
              auto& sm = py::cast<StockManager&>(stks);
              blk.add(sm.getStockList());
          } else {
              StockList sl = python_list_to_vector<Stock>(stks);
              blk.add(sl);
          }
          return RANK(blk, ref_ind, mode, fill_null, market);
      },
      py::arg("stks"), py::arg("ref_ind"), py::arg("mode") = 0, py::arg("fill_null") = true,
      py::arg("market") = "SH", R"(RANK(stks, ref_ind, mode = 0, fill_null = true, market = 'SH')
      
    计算指标值在指定板块中的排名

    :param stks: 指定证券列表 或 Block
    :param ref_ind: 参考指标
    :param mode: 排序方式: 0-降序排名(指标值最高值排名为1), 1-升序排名(指标值越大排名值越大), 2-降序排名百分比, 3-升序排名百分比
    :param fill_null: 是否填充缺失值
    :param market: 板块所属市场
    :return: 指标值在指定板块中的排名
    :rtype: Indicator)");

    PY_AGG_FUNC_DEFINE(AGG_MEAN, "聚合函数: 平均值, 可参考 AGG_STD 帮助")
    PY_AGG_FUNC_DEFINE(AGG_COUNT, "聚合函数: 非空值计数, 可参考 AGG_STD 帮助")
    PY_AGG_FUNC_DEFINE(AGG_SUM, "聚合函数: 总和, 可参考 AGG_STD 帮助")
    PY_AGG_FUNC_DEFINE(AGG_MAX, "聚合函数: 最大值, 可参考 AGG_STD 帮助")
    PY_AGG_FUNC_DEFINE(AGG_MIN, "聚合函数: 最小值, 可参考 AGG_STD 帮助")
    PY_AGG_FUNC_DEFINE(AGG_MAD, "聚合函数: 平均绝对偏差, 可参考 AGG_STD 帮助")
    PY_AGG_FUNC_DEFINE(AGG_MEDIAN, "聚合函数: 中位数, 可参考 AGG_STD 帮助")
    PY_AGG_FUNC_DEFINE(AGG_PROD, "聚合函数: 乘积, 可参考 AGG_STD 帮助")

    m.def("AGG_STD",
          py::overload_cast<const Indicator&, const KQuery::KType&, bool, int, int>(&AGG_STD),
          py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,
          py::arg("unit") = 1, py::arg("ddof") = 1,
          R"(AGG_STD(ind[, ktype=Query.MIN, fill_null=False, unit=1, ddof=1])

    聚合其他K线周期的标准差, 如计算日线时聚合分钟线收盘价的标准差

        >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
        >>> ind = AGG_STD(CLOSE(), ktype=Query.MIN, fill_null=False, unit=1, ddof=1)
        >>> ind(k)

    :param Indicator ind: 指标数据
    :param KQuery.KType ktype: 聚合的K线周期
    :param bool fill_null: 是否填充缺失值
    :param int unit: 聚合周期单位 (滚动聚合分组单位, 如使用日线计算分钟线聚合时, unit=2代表聚合2天的分钟线)
    :param int ddof: 自由度(1: 样本标准差, 0: 总体标准差)
    :return: 指标数据
    :rtype: Indicator)");

    m.def("AGG_VAR",
          py::overload_cast<const Indicator&, const KQuery::KType&, bool, int, int>(&AGG_VAR),
          py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,
          py::arg("unit") = 1, py::arg("ddof") = 1,
          R"(AGG_VAR(ind[, ktype=Query.MIN, fill_null=False, unit=1, ddof=1])

    聚合其他K线周期的方差, 如计算日线时聚合分钟线收盘价的方差

        >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
        >>> ind = AGG_VAR(CLOSE(), ktype=Query.MIN, fill_null=False, unit=1, ddof=1)
        >>> ind(k)

    :param Indicator ind: 待计算指标
    :param KQuery.KType ktype: 聚合的K线周期
    :param bool fill_null: 是否填充缺失值
    :param int unit: 聚合周期单位 (上下文K线分组单位, 使用日线计算分钟线聚合时, unit=2代表聚合2天的分钟线)
    :param int ddof: 自由度(1: 样本标准差, 0: 总体标准差)
    :return: 指标数据
    :rtype: Indicator)");

    m.def(
      "AGG_QUANTILE",
      py::overload_cast<const Indicator&, const KQuery::KType&, bool, int, double>(&AGG_QUANTILE),
      py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,
      py::arg("unit") = 1, py::arg("quantile") = 0.75,
      R"(AGG_QUANTILE(ind[, ktype=Query.MIN, fill_null=False, unit=1, quantile=0.75])

    聚合其他K线周期分位数, 可参考 AGG_STD 帮助

    :param Indicator ind: 待计算指标
    :param KQuery.KType ktype: 聚合的K线周期
    :param bool fill_null: 是否填充缺失值
    :param int unit: 聚合周期单位 (上下文K线分组单位, 使用日线计算分钟线聚合时, unit=2代表聚合2天的分钟线)
    :param float quantile: 分位数 (0, 1) 之间
    :return: 指标数据
    :rtype: Indicator)");
}