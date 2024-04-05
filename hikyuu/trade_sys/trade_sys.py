# -*- coding: utf8 -*-

from hikyuu.core import (
    System, SystemPart, ConditionBase, EnvironmentBase, MoneyManagerBase,
    ProfitGoalBase, SelectorBase, SignalBase, SlippageBase, StoplossBase, AllocateFundsBase,
    MultiFactorBase
)


def part_iter(self):
    for i in range(len(self)):
        yield self[i]


ConditionBase.__iter__ = part_iter


def part_init(self, name='', params={}):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def part_clone(self):
    cloned = self.__class__.__new__(self.__class__)
    self.__class__.__init__(cloned, self)
    cloned.__dict__.update(self.__dict__)
    return cloned


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
def crtCN(func, params={}, name='crtCN'):
    """
    快速创建系统有效条件

    :param func: 系统有效条件函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义系统有效条件实例
    """
    meta_x = type(name, (ConditionBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# environment
# ------------------------------------------------------------------
def crtEV(func, params={}, name='crtEV'):
    """
    快速创建市场环境判断策略

    :param func: 市场环境判断策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义市场环境判断策略实例
    """
    meta_x = type(name, (EnvironmentBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# moneymanager
# ------------------------------------------------------------------
def crtMM(get_buy_num, get_sell_num, params={}, name='crtMM', buy_notify=None, sell_notify=None):
    """
    快速创建资金管理策略

    :param get_buy_num: 买入数量接口
    :param sell_buy_num: 卖出数量接口
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param buy_notify: 接收买入交易记录通知
    :param sell_notify: 接收卖出交易记录通知
    :return: 自定义资金管理策略实例
    """
    meta_x = type(name, (MoneyManagerBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._get_buy_num = get_buy_num
    meta_x._get_sell_num = get_sell_num
    if buy_notify is not None:
        meta_x._buy_notify = buy_notify
    if sell_notify is not None:
        meta_x._sell_notify = sell_notify
    return meta_x(name, params)


# ------------------------------------------------------------------
# profitgoal
# ------------------------------------------------------------------
def crtPG(func, params={}, name='crtPG'):
    """
    快速创建盈利目标策略

    :param func: 盈利目标策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 盈利目标策略实例
    """
    meta_x = type(name, (ProfitGoalBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# signal
# ------------------------------------------------------------------
def crtSG(func, params={}, name='crtSG'):
    """
    快速创建信号指示器

    :param func: 信号策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义信号指示器实例
    """
    meta_x = type(name, (SignalBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    return meta_x(name, params)


# ------------------------------------------------------------------
# Selector
# ------------------------------------------------------------------
def crtSE(calculate, get_selected, is_match_af=None, params={}, name='crtSE'):
    """
    快速创建交易对象选择算法

    :param calculate function: 计算函数
    :param get_selected_on_close function: 收盘时刻选择算法
    :param get_selected_on_open function: 开盘时刻选择算法
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义交易对象选择算法实例
    """
    meta_x = type(name, (SelectorBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = calculate
    meta_x.get_selected = get_selected
    meta_x.is_match_af = (lambda self, af: True) if is_match_af is None else is_match_af
    return meta_x(name, params)


# ------------------------------------------------------------------
# allocatefunds
# ------------------------------------------------------------------
def crtAF(allocate_weight_func, params={}, name='crtAF'):
    """
    快速创建资产分配算法

    :param allocate_weight_func: 资产分配算法
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义资产分配算法实例
    """
    meta_x = type(name, (AllocateFundsBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._allocate_weight = allocate_weight_func
    return meta_x(name, params)


# ------------------------------------------------------------------
# multi_factor
# ------------------------------------------------------------------
def crtMF(calculate_func, params={}, name='crtMF'):
    """
    快速多因子合成算法

    :param calculate_func: 合成算法
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义多因子合成算法实例
    """
    meta_x = type(name, (MultiFactorBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = calculate_func
    return meta_x(name, params)


# ------------------------------------------------------------------
# slippage
# ------------------------------------------------------------------
def crtSP(get_real_buy_price, get_real_sell_price, params={}, name='crtSP', calculate=None):
    """
    快速创建移滑价差算法

    :param get_real_buy_price: 移滑价差算法接口计算实际买入价格
    :param get_real_sell_price: 移滑价差算法接口计算实际买入价格
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param calculate: 预处理函数
    :return: 移滑价差算法实例
    """
    meta_x = type(name, (SlippageBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x.get_real_buy_price = get_real_buy_price
    meta_x.get_real_sell_price = get_real_sell_price
    if calculate is not None:
        meta_x._calculate = calculate
    return meta_x(name, params)


# ------------------------------------------------------------------
# stoploss
# ------------------------------------------------------------------
def crtST(func, params={}, name='crtST'):
    """
    快速创建止损/止盈策略

    :param func: 止损/止盈策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 止损/止盈策略实例
    """
    meta_x = type(name, (StoplossBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    return meta_x(name, params)
