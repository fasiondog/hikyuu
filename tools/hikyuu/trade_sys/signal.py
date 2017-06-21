#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

SignalBase = csys.SignalBase
SignalBase.__unicode__ = unicodeFunc
SignalBase.__repr__ = reprFunc

SG_Single = csys.SG_Single
SG_Single2 = csys.SG_Single2
SG_Cross = csys.SG_Cross
SG_CrossGold = csys.SG_CrossGold
SG_Flex = csys.SG_Flex

def sig_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtSG(func, params={}, name='crtSG'):
    """
    快速创建自定义不带私有属性的信号指示器
    
    :param func: 信号策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义信号指示器实例
    """
    meta_x = type(name, (SignalBase,), {'__init__': sig_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)

SG_Flex.__doc__ += """\n
    使用自身的EMA(slow_n)作为慢线，自身作为快线，快线向上穿越慢线买入，快线向下穿越慢线卖出。

    :param Operand ind:
    :param int slow_n: 慢线EMA周期
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器
"""

SG_Cross.__doc__ += """\n
    双线交叉指示器，当快线从下向上穿越慢线时，买入；
    当快线从上向下穿越慢线时，卖出。
    如：5日MA上穿10日MA时买入，5日MA线下穿MA10日线时卖出:: 

        SG_Cross(OP(MA(n=10)), OP(MA(n=30)))

    :param Operand fast: 快线
    :param Operand slow: 慢线
    :param string kpart: OPEN|HIGH|LOW|CLOSE|AMO|VOL|KDATA
    :return: 信号指示器
"""

SG_CrossGold.__doc__ += """\n
    金叉指示器，当快线从下向上穿越慢线且快线和慢线的方向都是向上时为金叉，买入；
    当快线从上向下穿越慢线且快线和慢线的方向都是向下时死叉，卖出。::
    
        SG_CrossGold(OP(MA(n=10)), OP(MA(n=30)))
    
    :param Operand fast: 快线
    :param Operand slow: 慢线
    :param string kpart: OPEN|HIGH|LOW|CLOSE|AMO|VOL|KDATA
    :return: 信号指示器
"""

SG_Single.__doc__ += """\n
    生成单线拐点信号指示器。使用《精明交易者》 [BOOK1]_ 中给出的曲线拐点算法判断曲线趋势，公式见下::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter 
    
    :param Operand ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器
"""

SG_Single2.__doc__ += """\n
    生成单线拐点信号指示器2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter
    
    :param Operand ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器
"""
