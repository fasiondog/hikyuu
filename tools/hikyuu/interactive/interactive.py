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

#================================================================
#更新实时数据
def UpdateOneRealtimeRecord(tmpstr):
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
        
def realtimePartUpdate(queryStr):
    result = urllib.request.urlopen(queryStr).read()
    try:
        result = result.decode('gbk')
    except Exception as e:
        print(result)
        print(e)
        return
    
    result = result.split('\n')
    for tmpstr in result:
        UpdateOneRealtimeRecord(tmpstr)
        
def realtimeUpdate():
    queryStr = "http://hq.sinajs.cn/list="  
    max_size = 140
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
                urls.append(tmpstr)
                count = 0
                tmpstr = queryStr
    
    from multiprocessing import Pool
    from multiprocessing.dummy import Pool as ThreadPool
    pool = ThreadPool()
    pool.map(realtimePartUpdate, urls)
    pool.close()
    pool.join()
