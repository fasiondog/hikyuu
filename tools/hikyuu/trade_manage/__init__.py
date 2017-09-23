#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130213, Added by fasiondog
#===============================================================================

from ._trade_manage import *
from hikyuu import Datetime, Query, QueryByDate, StockManager
from hikyuu.util.slice import list_getitem
from hikyuu.util.unicode import (unicodeFunc, reprFunc)
from hikyuu.trade_sys.system import getSystemPartName

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

BorrowRecordList.__getitem__ = list_getitem
PositionRecordList.__getitem__ = list_getitem
TradeRecordList.__getitem__ = list_getitem

try:
    import numpy as np
    import pandas as pd

    def TradeList_to_np(t_list):
        """转化为numpy结构数组"""
        t_type = np.dtype({'names':['交易日期','证券代码', '证券名称', '业务名称', 
                                    '计划交易价格', '实际成交价格', '目标价格', 
                                    '成交数量', '佣金', '印花税', '过户费', 
                                    '其它成本', '交易总成本', '止损价', 
                                    '现金余额', '信号来源'], 
                'formats':['datetime64[D]','U10','U20', 'U10', 'd', 'd', 'd', 
                           'i', 'd', 'd', 'd', 'd','d', 'd', 'd', 'U5']})
        return np.array([(t.datetime, t.stock.market_code, t.stock.name,
                          getBusinessName(t.business), t.planPrice,
                          t.realPrice, t.goalPrice, t.number, 
                          t.cost.commission, t.cost.stamptax,
                          t.cost.transferfee, t.cost.others,
                          t.cost.total, t.stoploss, t.cash,
                          getSystemPartName(t.part)
                          ) for t in t_list], dtype=t_type)
        
    def TradeList_to_df(t):
        """转化为pandas的DataFrame"""
        return pd.DataFrame.from_records(TradeList_to_np(t), index='交易日期')

    TradeRecordList.to_np = TradeList_to_np
    TradeRecordList.to_df = TradeList_to_df
    
    def PositionList_to_np(pos_list):
        """转化为numpy结构数组"""
        t_type = np.dtype({'names':['证券代码', '证券名称', '买入日期', 
                                    '已持仓天数', '持仓数量', '投入金额', 
                                    '当前市值', '盈亏金额', '盈亏比例'], 
                'formats':['U10','U20', 'datetime64[D]', 'i', 'i', 'd',
                           'd', 'd', 'd']})
        
        sm = StockManager.instance()
        query = Query(-1)
        data = []
        for pos in pos_list:
            invest = pos.buyMoney - pos.sellMoney + pos.totalCost
            k = pos.stock.getKData(query)
            cur_val = k[0].closePrice * pos.number
            bonus = cur_val - invest
            date_list = sm.getTradingCalendar(QueryByDate(Datetime(pos.takeDatetime.date())))
            data.append((pos.stock.market_code, pos.stock.name, 
                         pos.takeDatetime, len(date_list), pos.number, 
                         invest, cur_val, bonus,
                         100 * bonus / invest))
        
        return np.array(data, dtype=t_type)
    
    def PositionList_to_df(pos_list):
        """转化为pandas的DataFrame"""
        return pd.DataFrame.from_records(PositionList_to_np(pos_list), index='证券代码')

        
    PositionRecordList.to_np = PositionList_to_np
    PositionRecordList.to_df = PositionList_to_df
    
except:
    pass

TC_FixedA.__doc__ = """
    2015年8月1日之前沪深A股交易成本算法,计算每次买入或卖出的成本
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
        lowest_transferfee 最低过户费，默认1元
    """

    
TC_FixedA2015.__doc__ = """
    2015年8月1日及之后沪深A股交易成本算法,计算每次买入或卖出的成本
    计算规则为：
       1）上证交易所
          买入：佣金＋过户费
          卖出：佣金＋过户费＋印花税
       2）深证交易所：
          买入：佣金
          卖出：佣金＋印花税
       佣金最低5元
       上证过户费为交易金额的千分之0.02
    参数：
        commission 佣金比例，默认千分之1.8，即0.0018
        lowest_commission 最低佣金值，默认5元
        stamptax 印花税，默认千分之一，即0.001
        transferfee 过户费，默认每股千分之一，即0.001
    """
    
    
crtTM.__doc__ = """
    创建交易管理模块，管理帐户的交易记录及资金使用情况
    考虑的移滑价差需要使用当日的最高或最低价，所以不在该模块内进行处理
    参数:
       datetime 账户建立日期, 默认1990-1-1
       initCash 初始现金，默认100000
       costFunc 交易成本算法,默认零成本算法TC_Zero()
       name 账户名称，默认“SYS”
    """   