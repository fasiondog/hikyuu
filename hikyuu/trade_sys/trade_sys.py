# -*- coding: utf8 -*-

from hikyuu.util.slice import list_getitem
from hikyuu.core import (
    System, SystemPart, ConditionBase, EnvironmentBase, MoneyManagerBase,
    ProfitGoalBase, SelectorBase, SignalBase, SlippageBase, StoplossBase
)


def part_iter(self):
    for i in range(len(self)):
        yield self[i]


ConditionBase.__iter__ = part_iter


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
def cn_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def crtCN(func, params={}, name='crtCN'):
    """
    快速创建自定义不带私有属性的系统有效条件

    :param func: 系统有效条件函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义系统有效条件实例
    """
    meta_x = type(name, (ConditionBase, ), {'__init__': cn_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# environment
# ------------------------------------------------------------------
def ev_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def crtEV(func, params={}, name='crtEV'):
    """
    快速创建自定义不带私有属性的市场环境判断策略

    :param func: 市场环境判断策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义市场环境判断策略实例
    """
    meta_x = type(name, (EnvironmentBase, ), {'__init__': ev_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# moneymanager
# ------------------------------------------------------------------
def mm_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def crtMM(func, params={}, name='crtMM'):
    """
    快速创建自定义不带私有属性的资金管理策略

    :param func: 资金管理策略计算函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义资金管理策略实例
    """
    meta_x = type(name, (MoneyManagerBase, ), {'__init__': mm_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# profitgoal
# ------------------------------------------------------------------
def pg_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def crtPG(func, params={}, name='crtPG'):
    """
    快速创建自定义不带私有属性的盈利目标策略

    :param func: 盈利目标策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 盈利目标策略实例
    """
    meta_x = type(name, (ProfitGoalBase, ), {'__init__': pg_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# signal
# ------------------------------------------------------------------
def sig_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def crtSG(func, params={}, name='crtSG'):
    """
    快速创建自定义不带私有属性的信号指示器

    :param func: 信号策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义信号指示器实例
    """
    meta_x = type(name, (SignalBase, ), {'__init__': sig_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
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

# ------------------------------------------------------------------
# slippage
# ------------------------------------------------------------------


def sl_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def crtSL(func, params={}, name='crtSL'):
    """
    快速创建自定义不带私有属性的移滑价差算法

    :param func: 移滑价差算法函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 移滑价差算法实例
    """
    meta_x = type(name, (SlippageBase, ), {'__init__': sl_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# stoploss
# ------------------------------------------------------------------
def st_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def crtST(func, params={}, name='crtST'):
    """
    快速创建自定义不带私有属性的止损/止盈策略

    :param func: 止损/止盈策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 止损/止盈策略实例
    """
    meta_x = type(name, (StoplossBase, ), {'__init__': st_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)
