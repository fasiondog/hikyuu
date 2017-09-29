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
    PositionRecord, FundsRecord


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
