# -*- coding: utf8 -*-

from hikyuu.util.slice import list_getitem
from hikyuu.core import (
    System, SystemPart, ConditionBase, EnvironmentBase, MoneyManagerBase,
    ProfitGoalBase, SelectorBase, SignalBase, SlippageBase, StoplossBase, AllocateFundsBase
)


def part_iter(self):
    for i in range(len(self)):
        yield self[i]


ConditionBase.__iter__ = part_iter


def part_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


# ------------------------------------------------------------------
# System
# ------------------------------------------------------------------
System.Part = SystemPart
System.ENVIRONMENT = System.Part.ENVIRONMENT
System.CONDITION = System.Part.CONDITION
System.SIGNAL = System.Part.SIGNAL
System.STOPLOSS = System.Part.STOPLOSS
System.TAKEPROFIT = System.Part.TAKEPROFIT
System.MONEYMANAGER = System.Part.MONEYMANAGER
System.PROFITGOAL = System.Part.PROFITGOAL
System.SLIPPAGE = System.Part.SLIPPAGE
System.INVALID = System.Part.INVALID


# ------------------------------------------------------------------
# condition
# ------------------------------------------------------------------
def crtCN(func, params={}, name='crtCN', clone=None):
    """
    快速创建系统有效条件

    :param func: 系统有效条件函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 自定义系统有效条件实例
    """
    meta_x = type(name, (ConditionBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# environment
# ------------------------------------------------------------------
def crtEV(func, params={}, name='crtEV', clone=None):
    """
    快速创建市场环境判断策略

    :param func: 市场环境判断策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 自定义市场环境判断策略实例
    """
    meta_x = type(name, (EnvironmentBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# moneymanager
# ------------------------------------------------------------------
def crtMM(func, params={}, name='crtMM', clone=None):
    """
    快速创建资金管理策略

    :param func: 资金管理策略计算函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 自定义资金管理策略实例
    """
    meta_x = type(name, (MoneyManagerBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# profitgoal
# ------------------------------------------------------------------
def crtPG(func, params={}, name='crtPG', clone=None):
    """
    快速创建盈利目标策略

    :param func: 盈利目标策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 盈利目标策略实例
    """
    meta_x = type(name, (ProfitGoalBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# signal
# ------------------------------------------------------------------
def crtSG(func, params={}, name='crtSG', clone=None):
    """
    快速创建信号指示器

    :param func: 信号策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 自定义信号指示器实例
    """
    meta_x = type(name, (SignalBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# Selector
# ------------------------------------------------------------------
def se_add_stock_list(self, stk_list, proto_sys):
    result = True
    for stk in stk_list:
        success = self.add_stock(stk, proto_sys)
        if not success:
            result = False
            break
    return result


SelectorBase.add_stock_list = se_add_stock_list


def crtSE(calculate, get_selected, is_match_af=None, params={}, name='crtSE', clone=None):
    """
    快速创建交易对象选择算法

    :param calculate function: 计算函数
    :param get_selected_on_close function: 收盘时刻选择算法
    :param get_selected_on_open function: 开盘时刻选择算法
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 自定义交易对象选择算法实例
    """
    meta_x = type(name, (SelectorBase, ), {'__init__': part_init})
    meta_x._calculate = calculate
    meta_x.get_selected = get_selected
    meta_x.is_match_af = lambda self, af: True if is_match_af is None else is_match_af
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    return meta_x(name, params)


# ------------------------------------------------------------------
# allocatefunds
# ------------------------------------------------------------------
def crtAF(allocate_weight_func, params={}, name='crtAF', clone=None):
    """
    快速创建资产分配算法

    :param allocate_weight_func: 资产分配算法
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 自定义资产分配算法实例
    """
    meta_x = type(name, (AllocateFundsBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._allocate_weight = allocate_weight_func
    return meta_x(name, params)


# ------------------------------------------------------------------
# slippage
# ------------------------------------------------------------------
def crtSL(func, params={}, name='crtSL', clone=None):
    """
    快速创建移滑价差算法

    :param func: 移滑价差算法函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 移滑价差算法实例
    """
    meta_x = type(name, (SlippageBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# stoploss
# ------------------------------------------------------------------
def crtST(func, params={}, name='crtST', clone=None):
    """
    快速创建止损/止盈策略

    :param func: 止损/止盈策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param clone: 带有私有属性时自定义的clone函数
    :return: 止损/止盈策略实例
    """
    meta_x = type(name, (StoplossBase, ), {'__init__': part_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params) if clone is None else clone
    meta_x._calculate = func
    return meta_x(name, params)
