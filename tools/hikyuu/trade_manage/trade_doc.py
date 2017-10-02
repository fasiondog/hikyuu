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

from .trade import *
from hikyuu.trade_manage._trade_manage import CostRecord, TradeCostBase,\
    PositionRecord, FundsRecord, TradeManager


#------------------------------------------------------------------
# CostRecord
#------------------------------------------------------------------

CostRecord.__doc__ = """交易成本记录"""
CostRecord.commission.__doc__ = """佣金(float)"""
CostRecord.stamptax.__doc__ = """印花税(float)"""
CostRecord.transferfee.__doc__ = """过户费(float)"""
CostRecord.others.__doc__ = """其它费用(float)"""
CostRecord.total.__doc__ = """总成本(float)，= 佣金 + 印花税 + 过户费 + 其它费用"""


#------------------------------------------------------------------
# PositionRecord
#------------------------------------------------------------------

PositionRecord.__doc__ = """持仓记录"""
PositionRecord.stock.__doc__ = """交易对象（Stock）"""
PositionRecord.takeDatetime.__doc__ = """初次建仓时刻（Datetime）"""
PositionRecord.cleanDatetime.__doc__ = """平仓日期，当前持仓记录中为 constant.null_datetime"""
PositionRecord.number.__doc__ = """当前持仓数量（int）"""
PositionRecord.stoploss.__doc__ = """当前止损价（float）"""
PositionRecord.goalPrice.__doc__ = """当前的目标价格（float）"""
PositionRecord.totalNumber.__doc__ = """累计持仓数量（int）"""
PositionRecord.buyMoney.__doc__ = """累计买入资金（float）"""
PositionRecord.totalCost.__doc__ = """累计交易总成本（float）"""
PositionRecord.totalRisk.__doc__ = """累计交易风险（float） = 各次 （买入价格-止损)*买入数量, 不包含交易成本"""
PositionRecord.sellMoney.__doc__ = """累计卖出资金（float）"""

PositionRecordList.__doc__ = """持仓记录列表，C++ std::vector<PositionRecord>包装"""


#------------------------------------------------------------------
# TradeCost
#------------------------------------------------------------------

TradeCostBase.__doc__ = """
交易成本算法基类

实现自定义交易成本算法接口：

* :py:meth:`TradeCostBase.getBuyCost` - 【必须】获取买入成本
* :py:meth:`TradeCostBase.getSellCost` - 【必须】获取卖出成本
* :py:meth:`TradeCostBase._clone` - 【必须】子类克隆接口
"""

TradeCostBase.name.__doc__ = """交易算法名称"""

TradeCostBase.getParam.__doc__ = """
getParam(name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

TradeCostBase.setParam.__doc__ = """
setParam(name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

TradeCostBase.clone.__doc__ = """
clone()
    
    克隆操作
"""

TradeCostBase.getBuyCost.__doc__ = """
getBuyCost(datetime, stock, price, num)
    
    【重载接口】获取买入成本
        
    :param Datetime datetime: 买入时刻
    :param Stock stock: 买入对象
    :param float price: 买入价格
    :param size_t num: 买入数量
    :return: 交易成本记录
    :rtype: CostRecord
"""

TradeCostBase.getSellCost.__doc__ = """
getSellCost()
    
    【重载接口】获取卖出成本
        
    :param Datetime datetime: 卖出时刻
    :param Stock stock: 卖出对象
    :param float price: 卖出价格
    :param size_t num: 卖出数量
    :return: 交易成本记录
    :rtype: CostRecord
"""

TradeCostBase._clone.__doc__ = """
_clone()
    
    【重载接口】子类克隆接口
"""

#------------------------------------------------------------------
# FundsRecord
#------------------------------------------------------------------

FundsRecord.__doc__ = """当前资产情况记录，由TradeManager.getFunds返回"""
FundsRecord.cash.__doc__ = """当前现金（float）"""
FundsRecord.market_value.__doc__ = """当前多头市值（float）"""
FundsRecord.short_market_value.__doc__ = """当前空头仓位市值（float）"""
FundsRecord.base_cash.__doc__ = """当前投入本金（float）"""
FundsRecord.base_asset.__doc__ = """当前投入的资产价值（float）"""
FundsRecord.borrow_cash.__doc__ = """当前借入的资金（float），即负债"""
FundsRecord.borrow_asset.__doc__ = """当前借入证券资产价值（float）"""


#------------------------------------------------------------------
# OrderBroker
#------------------------------------------------------------------

OrderBrokerBase.__doc__ = """
Python中非必须使用 OrderBrokerBase 来实现自定义的订单代理。只要 Python 的对象
包含 buy、sell方法，其方法参数规则与 :py:class:`OrderBrokerWrap` 中的 _buy、
_sell 方法相同，即可参见前一章节快速创建订单代理实例。

自定义订单代理接口：

* :py:meth:`OrderBrokerBase._buy` - 【必须】执行实际买入操作
* :py:meth:`OrderBrokerBase._sell` - 【必须】执行实际卖出操作
"""

OrderBrokerBase.name.__doc__ = """名称（可读写）"""

OrderBrokerBase.buy.__doc__ = """
buy(self, code, price, num)

    执行买入操作
    
    :param str code: 证券代码
    :param float price: 买入价格
    :param int num: 买入数量
    :return: 买入操作的执行时刻
    :rtype: Datetime
"""

OrderBrokerBase.sell.__doc__ = """
sell(self, code, price, num)
    
    执行买入操作
    
    :param str code: 证券代码
    :param float price: 买入价格
    :param int num: 买入数量
    :return: 卖出操作的执行时刻
    :rtype: Datetime
"""

OrderBrokerBase._buy.__doc__ = """
_buy(self, code, price, num)

    【重载接口】执行实际买入操作
    
    :param str code: 证券代码
    :param float price: 买入价格
    :param int num: 买入数量
"""

OrderBrokerBase._sell.__doc__ = """
_sell(self, code, price, num)
    
    【重载接口】执行实际买入操作
    
    :param str code: 证券代码
    :param float price: 买入价格
    :param int num: 买入数量
"""


#------------------------------------------------------------------
# TradeManager
#------------------------------------------------------------------

TradeManager.__doc__ = """
交易管理类，可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

公共参数：

    reinvest=False (bool) : 红利是否再投资
    precision=2 (int) : 金额计算精度
    support_borrow_cash=False (bool) : 是否自动融资
    support_borrow_stock=False (bool) : 是否自动融券
    save_action=True (bool) : 是否保存Python命令序列
"""

TradeManager.__init__.__doc__ = """
__init__(self, datetime, initcash, costfunc, name)
    
    初始化构造函数
        
    :param Datetime datetime: 账户建立日期
    :param float initCash: 初始资金
    :param TradeCostBase costFunc: 成本算法
    :param str name: 账户名称
"""

TradeManager.name.__doc__ = """名称"""
TradeManager.initCash.__doc__ = """（只读）初始资金"""
TradeManager.initDatetime.__doc__ = """（只读）账户建立日期"""
TradeManager.costFunc.__doc__ = """交易成本算法"""

TradeManager.firstDatetime.__doc__ = """
（只读）第一笔买入交易发生日期，如未发生交易返回 null_datetime
"""

TradeManager.lastDatetime.__doc__ = """
（只读）最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期
"""

TradeManager.reinvest.__doc__ = """
（只读）红利/股息/送股再投资标志，同公共参数“reinvest”
"""

TradeManager.precision.__doc__ = """
（只读）价格精度，同公共参数“precision”
"""

TradeManager.brokeLastDatetime.__doc__ = """
实际开始订单代理操作的时刻。
        
默认情况下，TradeManager会在执行买入/卖出操作时，调用订单代理执行代理的买入/卖出动作，
但这样在实盘操作时会存在问题。因为系统在计算信号指示时，需要回溯历史数据才能得到最新的
信号，这样TradeManager会在历史时刻就执行买入/卖出操作，此时如果订单代理本身没有对发出
买入/卖出指令的时刻进行控制，会导致代理发送错误的指令。此时，需要指定在某一个时刻之后，
才允许指定订单代理的买入/卖出操作。属性 brokeLastDatetime 即用于指定该时刻。
"""

TradeManager.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

TradeManager.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

TradeManager.reset.__doc__ = """
reset(self)
    
    复位，清空交易、持仓记录
"""

TradeManager.clone.__doc__ = """
clone(self)

    克隆（深复制）实例
        
    :rtype: TradeManager
"""


