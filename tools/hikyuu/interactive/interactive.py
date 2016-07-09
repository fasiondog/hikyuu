#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20120926, Added by fasiondog
#===============================================================================

from hikyuu import *
from hikyuu.indicator import *
from hikyuu.trade_manage import *

from hikyuu.trade_sys.system import *
from hikyuu.trade_sys.environment import *
from hikyuu.trade_sys.condition import *
from hikyuu.trade_sys.moneymanager import *
from hikyuu.trade_sys.signal import *
from hikyuu.trade_sys.stoploss import *
from hikyuu.trade_sys.profitgoal import *
from hikyuu.trade_sys.slippage import *

from .drawplot import *
#import time

from . import volume as vl
from . import elder as el
from . import kaufman as kf

import urllib
import tushare as ts

import sys
if sys.platform == 'win32':
    #config_file = "d:\\workspace\\hikyuu\\test\\data\\hikyuu_win.ini"
    config_file = "c:\\stock\\hikyuu_win.ini"
else:
    config_file = "/home/fasiondog/stock/hikyuu_linux.ini"
    
#starttime = time.time()
#print "Loading Day Data ..."
hikyuu_init(config_file)
sm = StockManager.instance()
#endtime = time.time()
#print "%.2fs" % (endtime-starttime)

blocka = Block("A", "ALL")
for s in sm:
    if s.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_GEM):
        blocka.add(s)

blocksh = Block("A", "SH")
for s in blocka:
    if s.market == "SH":
        blocksh.add(s)

blocksz = Block("A", "SZ")
for s in blocka:
    if s.market == "SZ":
        blocksz.add(s)

blockg = Block("G", "GEM")
for s in sm:
    if s.type == constant.STOCKTYPE_GEM:
        blockg.add(s)        

KData.plot = kplot
KData.kplot = kplot
KData.mkplot = mkplot

Indicator.plot = iplot
Indicator.bar = ibar

SignalBase.plot = sgplot
ConditionBase.plot = cnplot

#================================================================
#更新实时数据
def UpdateOneRealtimeRecord_from_sina(tmpstr):
    try:
        if len(tmpstr) > 3 and tmpstr[:3] == 'var':
            a = tmpstr.split(',')
            if len(a) < 9:
                return
            
            open, close, high, low = float(a[1]), float(a[3]), float(a[4]), float(a[5])
            transamount = float(a[9])
            transcount = float(a[8])
                
            d = Datetime(a[-3]+" 00")
            temp = (open, high, low, close)
            if 0 in temp:
                return

            stockstr = a[0].split('=')
            stock = sm[stockstr[0][-8:]]
            
            record = KRecord()
            record.datetime = d
            record.openPrice = open
            record.highPrice = high
            record.lowPrice = low
            record.closePrice = close
            record.transAmount = transamount
            record.transCount = transcount/100

            stock.realtimeUpdate(record)
    
    except Exception as e:
        print(tmpstr)
        print(e)

def UpdateOneRealtimeRecord_from_qq(tmpstr):
    try:
        if len(tmpstr) > 3 and tmpstr[:2] == 'v_':
            a = tmpstr.split('~')
            if len(a) < 9:
                return
            
            open, close, high, low = float(a[5]), float(a[3]), float(a[33]), float(a[34])
            transamount = float(a[36])
            transcount = float(a[37])
                
            d = Datetime(int(a[30][:8] + '0000'))
            temp = (open, high, low, close)
            if 0 in temp:
                return

            stockstr = a[0].split('=')
            stock = sm[stockstr[0][-8:]]
            
            record = KRecord()
            record.datetime = d
            record.openPrice = open
            record.highPrice = high
            record.lowPrice = low
            record.closePrice = close
            record.transAmount = transamount
            record.transCount = transcount/100

            stock.realtimeUpdate(record)
    
    except Exception as e:
        print(tmpstr)
        print(e)
        
def realtimePartUpdate_from_sina(queryStr):
    result = urllib.request.urlopen(queryStr).read()
    try:
        result = result.decode('gbk')
    except Exception as e:
        print(result)
        print(e)
        return
    
    result = result.split('\n')
    for tmpstr in result:
        UpdateOneRealtimeRecord_from_sina(tmpstr)
        
def realtimePartUpdate_from_qq(queryStr):
    result = urllib.request.urlopen(queryStr).read()
    try:
        result = result.decode('gbk')
    except Exception as e:
        print(result)
        print(e)
        return
    
    result = result.split('\n')
    for tmpstr in result:
        UpdateOneRealtimeRecord_from_qq(tmpstr)
                
def realtimeUpdate_from_sina_qq(source):
    if source == 'sina':
        queryStr = "http://hq.sinajs.cn/list=" 
        update_func = realtimePartUpdate_from_sina
        max_size = 140
    elif source == 'qq':
        queryStr = "http://qt.gtimg.cn/q="
        update_func = realtimePartUpdate_from_qq
        max_size = 60
    else:
        print('Not support!')
        return
        
    count  = 0
    urls = []
    tmpstr = queryStr
    for stock in sm:
        if stock.valid and stock.type in (constant.STOCKTYPE_A, 
                                          constant.STOCKTYPE_INDEX,
                                          constant.STOCKTYPE_ETF, 
                                          constant.STOCKTYPE_GEM):
            tmpstr += ("%s,") % (stock.market_code.lower())
            count = count + 1
            if count >= max_size:
                #urls.append(tmpstr)
                update_func(tmpstr)
                count = 0
                tmpstr = queryStr

    if tmpstr != queryStr:
        #urls.append(tmpstr)
        update_func(tmpstr)
   
    #不用并行，防止过快，ip被网站屏蔽
    #from multiprocessing import Pool
    #from multiprocessing.dummy import Pool as ThreadPool
    #pool = ThreadPool()
    #if source == 'sina':
    #    pool.map(realtimePartUpdate_from_sina, urls)
    #else:
    #    pool.map(realtimePartUpdate_from_qq, urls)
    #pool.close()
    #pool.join()

    
def realtimeUpdate_from_tushare():
    #更新股票行情
    df = ts.get_today_all()
    for i in range(len(df)):
        if df.ix[i, 'open'] == 0:
            continue  #停牌
        
        code = df.ix[i][0]
        stock = getStock('sh' + code)
        
        if stock.isNull() == True or stock.type != constant.STOCKTYPE_A:
            stock = getStock('sz' + code)
        if stock.isNull() == True:
            continue
        
        record = KRecord()
        record.openPrice = df.ix[i, 'open']
        record.highPrice = df.ix[i, 'high']
        record.lowPrice = df.ix[i, 'low']
        record.closePrice = df.ix[i, 'trade']
        record.transCount = df.ix[i, 'volume']
        
        from datetime import date
        d = date.today()
        record.datetime = Datetime(d)
        stock.realtimeUpdate(record)
        
    #更新指数行情
    df = ts.get_index()
    for i in range(len(df)):
        code = df.ix[i][0]
        stock = getStock('sh' + code)
        if stock.isNull() == True or stock.type != constant.STOCKTYPE_INDEX:
            stock = getStock('sz' + code)
        if stock.isNull() == True:
            continue
        
        total = stock.getCount(Query.DAY)
        if total == 0:
            continue
            
        last_record = stock.getKRecord(total - 1)
        
        record = KRecord()
        record.openPrice = df.ix[i, 'open']
        record.highPrice = df.ix[i, 'high']
        record.lowPrice = df.ix[i, 'low']
        record.closePrice = df.ix[i, 'close']
        record.transCount = float(df.ix[i, 'volume'])
        record.transAmount = df.ix[i, 'amount']
        
        if (last_record.closePrice != record.closePrice 
                or last_record.highPrice != record.highPrice 
                or last_record.lowPrice != record.lowPrice
                or last_record.openPrice != record.openPrice):
            from datetime import date
            d = date.today()
            record.datetime = Datetime(d)
            stock.realtimeUpdate(record)

def realtimeUpdate(source = 'sina'):
    if source == 'sina' or source == 'qq':
        realtimeUpdate_from_sina_qq(source)
    elif source == 'tushare':
        realtimeUpdate_from_tushare()
    else:
        print('Not support!')