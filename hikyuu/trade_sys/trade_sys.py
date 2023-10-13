# -*- coding: utf8 -*-

from hikyuu.util.slice import list_getitem
from hikyuu.cpp.core import SYS_Simple as cpp_SYS_Simple
from hikyuu.cpp.core import (
    System, SystemList, SystemWeight, SystemWeightList, ConditionBase, EnvironmentBase, MoneyManagerBase,
    ProfitGoalBase, SelectorBase, SignalBase, SlippageBase, StoplossBase
)

#------------------------------------------------------------------
# System
#------------------------------------------------------------------

System.Part.__doc__ = """
系统部件枚举值，系统的买入/卖出等操作可由这些部件触发，用于标识实际交易指令的来源，
参见：:py:class:`TradeRecord`。

实际使用中，可使用 System.ENVIRONMENT 的简化方式 代替 System.Part.ENVIRONMENT，
其他与此类似。

- System.Part.ENVIRONMENT  - 市场环境判断策略
- System.Part.CONDITION    - 系统有效条件
- System.Part.SIGNAL       - 信号指示器
- System.Part.STOPLOSS     - 止损策略
- System.Part.TAKEPROFIT   - 止盈策略
- System.Part.MONEYMANAGER - 资金管理策略
- System.Part.PROFITGOAL   - 盈利目标策略
- System.Part.SLIPPAGE     - 移滑价差算法
- System.Part.INVALID      - 无效值边界，大于等于该值时为无效部件
"""

System.ENVIRONMENT = System.Part.ENVIRONMENT
System.CONDITION = System.Part.CONDITION
System.SIGNAL = System.Part.SIGNAL
System.STOPLOSS = System.Part.STOPLOSS
System.TAKEPROFIT = System.Part.TAKEPROFIT
System.MONEYMANAGER = System.Part.MONEYMANAGER
System.PROFITGOAL = System.Part.PROFITGOAL
System.SLIPPAGE = System.Part.SLIPPAGE
System.INVALID = System.Part.INVALID

SystemList.__getitem__ = list_getitem
SystemList.__str__ = lambda self: str(list(self))
SystemList.__repr__ = lambda self: repr(list(self))


def SYS_Simple(tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None):
    """
    创建简单系统实例（每次交易不进行多次加仓或减仓，即每次买入后在卖出时全部卖出）， 
     系统实例在运行时(调用run方法），至少需要一个配套的交易管理实例、一个资金管理策略
    和一个信号指示器），可以在创建系统实例后进行指定。如果出现调用run时没有任何输出，
    且没有正确结果的时候，可能是未设置tm、sg、mm。进行回测时，使用 run 方法，如::
    
        #创建模拟交易账户进行回测，初始资金30万
        my_tm = crtTM(init_cash = 300000)

        #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA作为慢线，快线向上穿越
        #慢线时买入，反之卖出）
        my_sg = SG_Flex(EMA(C, n=5), slow_n=10)

        #固定每次买入1000股
        my_mm = MM_FixedCount(1000)

        #创建交易系统并运行
        sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
        sys.run(sm['sz000001'], Query(-150))
    
    :param TradeManager tm: 交易管理实例 
    :param MoneyManager mm: 资金管理策略
    :param EnvironmentBase ev: 市场环境判断策略
    :param ConditionBase cn: 系统有效条件
    :param SignalBase sg: 信号指示器
    :param StoplossBase st: 止损策略
    :param StoplossBase tp: 止盈策略
    :param ProfitGoalBase pg: 盈利目标策略
    :param SlippageBase sp: 移滑价差算法
    :return: system实例    
    """
    sys_ins = cpp_SYS_Simple()
    if tm:
        sys_ins.tm = tm
    if mm:
        sys_ins.mm = mm
    if ev:
        sys_ins.ev = ev
    if cn:
        sys_ins.cn = cn
    if sg:
        sys_ins.sg = sg
    if st:
        sys_ins.st = st
    if tp:
        sys_ins.tp = tp
    if pg:
        sys_ins.pg = pg
    if sp:
        sys_ins.sp = sp
    return sys_ins


#------------------------------------------------------------------
# allocatefunds
#------------------------------------------------------------------

SystemWeightList.__getitem__ = list_getitem
SystemWeightList.__str__ = lambda self: str(list(self))
SystemWeightList.__repr__ = lambda self: repr(list(self))

#------------------------------------------------------------------
# condition
#------------------------------------------------------------------

ConditionBase.__init__.__doc__ = """
__init__(self[, name="ConditionBase"])
    
    初始化构造函数
        
    :param str name: 名称
"""


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


#------------------------------------------------------------------
# environment
#------------------------------------------------------------------

EnvironmentBase.__init__.__doc__ = """
__init__(self[, name='EnvironmentBase'])
    
    初始化构造函数
        
    :param str name: 名称
"""


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


#------------------------------------------------------------------
# moneymanager
#------------------------------------------------------------------

MoneyManagerBase.__init__.__doc__ = """
__init__(self[, name="MoneyManagerBase])
    
    初始化构造函数
        
    :param str name: 名称
"""


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


#------------------------------------------------------------------
# profitgoal
#------------------------------------------------------------------

ProfitGoalBase.__init__.__doc__ = """
__init__(self[, name="ProfitGoalBase"])
    
    初始化构造函数
        
    :param str name: 名称
"""


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


#------------------------------------------------------------------
# selector
#------------------------------------------------------------------

SelectorBase.__init__.__doc__ = """
__init__(self[, name="SelectorBase])
    
    初始化构造函数
        
    :param str name: 名称
"""

#------------------------------------------------------------------
# signal
#------------------------------------------------------------------

SignalBase.__init__.__doc__ = """
__init__(self[, name="SignalBase"])
    
    :param str name: 名称
"""


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


#------------------------------------------------------------------
# Selector
#------------------------------------------------------------------
def se_add_stock_list(self, stk_list, proto_sys):
    result = True
    for stk in stk_list:
        success = self.add_stock(stk, proto_sys)
        if not success:
            result = False
            break
    return result


SelectorBase.add_stock_list = se_add_stock_list

#------------------------------------------------------------------
# slippage
#------------------------------------------------------------------

SlippageBase.__init__.__doc__ = """
__init__(self[, name="SlippageBase"])
    
    初始化构造函数
        
    :param str name: 名称
"""


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


#------------------------------------------------------------------
# stoploss
#------------------------------------------------------------------

StoplossBase.__init__.__doc__ = """
__init__(self[, name="StoplossBase"])
    
    :param str name: 名称
"""


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
