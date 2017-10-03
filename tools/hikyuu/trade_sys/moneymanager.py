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


from ._trade_sys import (MoneyManagerBase, 
                         MM_Nothing, 
                         MM_FixedRisk,
                         MM_FixedCapital,
                         MM_FixedCount,
                         MM_FixedPercent,
                         MM_FixedUnits,
                         MM_WilliamsFixedRisk)
from hikyuu.util.unicode import (unicodeFunc, reprFunc)


MoneyManagerBase.__unicode__ = unicodeFunc
MoneyManagerBase.__repr__ = reprFunc


def mm_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtMM(func, params={}, name='crtMM'):
    """
    快速创建自定义不带私有属性的资金管理策略
    
    :param func: 资金管理策略计算函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义资金管理策略实例
    """
    meta_x = type(name, (MoneyManagerBase,), {'__init__': mm_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

MoneyManagerBase.__doc__ = """
资金管理策略基类

公共参数：

    auto-checkin=False (bool) : 当账户现金不足以买入资金管理策略指示的买入数量时，
                                自动向账户中补充存入（checkin）足够的现金。
    max-stock=200 (int) : 最大持有的证券种类数量（即持有几只股票，而非各个股票的持仓数）

自定义资金管理策略接口：

    MoneyManagerBase.buyNotify() - 【可选】接收实际买入通知，预留用于多次增减仓处理
    MoneyManagerBase.sellNotify() - 【可选】接收实际卖出通知，预留用于多次增减仓处理
    MoneyManagerBase._getBuyNumber() - 【必须】获取指定交易对象可买入的数量
    MoneyManagerBase._getSellNumber() - 【可选】获取指定交易对象可卖出的数量，
                                        如未重载，默认为卖出全部已持仓数量
    MoneyManagerBase._reset() - 【可选】重置私有属性
    MoneyManagerBase._clone() - 【必须】克隆接口
"""

MoneyManagerBase.name.__doc__ = """名称"""

MoneyManagerBase.__init__.__doc__ = """
__init__(self[, name="MoneyManagerBase])
    
    初始化构造函数
        
    :param str name: 名称
"""

MoneyManagerBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
        
    .. note::

        所有的资金管理策略具有参数“auto-checkin”（bool类型，默认为False），
        其含义为“当账户现金不足以买入资金管理策略指示的买入数量时，自动向账户中
        补充存入（checkin）足够的现金。
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

MoneyManagerBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

MoneyManagerBase.reset.__doc__ = """
reset(self)
    
    复位操作
"""

MoneyManagerBase.clone.__doc__ = """
clone(self)
    
    克隆操作
"""

MoneyManagerBase.buyNotify.__doc__ = """
buyNotify(self, trade_record)
    
    【重载接口】交易系统发生实际买入操作时，通知交易变化情况，
    一般存在多次增减仓的情况才需要重载。
        
    :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录
"""

MoneyManagerBase.sellNotify.__doc__ = """
sellNotify(self, trade_record)
    
    【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，
    一般存在多次增减仓的情况才需要重载。
        
    :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录
"""

MoneyManagerBase.getBuyNumber.__doc__ = """
getBuyNumber(self, datetime, stock, price, risk)
    
    获取指定交易对象可买入的数量
        
    :param Datetime datetime: 交易时间
    :param Stock stock: 交易对象
    :param float price: 交易价格
    :param float risk: 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
    :return: 可买入数量
    :rtype: int
"""

MoneyManagerBase.getSellNumber.__doc__ = """
getSellNumber(self, datetime, stock, price, risk)
    
    获取指定交易对象可卖出的数量
        
    :param Datetime datetime: 交易时间
    :param Stock stock: 交易对象
    :param float price: 交易价格
    :param float risk: 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
    :return: 可卖出数量
    :rtype: int
"""

MoneyManagerBase._getBuyNumber.__doc__ = """
_getBuyNumber(self, datetime, stock, price, risk)

    【重载接口】获取指定交易对象可买入的数量
        
    :param Datetime datetime: 交易时间
    :param Stock stock: 交易对象
    :param float price: 交易价格
    :param float risk: 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
    :return: 可买入数量
    :rtype: int
"""

MoneyManagerBase._getSellNumber.__doc__ = """
_getSellNumber(self, datetime, stock, price, risk)
    
    【重载接口】获取指定交易对象可卖出的数量。如未重载，默认为卖出全部已持仓数量。
        
    :param Datetime datetime: 交易时间
    :param Stock stock: 交易对象
    :param float price: 交易价格
    :param float risk: 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
    :return: 可卖出数量
    :rtype: int
"""

MoneyManagerBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，复位内部私有变量
"""

MoneyManagerBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

MM_Nothing.__doc__ = """
MM_Nothing()

    特殊的资金管理策略，相当于不做资金管理，有多少钱买多少。
"""

MM_FixedRisk.__doc__ = """
MM_FixedRisk([risk = 1000.00])

    固定风险资金管理策略对每笔交易限定一个预先确定的或者固定的资金风险，
    如每笔交易固定风险1000元。公式：交易数量 = 固定风险 / 交易风险。

    :param float risk: 固定风险
    :return: 资金管理策略实例
"""

MM_FixedCapital.__doc__ = """
MM_FixedCapital([capital = 10000.0])

    固定资本资金管理策略

    :param float capital: 固定资本单位
    :return: 资金管理策略实例
"""

MM_FixedCount.__doc__ = """
MM_FixedCount([n = 100])

    固定交易数量资金管理策略。每次买入固定的数量。
    
    :param int n: 每次买入的数量（应该是交易对象最小交易数量的整数，此处程序没有此进行判断）
    :return: 资金管理策略实例
"""

MM_FixedPercent.__doc__ = """
MM_FixedPercent([p = 0.03])

    固定百分比风险模型。公式：P（头寸规模）＝ 账户余额 * 百分比 / R（每股的交易风险）。
    
    :param float p: 百分比
    :return: 资金管理策略实例
"""

MM_FixedUnits.__doc__ = """
MM_FixedUnits([n = 33])

    固定单位资金管理策略

    :param int n: n个资金单位
    :return: 资金管理策略实例
"""

MM_WilliamsFixedRisk.__doc__ = """
MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    威廉斯固定风险资金管理策略
""" 

    

