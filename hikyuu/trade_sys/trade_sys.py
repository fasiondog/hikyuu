# -*- coding: utf8 -*-

from hikyuu.core import (
    System, SystemPart, ConditionBase, EnvironmentBase, MoneyManagerBase,
    AllocateFundsBase, ProfitGoalBase, SelectorBase, SignalBase, SlippageBase, StoplossBase,
    MultiFactorBase, ScoresFilterBase, NormalizeBase
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
    Quickly create a system validity condition

    :param func: the system validity condition function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom system validity condition instance
    """
    meta_x = type(name, (ConditionBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    # Force importing the global namespace, to avoid losing the virtual function interfaces of the custom inheritance when used by the hub
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# environment
# ------------------------------------------------------------------
def crtEV(func, params={}, name='crtEV'):
    """
    Quickly create an environment judgement strategy

    :param func: the environment judgement strategy function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom environment judgement strategy instance
    """
    meta_x = type(name, (EnvironmentBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# moneymanager
# ------------------------------------------------------------------
def crtMM(get_buy_num, get_sell_num=None, params={}, name='crtMM', buy_notify=None, sell_notify=None):
    """
    Quickly create a money management strategy

    :param get_buy_num: the interface to get the buy number
    :param get_sell_num: the interface to get the sell number, defaults to None (sell all)
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param buy_notify: receive the notification of the buy trade records
    :param sell_notify: receive the notification of the sell trade records
    :return: the custom money management strategy instance
    """
    meta_x = type(name, (MoneyManagerBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._get_buy_num = get_buy_num
    if get_sell_num is not None:
        meta_x._get_sell_num = get_sell_num
    if buy_notify is not None:
        meta_x._buy_notify = buy_notify
    if sell_notify is not None:
        meta_x._sell_notify = sell_notify
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# profitgoal
# ------------------------------------------------------------------
def crtPG(get_goal, calculate=None, params={}, name='crtPG', buy_notify=None, sell_notify=None):
    """
    Quickly create a profit goal strategy

    :param get_goal: the interface to get the goal price
    :param calculate: the internal calculation interface (called when the trading object is specified)
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param buy_notify: receive the notification of the buy trade records
    :param sell_notify: receive the notification of the sell trade records
    :return: the profit goal strategy instance
    """
    meta_x = type(name, (ProfitGoalBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x.get_goal = get_goal
    if calculate is not None:
        meta_x._calculate = calculate
    if buy_notify is not None:
        meta_x._buy_notify = buy_notify
    if sell_notify is not None:
        meta_x._sell_notify = sell_notify
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# signal
# ------------------------------------------------------------------
def crtSG(func, params={}, name='crtSG'):
    """
    Quickly create a signal generator

    :param func: the signal strategy function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom signal generator instance
    """
    meta_x = type(name, (SignalBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# Selector
# ------------------------------------------------------------------
def crtSE(calculate, get_selected, params={}, name='crtSE'):
    """
    Quickly create a trading object selection algorithm

    :param calculate function: the calculation function
    :param get_selected_on_close function: the selection algorithm at the close time
    :param get_selected_on_open function: the selection algorithm at the open time
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom trading object selection algorithm instance
    """
    meta_x = type(name, (SelectorBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = calculate
    meta_x.get_selected = get_selected
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# allocate_funds
# ------------------------------------------------------------------
def crtAF(allocate_func, params={}, name='crtAF', to_targets_func=None, check_risk_func=None):
    """
    Quickly create an asset allocation algorithm (AF)

    :param allocate_func: the L1 system-level allocation interface, the prototype is
        ``func(self, date, tm, contexts, query)``, it returns ``{System: weight}``; in mode B it
        writes back ``contexts[i].quota`` in place and returns an empty table
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param to_targets_func: the L2 behavior-level conversion interface, the prototype is
        ``func(self, date, tm, suggestions, sys_weight, query)``, optional
    :param check_risk_func: the L3 portfolio risk control interface, the prototype is
        ``func(self, date, tm, suggestions, query)``, optional
    :return: the custom asset allocation algorithm instance
    """
    meta_x = type(name, (AllocateFundsBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._allocate = allocate_func
    if to_targets_func is not None:
        meta_x._to_targets = to_targets_func
    if check_risk_func is not None:
        meta_x._check_risk = check_risk_func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# multi_factor
# ------------------------------------------------------------------
def crtMF(calculate_func, params={}, name='crtMF'):
    """
    Quickly create a multi-factor composition algorithm

    :param calculate_func: the composition algorithm
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom multi-factor composition algorithm instance
    """
    meta_x = type(name, (MultiFactorBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = calculate_func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# slippage
# ------------------------------------------------------------------
def crtSP(get_real_buy_price, get_real_sell_price, params={}, name='crtSP', calculate=None):
    """
    Quickly create a slippage algorithm

    :param get_real_buy_price: the slippage algorithm interface to calculate the actual buy price
    :param get_real_sell_price: the slippage algorithm interface to calculate the actual sell price
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param calculate: the pre-processing function
    :return: the slippage algorithm instance
    """
    meta_x = type(name, (SlippageBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x.get_real_buy_price = get_real_buy_price
    meta_x.get_real_sell_price = get_real_sell_price
    if calculate is not None:
        meta_x._calculate = calculate
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# stoploss
# ------------------------------------------------------------------
def crtST(get_price, params={}, name='crtST', calculate=None, get_short_price=None):
    """
    Quickly create a stop-loss/take-profit strategy

    :param get_price: the interface function of the stop-loss/take-profit strategy to get the stop price
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param calculate: the initialization calculation function of the stop-loss/take-profit strategy
    :param get_short_price: the interface function to get the short stop price
    :return: the stop-loss/take-profit strategy instance
    """
    meta_x = type(name, (StoplossBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x.get_price = get_price
    if calculate is not None:
        meta_x._calculate = calculate
    if get_short_price is not None:
        meta_x.get_short_price = get_short_price
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# SCFilter
# ------------------------------------------------------------------
def crtSCFilter(filter_func, params={}, name='crtSCFilter'):
    """
    Quickly create a score filter

    :param filter_func: the score filter function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the score filter instance
    """
    meta_x = type(name, (ScoresFilterBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._filter = filter_func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# Normalize
# ------------------------------------------------------------------
def crtNorm(normalize_func, params={}, name='crtNorm'):
    """
    Quickly create algorithm functions such as standardization/normalization

    :param normalize_func: the algorithm function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the function instance
    """
    meta_x = type(name, (NormalizeBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._normalize = normalize_func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret
