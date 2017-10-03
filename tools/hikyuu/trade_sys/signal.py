#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


from ._trade_sys import (SignalBase,
                         SG_Bool,
                         SG_Single,
                         SG_Single2,
                         SG_Cross,
                         SG_CrossGold,
                         SG_Flex)
from hikyuu.util.unicode import (unicodeFunc, reprFunc)


SignalBase.__unicode__ = unicodeFunc
SignalBase.__repr__ = reprFunc


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



#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

SignalBase.__doc__ = """
信号指示器负责产生买入、卖出信号。

公共参数：

    alternate (bool|True) ：买入和卖出信号是否交替出现。单线型的信号通常通过拐点、
                            斜率等判断信号的产生，此种情况下可能出现连续出现买入
                            信号或连续出现卖出信号的情况，此时可通过该参数控制买入、
                            卖出信号是否交替出现。而双线交叉型的信号通常本身买入和
                            卖出已经是交替出现，此时该参数无效。
                            
自定义的信号指示器接口：

    SignalBase._calculate() - 【必须】子类计算接口
    SignalBase._clone() - 【必须】克隆接口
    SignalBase._reset() - 【可选】重载私有变量


示例1（不含私有变量，海龟交易策略）:

    from hikyuu.trade_sys.signal import SignalBase
    from hikyuu.indicator import HHV, LLV, CLOSE, REF
    
    class TurtleSignal(SignalBase):
        def __init__(self, n = 20):
            super(TurtleSignal, self).__init__("TurtleSignal")
            self.setParam("n", 20)
            
        def _clone(self):
            return TurtleSignal()
    
        def _calculate(self):
            n = self.getParam("n")
            k = self.getTO()
            c = CLOSE(k)
            h = REF(HHV(c, n), 1) #前n日高点
            L = REF(LLV(c, n), 1) #前n日低点
            for i in range(h.discard, len(k)):
                if (c[i] >= h[i]):
                    self._addBuySignal(k[i].datetime)
                elif (c[i] <= L[i]):
                    self._addSellSignal(k[i].datetime)
    
    if __name__ == "__main__":
        from examples_init import *
        
        sg = TurtleSignal()
        s = getStock("sh000001")
        k = s.getKData(Query(-500))
        
        #只有设置交易对象时，才会开始实际计算
        sg.setTO(k)
        dates = k.getDatetimeList()
        for d in dates:
            if (sg.shouldBuy(d)):
                print("买入：%s" % d)
            elif (sg.shouldSell(d)):
                print("卖出: %s" % d)

示例2（含私有属性）:

    class SignalPython(SignalBase):
        def __init__(self):
            super(SignalPython, self).__init__("SignalPython")
            self._x = 0 #私有属性
            self.setParam("test", 30)
    
        def _reset(self):
            self._x = 0
    
        def _clone(self):
            p = SignalPython()
            p._x = self._x
            return p
    
        def _calculate(self):
            self._addBuySignal(Datetime(201201210000))
            self._addSellSignal(Datetime(201201300000))
"""

SignalBase.name.__doc__ = """名称"""

SignalBase.__init__.__doc__ = """
__init__(self[, name="SignalBase"])
    
    :param str name: 名称
"""

SignalBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

SignalBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

SignalBase.setTO.__doc__ = """
setTO(self, k)
    
    :param KData k: 设置交易对象
"""

SignalBase.getTO.__doc__ = """
getTO(self)
    
    :return: 交易对象
    :rtype: KData
"""

SignalBase.shouldBuy.__doc__ = """
shouldBuy(self, datetime)
    
    指定时刻是否可以买入
    
    :param Datetime datetime: 指定时刻
    :rtype: bool
"""

SignalBase.shouldSell.__doc__ = """
shouldSell(self, datetime)
    
    指定时刻是否可以卖出
        
    :param Datetime datetime: 指定时刻
    :rtype: bool
"""

SignalBase.getBuySignal.__doc__ = """
getBuySignal(self)
    
    获取所有买入指示日期列表
        
    :rtype: DatetimeList
"""

SignalBase.getSellSignal.__doc__ = """
getSellSignal(self)
    
    获取所有卖出指示日期列表
        
    :rtype: DatetimeList
"""

SignalBase._addBuySignal.__doc__ = """
_addBuySignal(self, datetime)
    
    加入买入信号，在_calculate中调用
        
    :param Datetime datetime: 指示买入的日期
"""

SignalBase._addSellSignal.__doc__ = """
_addSellSignal(self, datetime)
    
    加入卖出信号，在_calculate中调用

    :param Datetime datetime: 指示卖出的日期
"""

SignalBase.reset.__doc__ = """
reset(self)
    
    复位操作
"""

SignalBase.clone.__doc__ = """
clone(self)
    
    克隆操作
"""

SignalBase._calculate.__doc__ = """
_calculate(self)
    
    【重载接口】子类计算接口
"""

SignalBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，复位内部私有变量
"""

SignalBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

SG_Flex.__doc__ = """
SG_Flex(ind, slow_n[, kpart = 'CLOSE'])

    使用自身的EMA(slow_n)作为慢线，自身作为快线，快线向上穿越慢线买入，快线向下穿越慢线卖出。

    :param Operand ind:
    :param int slow_n: 慢线EMA周期
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器
"""

SG_Cross.__doc__ = """
SG_Cross(fast, slow[, kpart = "CLOSE"])
    
    双线交叉指示器，当快线从下向上穿越慢线时，买入；当快线从上向下穿越慢线时，卖出。如：5日MA上穿10日MA时买入，5日MA线下穿MA10日线时卖出:: 

        SG_Cross(OP(MA(n=10)), OP(MA(n=30)))

    :param Operand fast: 快线
    :param Operand slow: 慢线
    :param string kpart: OPEN|HIGH|LOW|CLOSE|AMO|VOL|KDATA
    :return: 信号指示器
"""

SG_CrossGold.__doc__ = """
SG_CrossGold(fast, slow[, kpart = "CLOSE"])

    金叉指示器，当快线从下向上穿越慢线且快线和慢线的方向都是向上时为金叉，买入；
    当快线从上向下穿越慢线且快线和慢线的方向都是向下时死叉，卖出。::
    
        SG_CrossGold(OP(MA(n=10)), OP(MA(n=30)))
    
    :param Operand fast: 快线
    :param Operand slow: 慢线
    :param string kpart: OPEN|HIGH|LOW|CLOSE|AMO|VOL|KDATA
    :return: 信号指示器  
"""

SG_Single.__doc__ = """
SG_Single(ind[, filter_n = 10, filter_p = 0.1, kpart='CLOSE'])
    
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

SG_Single2.__doc__ = """
SG_Single2(ind[, filter_n = 10, filter_p = 0.1, kpart='CLOSE'])
    
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

SG_Bool.__doc__ = """
SG_Bool(buy, sell[, kpart='CLOSE'])

    布尔信号指示器，使用运算结果为类似bool数组的Operand分别作为买入、卖出指示。
    
    :param Operand buy: 买入指示（结果Indicator中相应位置>0则代表买入）
    :param Operand sell: 卖出指示（结果Indicator中相应位置>0则代表卖出）
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器
"""
