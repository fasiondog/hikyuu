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
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

TradeCostBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

TradeCostBase.clone.__doc__ = """
clone(self)
    
    克隆操作
"""

TradeCostBase.getBuyCost.__doc__ = """
getBuyCost(self, datetime, stock, price, num)
    
    【重载接口】获取买入成本
        
    :param Datetime datetime: 买入时刻
    :param Stock stock: 买入对象
    :param float price: 买入价格
    :param size_t num: 买入数量
    :return: 交易成本记录
    :rtype: CostRecord
"""

TradeCostBase.getSellCost.__doc__ = """
getSellCost(self, datetime, stock, price, num)
    
    【重载接口】获取卖出成本
        
    :param Datetime datetime: 卖出时刻
    :param Stock stock: 卖出对象
    :param float price: 卖出价格
    :param int num: 卖出数量
    :return: 交易成本记录
    :rtype: CostRecord
"""

TradeCostBase._clone.__doc__ = """
_clone(self)
    
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

TradeManager.regBroker.__doc__ = """
regBroker(self, broker)
    
    注册订单代理。可执行多次该命令注册多个订单代理。
        
    :param OrderBrokerBase broker: 订单代理实例
"""

TradeManager.clearBroker.__doc__ = """
clearBroker(self)

    清空所有已注册订单代理
"""

TradeManager.have.__doc__ = """
have(self, stock)
    
    当前是否持有指定的证券
        
    :param Stock stock: 指定证券
    :rtype: bool
"""

TradeManager.getStockNumber.__doc__ = """
getStockNumber(self)
    
    当前持有的证券种类数量，即当前持有几只股票（非各个股票的持仓数）
        
    :rtype: int
"""

TradeManager.getHoldNumber.__doc__ = """
getHoldNumber(self, datetime, stock)

    获取指定时刻指定证券的持有数量
        
    :param Datetime datetime: 指定时刻
    :param Stock stock: 指定的证券
    :rtype: int
"""

TradeManager.getTradeList.__doc__ = """
getTradeList(self)
getTradeList(self, start, end)
    
    获取交易记录，未指定参数时，获取全部交易记录
        
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :rtype: TradeRecordList
"""

TradeManager.getPositionList.__doc__ = """
getPositionList(self)
    
    获取当前全部持仓记录
        
    :rtype: PositionRecordList
"""

TradeManager.getHistoryPositionList.__doc__ = """
getHistoryPositionList(self)
    
    获取全部历史持仓记录，即已平仓记录
        
    :rtype: PositionRecordList
"""

TradeManager.getPosition.__doc__ = """
getPosition(self, stock)

    获取指定证券的当前持仓记录，如当前未持有该票，返回PositionRecord()
        
    :param Stock stock: 指定的证券
    :rtype: PositionRecord
"""

TradeManager.getBuyCost.__doc__ = """
getBuyCost(self, datetime, stock, price, num)
    
    计算买入成本
        
    :param Datetime datetime: 交易时间
    :param Stock stock:       交易的证券
    :param float price:      买入价格
    :param int num:           买入数量
    :rtype: CostRecord
"""

TradeManager.getSellCost.__doc__ = """
getSellCost(self, datetime, stock, price, num)
    
    计算卖出成本

    :param Datetime datetime: 交易时间
    :param Stock stock:       交易的证券
    :param float price:      卖出价格
    :param int num:           卖出数量
    :rtype: CostRecord        
"""

TradeManager.cash.__doc__ = """
cash(self, datetime[, ktype=KQuery.KType.DAY])
    
    获取指定日期的现金。（注：如果不带日期参数，无法根据权息信息调整持仓。）
        
    :param Datetime datetime: 指定时刻
    :param ktype: K线类型
    :rtype: float
"""

TradeManager.getFunds.__doc__ = """
getFunds(self[,ktype = KQuery.DAY])
    
    获取账户当前时刻的资产详情
        
    :param KQuery.KType ktype: K线类型
    :rtype: FundsRecord
    
getFunds(self, datetime, [ktype = KQuery.DAY])
    
    获取指定时刻的资产市值详情
        
    :param Datetime datetime:  指定时刻
    :param KQuery.KType ktype: K线类型
    :rtype: FundsRecord    
"""

TradeManager.getFundsCurve.__doc__ = """
getFundsCurve(self, dates[, ktype = KQuery.DAY])
    
    获取资产净值曲线
        
    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的资产净值曲线
    :param KQuery.KType ktype: K线类型，必须与日期列表匹配
    :return: 资产净值列表
    :rtype: PriceList
"""

TradeManager.getProfitCurve.__doc__ = """
getProfitCurve(self, dates[, ktype = KQuery.DAY])
    
    获取收益曲线，即扣除历次存入资金后的资产净值曲线
        
    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
    :param KQuery.KType ktype: K线类型，必须与日期列表匹配
    :return: 收益曲线
    :rtype: PriceList
"""

TradeManager.checkin.__doc__ = """
checkin(self, datetime, cash)
    
    向账户内存入现金
    
    :param Datetime datetime: 交易时间
    :param float cash: 存入的现金量
    :rtype: TradeRecord
"""

TradeManager.checkout.__doc__ = """
checkout(self, datetime, cash)
    
    从账户内取出现金
        
    :param Datetime datetime: 交易时间
    :param float cash: 取出的资金量
    :rtype: TradeRecord
"""

TradeManager.buy.__doc__ = """
buy(self, datetime, stock, realPrice, number[, stoploss=0.0, goalPrice=0.0, planPrice=0.0, part=System.INVALID])
    
    买入操作
        
    :param Datetime datetime: 买入时间
    :param Stock stock:       买入的证券
    :param float realPrice:  实际买入价格
    :param int num:           买入数量
    :param float stoploss:   止损价
    :param float goalPrice:  目标价格
    :param float planPrice:  计划买入价格
    :param SystemPart part:   交易指示来源
    :rtype: TradeRecord
"""

TradeManager.sell.__doc__ = """
sell(self, datetime, stock, realPrice[, number=Constant.null_size, stoploss=0.0, goalPrice=0.0, planPrice=0.0, part=System.INVALID])
    
    卖出操作
        
    :param Datetime datetime: 卖出时间
    :param Stock stock:       卖出的证券
    :param float realPrice:  实际卖出价格
    :param int num:           卖出数量，如果等于Constant.null_size，表示全部卖出
    :param float stoploss:   新的止损价
    :param float goalPrice:  新的目标价格
    :param float planPrice:  原计划卖出价格
    :param SystemPart part:   交易指示来源
    :rtype: TradeRecord
"""

TradeManager.tocsv.__doc__ = """
tocsv(self, path)
    
    以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
        
    :param string path: 输出文件所在目录
"""

TradeManager.addTradeRecord.__doc__ = """
addTradeRecord(self, tr)

    直接加入交易记录，如果加入初始化账户记录，将清除全部已有交易及持仓记录。

    :param TradeRecord tr: 交易记录
    :return: True（成功） | False（失败）
    :rtype: bool
"""


#------------------------------------------------------------------
# Performance
#------------------------------------------------------------------

Performance.__doc__ = """简单绩效统计"""

Performance.reset.__doc__ = """
reset(self)
    
    复位，清除已计算的结果
"""

Performance.report.__doc__ = """
report(self, tm[, datetime=Datetime.now()])
    
    简单的文本统计报告，用于直接输出打印
        
    :param TradeManager tm: 指定的交易管理实例
    :param Datetime datetime: 统计截止时刻
    :rtype: str
"""

Performance.statistics.__doc__ = """
statistics(self, tm[, datetime=Datetime.now()])
    
    根据交易记录，统计截至某一时刻的系统绩效, datetime必须大于等于lastDatetime

    :param TradeManager tm: 指定的交易管理实例
    :param Datetime datetime: 统计截止时刻
"""

Performance.get.__doc__ = """
get(self, name)
    
    按指标名称获取指标值，必须在运行 statistics 或 report 之后生效
        
    :param str name: 指标名称
    :rtype: float
"""

Performance.__getitem__.__doc__ = """
__getitem__(self, name)
    
    同 get 方法。按指标名称获取指标值，必须在运行 statistics 或 report 之后生效
        
    :param str name: 指标名称
    :rtype: float
"""


#------------------------------------------------------------------
# build_in
#------------------------------------------------------------------

crtTM.__doc__ = """
crtTM([datetime = Datetime(199001010000), initcash = 100000, costfunc = TC_Zero(), name = "SYS"])

    创建交易管理模块，管理帐户的交易记录及资金使用情况
    
    :param Datetime datetime:  账户建立日期
    :param float initcash:    初始资金
    :param TradeCost costfunc: 交易成本算法
    :param string name:        账户名称
    :rtype: TradeManager
"""

TC_TestStub.__doc__ = """
"""

TC_FixedA.__doc__ = """
TC_FixedA([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.001, lowestTransferfee=1.0])

    2015年8月1日之前的A股交易成本算法。上证过户费为交易数量的千分之一，不足1元，按1元计。
    
    计算规则如下::

    1）上证交易所
        买入：佣金＋过户费
        卖出：佣金＋过户费＋印花税
    2）深证交易所：
        买入：佣金
        卖出：佣金＋印花税

    其中，佣金最低5元

    :param float commission: 佣金比例
    :param float lowestCommission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :param float lowestTransferfee: 最低过户费
    :return: :py:class:`TradeCostBase` 子类实例
"""

    
TC_FixedA2015.__doc__ = """
TC_FixedA2015([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.00002])

    2015年8月1日及之后的A股交易成本算法，上证过户费改为成交金额的千分之0.02
    
    计算规则如下::

    1）上证交易所
        买入：佣金＋过户费
        卖出：佣金＋过户费＋印花税
    2）深证交易所：
        买入：佣金
        卖出：佣金＋印花税

    其中，佣金最低5元

    :param float commission: 佣金比例
    :param float lowestCommission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :return: :py:class:`TradeCostBase` 子类实例
"""

TC_Zero.__doc__ = """
TC_Zero()

    创建零成本算法实例
"""

TC_TestStub.__doc__ = """仅用于测试"""
