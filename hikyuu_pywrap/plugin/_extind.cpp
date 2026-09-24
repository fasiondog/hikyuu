/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#include "hikyuu/plugin/extind.h"
#include "../pybind_utils.h"

#define PY_AGG_IND_DEFINE(agg_func, doc)                                                   \
    m.def(#agg_func,                                                                       \
          py::overload_cast<const Indicator&, const KQuery::KType&, bool, int>(&agg_func), \
          py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,    \
          py::arg("unit") = 1, doc);

#if HKU_OS_LINUX
class __attribute__((visibility("hidden"))) PyAggFunc {
#else
class PyAggFunc {
#endif
public:
    PyAggFunc() = default;
    explicit PyAggFunc(py::object func) : m_func(func) {}

    Indicator::value_t operator()(const DatetimeList& src_ds, const Indicator::value_t* src,
                                  size_t group_start, size_t group_last) const {
        py::gil_scoped_acquire gil;
        size_t total = group_last + 1 - group_start;
        DatetimeList ds(total);
        std::copy(src_ds.begin() + group_start, src_ds.begin() + group_last + 1, ds.begin());

        std::vector<size_t> shape = {total};
        py::array_t<Indicator::value_t> arr(shape, src + group_start);
        py::object ret = m_func(ds, arr);
        return ret.cast<Indicator::value_t>();
    }

private:
    py::object m_func;
};

#define PY_GROUP_IND_DEFINE(group_func, doc)                                           \
    m.def(#group_func,                                                                 \
          py::overload_cast<const Indicator&, const KQuery::KType&, int>(&group_func), \
          py::arg("ind"), py::arg("ktype") = KQuery::DAY, py::arg("unit") = 1, doc);

#if HKU_OS_LINUX
class __attribute__((visibility("default"))) PyGroupFunc {
#else
class PyGroupFunc {
#endif
public:
    PyGroupFunc() = default;
    explicit PyGroupFunc(py::object func) : m_func(func) {}

    void operator()(Indicator::value_t* dst, const DatetimeList& src_ds,
                    const Indicator::value_t* src, size_t group_start, size_t group_last) const {
        py::gil_scoped_acquire gil;

        size_t total = group_last + 1 - group_start;
        DatetimeList ds(total);
        std::copy(src_ds.begin() + group_start, src_ds.begin() + group_last + 1, ds.begin());

        std::vector<size_t> shape = {total};
        py::array_t<Indicator::value_t> arr(shape, src + group_start);
        py::array_t<Indicator::value_t> ret = m_func(ds, arr);
        auto dim = ret.ndim();
        HKU_CHECK(dim == 1,
                  "The return value of a Python function must be a one-dimensional array!");
        HKU_CHECK(ret.shape()[0] == shape[0],
                  "The length of the return value of the Python function is inconsistent with "
                  "the input!");
        const Indicator::value_t* data = ret.data();  // The data pointer (accessing the underlying memory directly)
        memcpy(dst + group_start, data, total * sizeof(Indicator::value_t));
    }

private:
    py::object m_func;
};

void export_extend_Indicator(py::module& m) {
    m.def("FIXED_START_INDEX", py::overload_cast<int>(FIXED_START_INDEX),
          py::arg("start_index") = 0);
    m.def("FIXED_START_INDEX", py::overload_cast<const Indicator&, int>(FIXED_START_INDEX),
          py::arg("ind"), py::arg("start_index") = 0,
          R"(FIXED_START_INDEX([ind, start_index=0])

    Fix the start index of the query range used when calculating the indicator.

    For some indicators that change with the time start point (such as AD), fixing the start index ensures the calculation from the first data of the stock.

    :param Indicator ind: the input indicator
    :param int start_index: the start index position, defaulting to 0; when it is negative, it means starting the calculation from the time point moved forward by index from the current latest
    :return: the indicator data
    :rtype: Indicator)");

    m.def("FIXED_START_DATE", py::overload_cast<const Datetime&>(FIXED_START_DATE),
          py::arg("start_date") = Datetime::min());
    m.def("FIXED_START_DATE",
          py::overload_cast<const Indicator&, const Datetime&>(FIXED_START_DATE), py::arg("ind"),
          py::arg("start_date") = Datetime::min(),
          R"(FIXED_START_DATE([ind, start_date=Datetime.min()])

    Fix the start date of the query range used when calculating the indicator.

    For some indicators that change with the time start point (such as AD), fixing the start date ensures the calculation from the specified date.

    :param Indicator ind: the input indicator
    :param Datetime start_date: the start date, defaulting to Datetime.min()
    :return: the indicator data
    :rtype: Indicator)");

    m.def("WITHKTYPE", py::overload_cast<const KQuery::KType&, bool>(WITHKTYPE), py::arg("ktype"),
          py::arg("fill_null") = false);
    m.def("WITHKTYPE", py::overload_cast<const Indicator&, const KQuery::KType&, bool>(WITHKTYPE),
          py::arg("ind"), py::arg("ktype"), py::arg("fill_null") = false,
          R"(WITHKTYPE([ind, ktype, fill_null])

    Convert the indicator data to the specified period

    :param Indicator ind: the indicator data
    :param KQuery.KType ktype: the indicator period
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHDAY", py::overload_cast<bool>(WITHDAY), py::arg("fill_null") = false);
    m.def("WITHDAY", py::overload_cast<const Indicator&, bool>(WITHDAY), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHDAY([ind, fill_null])

    Convert the indicator data to the daily line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHWEEK", py::overload_cast<bool>(WITHWEEK), py::arg("fill_null") = false);
    m.def("WITHWEEK", py::overload_cast<const Indicator&, bool>(WITHWEEK), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHWEEK([ind, fill_null])

    Convert the indicator data to the weekly line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHMONTH", py::overload_cast<bool>(WITHMONTH), py::arg("fill_null") = false);
    m.def("WITHMONTH", py::overload_cast<const Indicator&, bool>(WITHMONTH), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMONTH([ind, fill_null])

    Convert the indicator data to the monthly line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHQUARTER", py::overload_cast<bool>(WITHQUARTER), py::arg("fill_null") = false);
    m.def("WITHQUARTER", py::overload_cast<const Indicator&, bool>(WITHQUARTER), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHQUARTER([ind, fill_null])

    Convert the indicator data to the quarterly line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHHALFYEAR", py::overload_cast<bool>(WITHHALFYEAR), py::arg("fill_null") = false);
    m.def("WITHHALFYEAR", py::overload_cast<const Indicator&, bool>(WITHHALFYEAR), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHALFYEAR([ind, fill_null])

    Convert the indicator data to the half-year line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHYEAR", py::overload_cast<bool>(WITHYEAR), py::arg("fill_null") = false);
    m.def("WITHYEAR", py::overload_cast<const Indicator&, bool>(WITHYEAR), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHYEAR([ind, fill_null])

    Convert the indicator data to the yearly line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHMIN", py::overload_cast<bool>(WITHMIN), py::arg("fill_null") = false);
    m.def("WITHMIN", py::overload_cast<const Indicator&, bool>(WITHMIN), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN([ind, fill_null])

    Convert the indicator data to the minute line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHMIN5", py::overload_cast<bool>(WITHMIN5), py::arg("fill_null") = false);
    m.def("WITHMIN5", py::overload_cast<const Indicator&, bool>(WITHMIN5), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN5([ind, fill_null])

    Convert the indicator data to the 5-minute line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHMIN15", py::overload_cast<bool>(WITHMIN15), py::arg("fill_null") = false);
    m.def("WITHMIN15", py::overload_cast<const Indicator&, bool>(WITHMIN15), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN15([ind, fill_null])

    Convert the indicator data to the 15-minute line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHMIN30", py::overload_cast<bool>(WITHMIN30), py::arg("fill_null") = false);
    m.def("WITHMIN30", py::overload_cast<const Indicator&, bool>(WITHMIN30), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN30([ind, fill_null])

    Convert the indicator data to the 30-minute line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHMIN60", py::overload_cast<bool>(WITHMIN60), py::arg("fill_null") = false);
    m.def("WITHMIN60", py::overload_cast<const Indicator&, bool>(WITHMIN60), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHMIN60([ind, fill_null])

    Convert the indicator data to the 60-minute line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHHOUR", py::overload_cast<bool>(WITHHOUR), py::arg("fill_null") = false);
    m.def("WITHHOUR", py::overload_cast<const Indicator&, bool>(WITHHOUR), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHOUR([ind, fill_null])

    Convert the indicator data to the 60-minute line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHHOUR2", py::overload_cast<bool>(WITHHOUR2), py::arg("fill_null") = false);
    m.def("WITHHOUR2", py::overload_cast<const Indicator&, bool>(WITHHOUR2), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHOUR2([ind, fill_null])

    Convert the indicator data to the 2-hour line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def("WITHHOUR4", py::overload_cast<bool>(WITHHOUR4), py::arg("fill_null") = false);
    m.def("WITHHOUR4", py::overload_cast<const Indicator&, bool>(WITHHOUR4), py::arg("ind"),
          py::arg("fill_null") = false,
          R"(WITHHOUR4([ind, fill_null])

    Convert the indicator data to the 4-hour line

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator)");

    m.def(
      "RANK",
      [](const py::object& stks, int mode, bool fill_null, const string& market) {
          Block blk;
          blk.add(get_stock_list_from_python(stks));
          return RANK(blk, mode, fill_null, market);
      },
      py::arg("stks"), py::arg("mode") = 0, py::arg("fill_null") = true, py::arg("market") = "SH");
    m.def(
      "RANK",
      [](const py::sequence stks, const Indicator& ref_ind, int mode, bool fill_null,
         const string& market) {
          Block blk;
          blk.add(get_stock_list_from_python(stks));
          return RANK(blk, ref_ind, mode, fill_null, market);
      },
      py::arg("stks"), py::arg("ref_ind"), py::arg("mode") = 0, py::arg("fill_null") = true,
      py::arg("market") = "SH",
      R"(RANK(stks, ref_ind, mode = 0, fill_null = true, market = 'SH')
      
    Calculate the ranking of the indicator value in the specified block

    :param stks: the specified security list or Block
    :param ref_ind: the reference indicator
    :param mode: the sorting way: 0-descending ranking (the highest indicator value is ranked 1), 1-ascending ranking (the greater the indicator value, the greater the ranking value), 2-the descending ranking percentage, 3-the ascending ranking percentage, 4-the descending ranking percentage (0~1), 5-the ascending ranking percentage (0~1)
    :param fill_null: whether to fill the missing values
    :param market: the market the block belongs to
    :return: the ranking of the indicator value in the specified block
    :rtype: Indicator)");

    PY_AGG_IND_DEFINE(AGG_MEAN, "The aggregation function: the average value, refer to the AGG_STD help")
    PY_AGG_IND_DEFINE(AGG_COUNT, "The aggregation function: the non-empty value counting, refer to the AGG_STD help")
    PY_AGG_IND_DEFINE(AGG_SUM, "The aggregation function: the total sum, refer to the AGG_STD help")
    PY_AGG_IND_DEFINE(AGG_MAX, "The aggregation function: the maximum value, refer to the AGG_STD help")
    PY_AGG_IND_DEFINE(AGG_MIN, "The aggregation function: the minimum value, refer to the AGG_STD help")
    PY_AGG_IND_DEFINE(AGG_MAD, "The aggregation function: the average absolute deviation, refer to the AGG_STD help")
    PY_AGG_IND_DEFINE(AGG_MEDIAN, "The aggregation function: the median, refer to the AGG_STD help")
    PY_AGG_IND_DEFINE(AGG_PROD, "The aggregation function: the product, refer to the AGG_STD help")

    m.def("AGG_SAMPLE",
          py::overload_cast<const Indicator&, const string&, const KQuery::KType&, bool, int>(
            &AGG_SAMPLE),
          py::arg("ind"), py::arg("time") = "9:35", py::arg("ktype") = KQuery::MIN,
          py::arg("fill_null") = false, py::arg("unit") = 1,
          R"(AGG_SAMPLE(ind[, time="9:35", ktype=Query.MIN, fill_null=False, unit=1])

    The time sampling aggregation indicator, sampling the indicator data at the specified time points.

    If the exactly matching time cannot be found, the valid data closest before the target time is selected.

    :param Indicator ind: the input indicator
    :param str time: the specified sampling time, in the format HH:MM, defaulting to "9:35"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator)");

    m.def(
      "AGG_SAMPLE_MAX",
      py::overload_cast<const Indicator&, const string&, const string&, const KQuery::KType&, bool,
                        int>(&AGG_SAMPLE_MAX),
      py::arg("ind"), py::arg("start_time") = "9:30", py::arg("last_time") = "10:00",
      py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false, py::arg("unit") = 1,
      R"(AGG_SAMPLE_MAX(ind[, start_time="9:30", last_time="10:00", ktype=Query.MIN, fill_null=False, unit=1])

    The maximum in the time period aggregation indicator, counting the maximum of the indicator data within the specified time period [start_time, last_time].

    It includes the start_time and the last_time themselves.

    :param Indicator ind: the input indicator
    :param str start_time: the start time of the time period, in the format HH:MM, defaulting to "9:30"
    :param str last_time: the end time of the time period, in the format HH:MM, defaulting to "10:00"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator)");

    m.def(
      "AGG_SAMPLE_MIN",
      py::overload_cast<const Indicator&, const string&, const string&, const KQuery::KType&, bool,
                        int>(&AGG_SAMPLE_MIN),
      py::arg("ind"), py::arg("start_time") = "9:30", py::arg("last_time") = "10:00",
      py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false, py::arg("unit") = 1,
      R"(AGG_SAMPLE_MIN(ind[, start_time="9:30", last_time="10:00", ktype=Query.MIN, fill_null=False, unit=1])

    The minimum in the time period aggregation indicator, counting the minimum of the indicator data within the specified time period [start_time, last_time].

    It includes the start_time and the last_time themselves.

    :param Indicator ind: the input indicator
    :param str start_time: the start time of the time period, in the format HH:MM, defaulting to "9:30"
    :param str last_time: the end time of the time period, in the format HH:MM, defaulting to "10:00"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator)");

    m.def(
      "AGG_SAMPLE_MEAN",
      py::overload_cast<const Indicator&, const string&, const string&, const KQuery::KType&, bool,
                        int>(&AGG_SAMPLE_MEAN),
      py::arg("ind"), py::arg("start_time") = "9:30", py::arg("last_time") = "10:00",
      py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false, py::arg("unit") = 1,
      R"(AGG_SAMPLE_MEAN(ind[, start_time="9:30", last_time="10:00", ktype=Query.MIN, fill_null=False, unit=1])

    The average in the time period aggregation indicator, counting the average of the indicator data within the specified time period [start_time, last_time].

    It includes the start_time and the last_time themselves.

    :param Indicator ind: the input indicator
    :param str start_time: the start time of the time period, in the format HH:MM, defaulting to "9:30"
    :param str last_time: the end time of the time period, in the format HH:MM, defaulting to "10:00"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator)");

    m.def("AGG_STD",
          py::overload_cast<const Indicator&, const KQuery::KType&, bool, int, int>(&AGG_STD),
          py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,
          py::arg("unit") = 1, py::arg("ddof") = 1,
          R"(AGG_STD(ind[, ktype=Query.MIN, fill_null=False, unit=1, ddof=1])

    Aggregate the standard deviation of the other K-line periods, e.g. calculating the standard deviation of the minute-line close prices aggregated when calculating the daily line

        >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
        >>> ind = AGG_STD(CLOSE(), ktype=Query.MIN, fill_null=False, unit=1, ddof=1)
        >>> ind(k)

    :param Indicator ind: the indicator data
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the rolling aggregation grouping unit; e.g. when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :param int ddof: the degree of freedom (1: the sample standard deviation, 0: the population standard deviation)
    :return: the indicator data
    :rtype: Indicator)");

    m.def("AGG_VAR",
          py::overload_cast<const Indicator&, const KQuery::KType&, bool, int, int>(&AGG_VAR),
          py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,
          py::arg("unit") = 1, py::arg("ddof") = 1,
          R"(AGG_VAR(ind[, ktype=Query.MIN, fill_null=False, unit=1, ddof=1])

    Aggregate the variance of the other K-line periods, e.g. calculating the variance of the minute-line close prices aggregated when calculating the daily line

        >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
        >>> ind = AGG_VAR(CLOSE(), ktype=Query.MIN, fill_null=False, unit=1, ddof=1)
        >>> ind(k)

    :param Indicator ind: the indicator to calculate
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the grouping unit of the context K-lines; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :param int ddof: the degree of freedom (1: the sample standard deviation, 0: the population standard deviation)
    :return: the indicator data
    :rtype: Indicator)");

    m.def(
      "AGG_QUANTILE",
      py::overload_cast<const Indicator&, const KQuery::KType&, bool, int, double>(&AGG_QUANTILE),
      py::arg("ind"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,
      py::arg("unit") = 1, py::arg("quantile") = 0.75,
      R"(AGG_QUANTILE(ind[, ktype=Query.MIN, fill_null=False, unit=1, quantile=0.75])

    Aggregate the quantile of the other K-line periods, refer to the AGG_STD help

    :param Indicator ind: the indicator to calculate
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the grouping unit of the context K-lines; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :param float quantile: the quantile, between (0, 1)
    :return: the indicator data
    :rtype: Indicator)");

    m.def("AGG_VWAP", py::overload_cast<const KData&, const KQuery::KType&, bool, int>(&AGG_VWAP),
          py::arg("k"), py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false,
          py::arg("unit") = 1);
    m.def("AGG_VWAP", py::overload_cast<const KQuery::KType&, bool, int>(&AGG_VWAP),
          py::arg("ktype") = KQuery::MIN, py::arg("fill_null") = false, py::arg("unit") = 1,
          R"(AGG_VWAP([ktype=Query.MIN, fill_null=False, unit=1])

    Aggregate the volume weighted average price of the other K-line periods (Volume Weighted Average Price)
    
    VWAP is the volume-weighted average price, calculated by multiplying the volume of each minute (or unit time) by the trading price of that minute,
    then summing all the products, and finally dividing by the total volume.

    Usage: AGG_VWAP([ktype, fill_null, unit])
    
    For example: AGG_VWAP(Query.MIN) calculates the VWAP value of the minute lines

    :param KQuery.KType ktype: the aggregated K-line period type
    :param bool fill_null: whether to fill the missing values, defaulting to False
    :param int unit: the aggregation period unit, defaulting to 1. The grouping unit of the context K-lines; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days
    :return: the indicator data
    :rtype: Indicator)");

    m.def(
      "AGG_FUNC",
      [](const Indicator& ind, py::object agg_func, const KQuery::KType& ktype, bool fill_null,
         int unit) {
          HKU_CHECK(!agg_func.is_none(), "agg_func is None!");
          HKU_CHECK(py::hasattr(agg_func, "__call__"), "agg_func not callable!");
          HKU_CHECK(check_pyfunction_arg_num(agg_func, 2), "Number of parameters does not match!");
          PyAggFunc agg_func_obj(agg_func.attr("__call__"));
          Indicator ret = AGG_FUNC(ind, agg_func_obj, ktype, fill_null, unit);
          return ret;
      },
      py::arg("ind"), py::arg("agg_func"), py::arg("ktype") = KQuery::MIN,
      py::arg("fill_null") = false, py::arg("unit") = 1,
      R"(AGG_FUNC(ind, agg_func[, ktype=Query.MIN, fill_null=False, unit=1]
      
    Aggregate the indicators of the other K-line periods with a custom function.
    
    Example, calculating the sum of the minute-line close prices aggregated when calculating the daily line:

      >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
      >>> ind = AGG_FUNC(CLOSE(), lambda ds, x: np.sum(x))
      >>> ind(k)

    :param Indicator ind: the indicator to calculate
    :param callable agg_func: the custom aggregation function, whose input parameters are arg1: a datetime list, arg2: a numpy array, and which returns the aggregation result for the list; note that it is a single value
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the grouping unit of the context K-lines; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :return: the aggregation result
    :rtype: Indicator)");

    PY_GROUP_IND_DEFINE(GROUP_COUNT, "The group cumulative counting")
    PY_GROUP_IND_DEFINE(GROUP_SUM, "The group cumulative sum")
    PY_GROUP_IND_DEFINE(GROUP_PROD, "The group cumulative product")
    PY_GROUP_IND_DEFINE(GROUP_MEAN, "The group cumulative average")
    PY_GROUP_IND_DEFINE(GROUP_MAX, "The group cumulative maximum")
    PY_GROUP_IND_DEFINE(GROUP_MIN, "The group cumulative minimum")

    m.def(
      "GROUP_FUNC",
      [](const Indicator& ind, py::object group_func, const KQuery::KType& ktype, int unit) {
          HKU_CHECK(!group_func.is_none(), "group_func is None!");
          HKU_CHECK(py::hasattr(group_func, "__call__"), "agg_func not callable!");
          HKU_CHECK(check_pyfunction_arg_num(group_func, 2),
                    "Number of parameters does not match!");
          PyGroupFunc func_obj(group_func.attr("__call__"));
          Indicator ret = GROUP_FUNC(ind, func_obj, ktype, unit);
          return ret;
      },
      py::arg("ind"), py::arg("group_func"), py::arg("ktype") = KQuery::DAY, py::arg("unit") = 1,
      R"(GROUP_FUNC(ind, group_func[, ktype=Query.DAY,  unit=1])
      
    The custom group cumulative calculation indicator.
    
    Example, calculating the sum of the minute-line close prices aggregated when calculating the daily line:

      >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
      >>> ind = GROUP_FUNC(CLOSE(), lambda dates, data: data/2.0)
      >>> ind(k)

    :param Indicator ind: the indicator to calculate
    :param callable group_func: the custom group cumulative function, whose input parameters are arg1: a datetime list, arg2: a numpy array, and which returns the cumulative calculation result with the same length as the input; the type must also be np.array
    :param KQuery.KType ktype: the grouped K-line period
    :param int unit: the grouping period unit (the grouping K-line period unit; when using the daily line to calculate the minute-lines, unit=2 means the minute-lines accumulated by 2 days)
    :rtype: Indicator)");
}