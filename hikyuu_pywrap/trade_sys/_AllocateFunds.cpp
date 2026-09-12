/*
 * _AllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  组合级资金分配（AF）绑定。AF 承载 L1/L2/L3 三个可替换算法部件：
 *   - L1 _allocate  子系统上下文 → 权重
 *   - L2 _to_targets 权重       → 父账户可执行数量
 *   - L3 _check_risk 组合风控裁剪
 *  MM 已限定为单系统形态，不再承载组合级分配。
 *  见 docs/design/pf_af_compat/design.md §5
 */

#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyAllocateFundsBase : public AllocateFundsBase {
    PY_CLONE(PyAllocateFundsBase, AllocateFundsBase)

public:
    PyAllocateFundsBase() : AllocateFundsBase() {
        m_is_python_object = true;
    }

    PyAllocateFundsBase(const string& name) : AllocateFundsBase(name) {
        m_is_python_object = true;
    }

    PyAllocateFundsBase(const AllocateFundsBase& base) : AllocateFundsBase(base) {
        m_is_python_object = true;
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, AllocateFundsBase, _reset, );
    }

    // L1：子系统上下文 → 权重
    Weights _allocate(const Datetime& date, const TradeManagerPtr& tm, SubSystemContextList& contexts,
                      const KQuery& query) override {
        PYBIND11_OVERLOAD_NAME(Weights, AllocateFundsBase, "_allocate", _allocate, date, tm, contexts,
                               query);
    }

    // L2：权重 → 父账户可执行数量
    void _toTargets(const Datetime& date, const TradeManagerPtr& tm,
                    TradeSuggestionList& suggestions, const Weights& sys_weight,
                    const KQuery& query) override {
        PYBIND11_OVERLOAD_NAME(void, AllocateFundsBase, "_to_targets", _toTargets, date, tm,
                               suggestions, sys_weight, query);
    }

    // L3：组合风控裁剪
    void _checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                    TradeSuggestionList& suggestions, const KQuery& query) override {
        PYBIND11_OVERLOAD_NAME(void, AllocateFundsBase, "_check_risk", _checkRisk, date, tm,
                               suggestions, query);
    }
};

void export_AllocateFunds(py::module& m) {
    py::class_<AllocateFundsBase, AllocateFundsPtr, PyAllocateFundsBase>(
      m, "AllocateFundsBase", py::dynamic_attr(),
      R"(组合级资金分配（AF）基类，仅聚合系统（MultiSystem）使用

AF 由三个可替换算法部件构成（各自对应一个重载接口）：

    - _allocate   【必须】L1 系统级分配：子系统上下文 → 权重
    - _to_targets 【可选】L2 行为级换算：权重 → 父账户可执行数量
    - _check_risk 【可选】L3 组合风控裁剪

公共参数：

    - max-single-position=1.0 (float) : L3 单标的集中度上限（占总资产比例），<=0 或 >=1 表示不限制
    - weight-list="" (str) : L1 固定权重列表（逗号分隔），非空时按序覆盖等权默认
    - fixed-amount=0.0 (float) : L2 固定金额；模式 A 为每标的目标市值，模式 B 为每子系统额度)")
      .def(py::init<>())
      .def(py::init<const AllocateFundsBase&>())
      .def(py::init<const string&>(), R"(初始化构造函数

    :param str name: 名称)")

      .def("__str__", to_py_str<AllocateFundsBase>)
      .def("__repr__", to_py_str<AllocateFundsBase>)

      .def_property("name", py::overload_cast<>(&AllocateFundsBase::name, py::const_),
                    py::overload_cast<const string&>(&AllocateFundsBase::name),
                    py::return_value_policy::copy, "名称")
      .def_property("tm", &AllocateFundsBase::getTM, &AllocateFundsBase::setTM,
                    "设置或获取交易管理对象")
      .def_property("query", &AllocateFundsBase::getQuery, &AllocateFundsBase::setQuery,
                    py::return_value_policy::copy, "设置或获取查询条件")
      .def_property("mode", &AllocateFundsBase::getMode, &AllocateFundsBase::setMode,
                    py::return_value_policy::copy, "分配模式：A（信号汇总）/ B（资金划拨）")

      .def("get_param", &AllocateFundsBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param",
           static_cast<void (AllocateFundsBase::*)(const std::string&, const boost::any&)>(
             &AllocateFundsBase::setParam),
           R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &AllocateFundsBase::haveParam, "是否存在指定参数")
      .def("reset", &AllocateFundsBase::reset, "复位操作")
      .def("clone", &AllocateFundsBase::clone, "克隆操作")

      .def("allocate", &AllocateFundsBase::allocate, py::arg("date"), py::arg("tm"),
           py::arg("suggestions"), py::arg("contexts"), py::arg("query"),
           R"(allocate(self, date, tm, suggestions, contexts, query)

    L1/L2/L3 统一入口（通常由 MultiSystem 内部调用）)")

      .def("_allocate", &AllocateFundsBase::_allocate, py::arg("date"), py::arg("tm"),
           py::arg("contexts"), py::arg("query"),
           R"(_allocate(self, date, tm, contexts, query)

    【重载接口】L1 系统级分配：由子系统上下文决定各子系统权重

    :return: 权重表 { System: weight })")

      .def("_to_targets", &AllocateFundsBase::_toTargets, py::arg("date"), py::arg("tm"),
           py::arg("suggestions"), py::arg("sys_weight"), py::arg("query"),
           R"(_to_targets(self, date, tm, suggestions, sys_weight, query)

    【重载接口】L2 行为级换算：将权重换算为父账户可执行数量，就地改写 suggestions)")

      .def("_check_risk", &AllocateFundsBase::_checkRisk, py::arg("date"), py::arg("tm"),
           py::arg("suggestions"), py::arg("query"),
           R"(_check_risk(self, date, tm, suggestions, query)

    【重载接口】L3 组合风控裁剪，就地改写 suggestions 数量)")

      .def("_reset", &AllocateFundsBase::_reset,
           R"(【重载接口】子类复位接口，复位内部私有变量)")

        DEF_PICKLE(AllocateFundsPtr);

    //--------------------------------------------------------------------------------------
    // AF 内置算法
    m.def("AF_EqualWeight", AF_EqualWeight, R"(AF_EqualWeight()

    等权重资产分配，对选中的资产进行等比例分配（L1 等权 1/N）)");

    m.def("AF_FixedAmount", AF_FixedAmount, py::arg("amount") = 20000.0, R"(AF_FixedAmount(amount=20000.0)

    固定金额资产分配（L1 等权 + L2 固定金额）

    :param float amount: 交易最大金额)");

    m.def("AF_FixedWeight", AF_FixedWeight, py::arg("weight") = 0.1, R"(AF_FixedWeight(weight=0.1)

    固定比例资产分配（L1 直接返回固定比例，不归一化）

    :param float weight: 指定的资产比例 (0, 1])");

    m.def("AF_FixedWeightList", AF_FixedWeightList, py::arg("weights"),
          R"(AF_FixedWeightList(weights)

    固定比例列表资产分配（L1 按序取 weights[i]，不归一化）

    :param list weights: 指定的资产比例列表)");

    m.def("AF_MultiFactor", AF_MultiFactor, R"(AF_MultiFactor()

    创建 MultiFactor 评分权重的资产分配算法实例, 即直接以SE返回的评分作为权重。)");
}
