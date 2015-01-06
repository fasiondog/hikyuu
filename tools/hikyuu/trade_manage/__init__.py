#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130213, Added by fasiondog
#===============================================================================

from . import _trade_manage as ctm
from hikyuu import Datetime
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

TradeManager = ctm.TradeManager
CostRecord = ctm.CostRecord
PositionRecord = ctm.PositionRecord
PositionRecordList = ctm.PositionRecordList
TradeCostBase = ctm.TradeCostBase
TradeRecord = ctm.TradeRecord
TradeRecordList = ctm.TradeRecordList
FundsRecord = ctm.FundsRecord
BorrowRecord = ctm.BorrowRecord
BorrowRecordList = ctm.BorrowRecordList
Performance = ctm.Performance

BUSINESS = ctm.BUSINESS
BUSINESS.INIT = BUSINESS.BUSINESS_INIT
BUSINESS.BUY = BUSINESS.BUSINESS_BUY
BUSINESS.SELL = BUSINESS.BUSINESS_SELL
BUSINESS.GIFT = BUSINESS.BUSINESS_GIFT
BUSINESS.BONUS = BUSINESS.BUSINESS_BONUS
BUSINESS.CHECKIN = BUSINESS.BUSINESS_CHECKIN
BUSINESS.CHECKOUT = BUSINESS.BUSINESS_CHECKOUT
BUSINESS.INVALID = BUSINESS.INVALID_BUSINESS

   
TradeManager.__unicode__ = unicodeFunc
TradeManager.__repr__ = reprFunc
CostRecord.__unicode__ = unicodeFunc
CostRecord.__repr__ = reprFunc
TradeRecord.__unicode__ = unicodeFunc
TradeRecord.__repr__ = reprFunc
PositionRecord.__unicode__ = unicodeFunc
PositionRecord.__repr__ = reprFunc
TradeCostBase.__unicode__ = unicodeFunc
TradeCostBase.__repr__ = reprFunc
FundsRecord.__unicode__ = unicodeFunc
FundsRecord.__repr__ = reprFunc
BorrowRecord.__unicode__ = unicodeFunc
BorrowRecord.__repr__ = reprFunc

TestStub_TC = ctm.TestStub_TC
crtZeroTC = ctm.crtZeroTC
Zero_TC = ctm.crtZeroTC

def crtFixedATC(commission = 0.0018, lowest_commission = 5.0, stamptax = 0.001,
                transferfee = 0.001, lowest_transferfee = 1.0):
    """
    沪深A股交易成本算法,计算每次买入或卖出的成本
    计算规则为：
       1）上证交易所
          买入：佣金＋过户费
          卖出：佣金＋过户费＋印花税
       2）深证交易所：
          买入：佣金
          卖出：佣金＋印花税
       佣金和过户费均有最低值，当前佣金比例为千分之1.8（最低5元），印花税为千分之一
       上证过户费为交易数量的千分之一，不足1元，按一元计
    参数：
        commission 佣金比例，默认千分之1.8，即0.0018
        lowest_commission 最低佣金值，默认5元
        stamptax 印花税，默认千分之一，即0.001
        transferfee 过户费，默认每股千分之一，即0.001
        lowestTransferfee 最低过户费，默认1元
    """
    return ctm.crtFixedATC(commission, lowest_commission, stamptax, 
                           transferfee, lowest_transferfee)

FixedA_TC = crtFixedATC

def crtTM(datetime = Datetime(199001010000), initcash = 100000, 
          costfunc = crtZeroTC(), name = "SYS"):
    """
    创建交易管理模块，管理帐户的交易记录及资金使用情况
    考虑的移滑价差需要使用当日的最高或最低价，所以不在该模块内进行处理
    参数:
       datetime 账户建立日期, 默认1990-1-1
       initcash 初始现金，默认100000
       costfunc 交易成本算法,默认零成本算法
       name 账户名称，默认“SYS”
    """
    return ctm.crtTM(datetime, initcash, costfunc, name)
    