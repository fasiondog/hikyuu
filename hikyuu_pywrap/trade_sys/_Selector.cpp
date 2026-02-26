/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/selector/build_in.h>
#include <hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PySelectorBase : public SelectorBase {
    PY_CLONE(PySelectorBase, SelectorBase)

public:
    PySelectorBase() : SelectorBase() {
        m_is_python_object = true;
    }

    PySelectorBase(const string& name) : SelectorBase(name) {
        m_is_python_object = true;
    }

    PySelectorBase(const SelectorBase& base) : SelectorBase(base) {
        m_is_python_object = true;
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, SelectorBase, _reset, );
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, SelectorBase, _calculate, );
    }

    void _addSystem(const SystemPtr& sys) override {
        PYBIND11_OVERLOAD(void, SelectorBase, _addSystem, sys);
    }

    void _removeAll() override {
        PYBIND11_OVERLOAD(void, SelectorBase, _removeAll, );
    }

    SystemWeightList _getSelected(Datetime date) override {
        // PYBIND11_OVERLOAD_PURE_NAME(SystemWeightList, SelectorBase, "get_selected", getSelected,
        //                             date);
        auto self = py::cast(this);
        py::sequence py_ret = self.attr("get_selected")(date);
        auto c_ret = python_list_to_vector<SystemWeight>(py_ret);
        return c_ret;
    }

    bool isMatchAF(const AFPtr& af) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, SelectorBase, "is_match_af", isMatchAF, af);
    }

    string str() const override {
        PYBIND11_OVERRIDE_NAME(string, SelectorBase, "__str__", str, );
    }
};

#ifdef __GNUC__
#pragma GCC visibility push(hidden)
#endif
class PyOptimalSelector : public OptimalSelectorBase {
    OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    PyOptimalSelector() : OptimalSelectorBase("SE_PyOptimal") {
        m_is_python_object = false;
    }

    explicit PyOptimalSelector(const py::function& evalfunc)
    : OptimalSelectorBase("SE_PyOptimal"), m_evaluate(evalfunc) {
        m_is_python_object = false;
    }

    virtual ~PyOptimalSelector() = default;

public:
    virtual SelectorPtr _clone() override {
        return std::make_shared<PyOptimalSelector>();
    }

    virtual double evaluate(const SYSPtr& sys, const Datetime& lastDate) noexcept override {
        double ret = Null<double>();
        try {
            ret = m_evaluate(sys, lastDate).cast<double>();
        } catch (const std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR("Unknown error!");
        }
        return ret;
    }

private:
    // 目前无法序列化
    py::function m_evaluate;
};
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

SEPtr crtSEOptimal(const py::function& evalfunc) {
    return std::make_shared<PyOptimalSelector>(evalfunc);
}

void export_Selector(py::module& m) {
    py::class_<SystemWeight>(m, "SystemWeight", py::dynamic_attr(),
                             "系统权重系数结构，在资产分配时，指定对应系统的资产占比系数")
      .def(py::init<>())
      .def(py::init<const SystemPtr&, price_t>())
      .def("__str__", to_py_str<SystemWeight>)
      .def("__repr__", to_py_str<SystemWeight>)
      .def_readwrite("sys", &SystemWeight::sys, "对应的 System 实例")
      .def_readwrite("weight", &SystemWeight::weight)

        DEF_PICKLE(SystemWeight);

    m.def("systemweights_to_np", [](const SystemWeightList& swl) {
        size_t total = swl.size();
        HKU_IF_RETURN(total == 0, py::array());

        struct alignas(8) RawData {
            int32_t sys_name[20];
            int32_t code[10];
            int32_t name[20];
            double weight;
        };

        RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        for (size_t i = 0, total = swl.size(); i < total; i++) {
            const SystemWeight& sw = swl[i];
            if (sw.sys) {
                utf8_to_utf32(sw.sys->name(), data[i].sys_name, 20);
                utf8_to_utf32(sw.sys->getStock().market_code(), data[i].code, 10);
                utf8_to_utf32(sw.sys->getStock().name(), data[i].name, 20);
            } else {
                memset(data[i].sys_name, 0, 20 * sizeof(int32_t));
                memset(data[i].code, 0, 10 * sizeof(int32_t));
                memset(data[i].name, 0, 20 * sizeof(int32_t));
            }
            data[i].weight = sw.weight;
        }
        // 定义NumPy结构化数据类型
        py::dtype dtype =
          py::dtype(vector_to_python_list<string>(
                      {htr("sys_name"), htr("market_code"), htr("stock_name"), htr("weight")}),
                    vector_to_python_list<string>({"U20", "U10", "U20", "d"}),
                    vector_to_python_list<int64_t>({0, 80, 120, 200}), 208);

        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
    });

    m.def("systemweights_to_df", [](const SystemWeightList& sws) {
        size_t total = sws.size();
        if (total == 0) {
            return py::module_::import("pandas").attr("DataFrame")();
        }

        // 创建 Python 字符串对象数组
        py::list sysname_list(total);
        py::list code_list(total);
        py::list name_list(total);
        py::array_t<double> value_arr(total);

        // 获取 value 数组缓冲区
        auto value_buf = value_arr.request();
        double* value_ptr = static_cast<double*>(value_buf.ptr);

        // 填充数据
        for (size_t i = 0; i < total; i++) {
            const SystemWeight& sw = sws[i];
            if (sw.sys) {
                sysname_list[i] = py::str(sw.sys->name());
                code_list[i] = py::str(sw.sys->getStock().market_code());
                name_list[i] = py::str(sw.sys->getStock().name());
                value_ptr[i] = sw.weight;
            } else {
                sysname_list[i] = py::str("");
                code_list[i] = py::str("");
                name_list[i] = py::str("");
                value_ptr[i] = Null<double>();
            }
        }

        // 构建 DataFrame
        auto pandas = py::module_::import("pandas");
        py::dict columns;
        columns[htr("sys_name").c_str()] =
          pandas.attr("Series")(sysname_list, py::arg("dtype") = "string");
        columns[htr("market_code").c_str()] =
          pandas.attr("Series")(code_list, py::arg("dtype") = "string");
        columns[htr("stock_name").c_str()] =
          pandas.attr("Series")(name_list, py::arg("dtype") = "string");
        columns["weight"] = value_arr;

        return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
    });

    py::class_<SelectorBase, SEPtr, PySelectorBase>(
      m, "SelectorBase",
      R"(选择器策略基类，实现标的、系统策略的评估和选取算法，自定义选择器策略子类接口：

    - get_selected - 【必须】获取指定时刻选择的系统实例列表
    - _calculate - 【必须】计算接口
    - _reset - 【可选】重置私有属性
    - _clone - 【必须】克隆接口)")

      .def(py::init<>())
      .def(py::init<const SelectorBase&>())
      .def(py::init<const string&>(), R"(初始化构造函数
        
    :param str name: 名称)")

      .def("__str__", to_py_str<SelectorBase>)
      .def("__repr__", to_py_str<SelectorBase>)

      .def_property("name", py::overload_cast<>(&SelectorBase::name, py::const_),
                    py::overload_cast<const string&>(&SelectorBase::name),
                    py::return_value_policy::copy, "算法名称")
      .def_property_readonly("proto_sys_list", &SelectorBase::getProtoSystemList,
                             py::return_value_policy::copy, "原型系统列表")
      .def_property_readonly("real_sys_list", &SelectorBase::getRealSystemList,
                             py::return_value_policy::copy, "由 PF 运行时设定的实际运行系统列表")
      .def_property_readonly("scfilter", &SelectorBase::getScoresFilter, "获取 ScoresFilter")

      .def_property(
        "mf", &SelectorBase::getMF,
        [](SelectorBase& self, py::object mf) {
            py::gil_scoped_acquire gil;
            auto tmp = mf;
            self.setMF(mf.cast<MFPtr>());
            tmp.release();
        },
        "获取关联的 MF")

      .def("get_param", &SelectorBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &SelectorBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &SelectorBase::haveParam, "是否存在指定参数")

      .def("reset", &SelectorBase::reset, "复位操作")
      .def("clone", &SelectorBase::clone, "克隆操作")
      .def("remove_all", &SelectorBase::removeAll, "清除所有已加入的原型系统")

      .def("add_stock", &SelectorBase::addStock, py::arg("stock"), py::arg("sys"),
           R"(add_stock(self, stock, sys)

    加入初始标的及其对应的系统策略原型

    :param Stock stock: 加入的初始标的
    :param System sys: 系统策略原型)")

      .def(
        "add_stock_list",
        [](SelectorBase& self, py::object stk_list, const SYSPtr& sys) {
            self.addStockList(get_stock_list_from_python(stk_list), sys);
        },
        py::arg("stk_list"), py::arg("sys"),
        R"(add_stock_list(self, stk_list, sys)

    加入初始标的列表及其系统策略原型

    :param StockList stk_list: 加入的初始标的列表
    :param System sys: 系统策略原型)")

      .def("get_proto_sys_list", &SelectorBase::getProtoSystemList, py::return_value_policy::copy)
      .def("get_real_sys_list", &SelectorBase::getRealSystemList, py::return_value_policy::copy)
      .def("calculate", &SelectorBase::calculate)

      .def("_reset", &SelectorBase::_reset, "子类复位操作实现")
      .def("_calculate", &SelectorBase::_calculate, "【重载接口】子类计算接口")

      .def("is_match_af", &SelectorBase::isMatchAF, R"(is_match_af(self)

    【重载接口】判断是否和 AF 匹配

    :param AllocateFundsBase af: 资产分配算法)")

      .def("get_selected", &SelectorBase::getSelected,
           R"(get_selected(self, datetime)

    【重载接口】获取指定时刻选取的系统实例

    :param Datetime datetime: 指定时刻
    :return: 选取的系统实例列表
    :rtype: SystemList)")

      .def("add_sys", &SelectorBase::addSystem)
      .def("add_sys_list", &SelectorBase::addSystemList)

      .def(
        "set_scores_filter",
        [](SelectorBase& self, py::object filter) {
            py::gil_scoped_acquire gil;
            auto tmp = filter;
            self.setScoresFilter(filter.cast<ScoresFilterPtr>());
            tmp.release();
        },
        R"(set_scores_filter(self, filter)
           
    设置 ScoresFilter, 将替换现有的过滤器. 仅适用于 SE_MultiFactor
    
    :param ScoresFilter filter: ScoresFilter)")

      .def(
        "add_scores_filter",
        [](SelectorBase& self, py::object filter) {
            py::gil_scoped_acquire gil;
            auto tmp = filter;
            self.addScoresFilter(filter.cast<ScoresFilterPtr>());
            tmp.release();
        },
        R"(add_scores_filter(self, filter)
        
    在已有过滤基础上新增过滤, 仅适用于 SE_MultiFactor    

    :param ScoresFilter filter: 新的过滤器)")

      .def("__add__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self + other; })
      .def("__add__", [](const SelectorPtr& self, double other) { return self + other; })
      .def("__radd__", [](const SelectorPtr& self, double other) { return self + other; })

      .def("__sub__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self - other; })
      .def("__sub__", [](const SelectorPtr& self, double other) { return self - other; })
      .def("__rsub__", [](const SelectorPtr& self, double other) { return other - self; })

      .def("__mul__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self * other; })
      .def("__mul__", [](const SelectorPtr& self, double other) { return self * other; })
      .def("__rmul__", [](const SelectorPtr& self, double other) { return self * other; })

      .def("__truediv__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self / other; })
      .def("__truediv__", [](const SelectorPtr& self, double other) { return self / other; })
      .def("__rtruediv__", [](const SelectorPtr& self, double other) { return other / self; })

      .def("__and__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self & other; })
      .def("__or__", [](const SelectorPtr& self, const SelectorPtr& other) { return self | other; })

        DEF_PICKLE(SEPtr);

    m.def("SE_Fixed", [](double weight) { return SE_Fixed(weight); }, py::arg("weight") = 1.0);
    m.def(
      "SE_Fixed",
      [](const py::object& pystks, const SystemPtr& sys, double weight) {
          StockList stks = get_stock_list_from_python(pystks);
          return SE_Fixed(stks, sys, weight);
      },
      py::arg("stk_list"), py::arg("sys"), py::arg("weight") = 1.0,
      R"(SE_Fixed([stk_list, sys])

    固定选择器，即始终选择初始划定的标的及其系统策略原型

    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :param float weight: 默认权重
    :return: SE选择器实例)");

    m.def("SE_Signal", py::overload_cast<>(SE_Signal));
    m.def("SE_Signal", py::overload_cast<const StockList&, const SystemPtr&>(SE_Signal),
          R"(SE_Signal([stk_list, sys])

    信号选择器，仅依靠系统买入信号进行选中

    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例)");

    m.def("SE_MultiFactor", py::overload_cast<const MFPtr&, int>(SE_MultiFactor), py::arg("mf"),
          py::arg("topn") = 10);
    // 添加支持FactorSet和Indicator序列的重载函数
    m.def(
      "SE_MultiFactor",
      [](const py::object& input, int topn, int ic_n, int ic_rolling_n, const py::object& ref_stk,
         bool spearman, const string& mode) {
          Stock c_ref_stk = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

          // 判断输入类型
          if (py::isinstance<FactorSet>(input)) {
              // 输入是FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return SE_MultiFactor(factset, topn, ic_n, ic_rolling_n, c_ref_stk, spearman, mode);
          } else if (py::isinstance<py::sequence>(input)) {
              // 输入是序列（假设为Indicator列表）
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return SE_MultiFactor(c_inds, topn, ic_n, ic_rolling_n, c_ref_stk, spearman, mode);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("topn") = 10, py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120,
      py::arg("ref_stk") = py::none(), py::arg("spearman") = true,
      py::arg("mode") = "MF_ICIRWeight",
      R"(SE_MultiFactor

    创建基于多因子评分的选择器，支持多种创建方式

    - 直接指定 MF:
      :param MultiFactorBase mf: 直接指定的多因子合成算法
      :param int topn: 只选取时间截面中前 topn 个系统

    - 使用FactorSet:
      :param FactorSet input: 因子集合
      :param int topn: 只选取时间截面中前 topn 个系统，小于等于0时代表不限制
      :param int ic_n: 默认 IC 对应的 N 日收益率
      :param int ic_rolling_n: IC 滚动周期
      :param Stock ref_stk: 参考证券,用于日期对齐，未指定时为 sh000001
      :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称

    - 使用Indicator序列:
      :param sequense(Indicator) input: 原始因子列表
      :param int topn: 只选取时间截面中前 topn 个系统，小于等于0时代表不限制
      :param int ic_n: 默认 IC 对应的 N 日收益率
      :param int ic_rolling_n: IC 滚动周期
      :param Stock ref_stk: 参考证券,用于日期对齐，未指定时为 sh000001
      :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称

    .. code-block:: python
    
        # 使用Indicator列表（原有方式）
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor(indicators, topn=10)
        
        # 使用FactorSet（新增方式）
        factor_set = FactorSet(indicators)
        selector2 = SE_MultiFactor(factor_set, topn=10)
        
        # 直接使用MultiFactor对象
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor(mf, topn=10))");

    m.def("SE_MultiFactor2", py::overload_cast<const MFPtr&, const SCFilterPtr&>(SE_MultiFactor2),
          py::arg("mf"), py::arg("filter") = SCFilter_IgnoreNan());
    // 添加支持FactorSet和Indicator序列的重载函数
    m.def(
      "SE_MultiFactor2",
      [](const py::object& input, int ic_n, int ic_rolling_n, const py::object& ref_stk,
         bool spearman, const string& mode, const SCFilterPtr& filter) {
          Stock c_ref_stk = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

          // 判断输入类型
          if (py::isinstance<FactorSet>(input)) {
              // 输入是FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return SE_MultiFactor2(factset, ic_n, ic_rolling_n, c_ref_stk, spearman, mode,
                                     filter);
          } else if (py::isinstance<py::sequence>(input)) {
              // 输入是序列（假设为Indicator列表）
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return SE_MultiFactor2(c_inds, ic_n, ic_rolling_n, c_ref_stk, spearman, mode, filter);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120,
      py::arg("ref_stk") = py::none(), py::arg("spearman") = true,
      py::arg("mode") = "MF_ICIRWeight", py::arg("filter") = SCFilter_IgnoreNan(),
      R"(SE_MultiFactor2

    创建基于多因子评分的选择器，支持多种创建方式

    - 直接指定 MF:
      :param MultiFactorBase mf: 直接指定的多因子合成算法
      :param ScoresFilterBase filter: 评分过滤器

    - 使用FactorSet:
      :param FactorSet input: 因子集合
      :param int ic_n: 默认 IC 对应的 N 日收益率
      :param int ic_rolling_n: IC 滚动周期
      :param Stock ref_stk: 参考证券,用于日期对齐，未指定时为 sh000001
      :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
      :param ScoresFilterBase filter: 评分过滤器

    - 使用Indicator序列:
      :param sequense(Indicator) input: 原始因子列表
      :param int ic_n: 默认 IC 对应的 N 日收益率
      :param int ic_rolling_n: IC 滚动周期
      :param Stock ref_stk: 参考证券,用于日期对齐，未指定时为 sh000001
      :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
      :param ScoresFilterBase filter: 评分过滤器

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor2(indicators)
        
        # 使用FactorSet
        factor_set = FactorSet(indicators)
        selector2 = SE_MultiFactor2(factor_set)
        
        # 直接使用MultiFactor对象
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor2(mf))");

    m.def("crtSEOptimal", crtSEOptimal, R"(crtSEOptimal(func)
    
    快速创建自定义绩效评估函数的寻优选择器

    :param func: 一个可调用对象，接收参数为 (sys, lastdate)，返回一个 float 数值)");

    m.def("SE_MaxFundsOptimal", SE_MaxFundsOptimal, "账户资产最大寻优选择器");

    m.def("SE_PerformanceOptimal", SE_PerformanceOptimal, py::arg("key") = "帐户平均年收益率%",
          py::arg("mode") = 0, R"(SE_PerformanceOptimal(key="帐户平均年收益率%", mode=0)

    使用 Performance 统计结果进行寻优的选择器

    :param string key: Performance 统计项
    :param int mode:  0 取统计结果最大的值系统 | 1 取统计结果为最小值的系统)");

    m.def(
      "SE_EvaluateOptimal",
      [](py::object evalulate_func) {
          py::object pyfunc = evalulate_func.attr("__call__");
          check_pyfunction_arg_num(pyfunc, 2);
          return SE_EvaluateOptimal([=](const SystemPtr& sys, const Datetime& enddate) {
              py::object pyfunc = evalulate_func.attr("__call__");
              return pyfunc(sys, enddate).cast<double>();
          });
      },
      R"(SE_EvaluateOptimal(evalulate_func)

    使用自定义函数进行寻优的选择器

    :param func: 一个可调用对象，接收参数为 (sys, lastdate)，返回一个 float 数值)");
}
