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

#===============================================================================
# History:
# 1. 20120926, Added by fasiondog
#===============================================================================

import urllib
import sys
import os
import configparser

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
from hikyuu.trade_sys.selector import *
from hikyuu.trade_sys.allocatefunds import *
from hikyuu.trade_sys.portfolio import *

from hikyuu.interactive import *
#import time


#==============================================================================
# 引入扯线木偶
#==============================================================================
#Puppet是一套以同花顺交易客户端为核心的完整的闭环实盘交易系统框架。
#来自："睿瞳深邃(https://github.com/Raytone-D" 感谢睿瞳深邃的大度共享 :-)
#可以用：tm.regBroker(crtRB(Puppet())) 的方式注册进tm实例，实现实盘下单
if sys.platform == 'win32':
    from .puppet import *


#==============================================================================
#
# 读取配置信息，并初始化
#
#==============================================================================
data_config_file = os.path.expanduser('~') + "/.hikyuu/data_dir.ini"
data_config = configparser.ConfigParser()
data_config.read(data_config_file)
data_dir = data_config['data_dir']['data_dir']

if sys.platform == 'win32':
    config_file = data_dir + "\\hikyuu_win.ini"
else:
    config_file = data_dir + "/hikyuu_linux.ini"

#starttime = time.time()
#print "Loading Day Data ..."
#hikyuu_init(config_file)
#sm = StockManager.instance()
#endtime = time.time()
#print "%.2fs" % (endtime-starttime)
import configparser
ini = configparser.ConfigParser()
ini.read(config_file)
hku_param = Parameter()
hku_param.set("tmpdir", ini.get('hikyuu', 'tmpdir'))
if ini.has_option('hikyuu', 'logger'):
    hku_param.set("logger", ini['hikyuu']['logger'])
    
base_param = Parameter()
base_info_config = ini.options('baseinfo')
for p in base_info_config:
    base_param.set(p, ini.get('baseinfo', p))
    
block_param = Parameter()
block_config = ini.options('block')
for p in block_config:
    block_param.set(p, ini.get('block', p))
    
preload_param = Parameter()
preload_config = ini.options('preload')
for p in preload_config:
    #注意：proload参数是布尔类型
    preload_param.set(p, ini.getboolean('preload', p))
    
kdata_param = Parameter()
kdata_config = ini.options('kdata')
for p in kdata_config:
    kdata_param.set(p, ini.get('kdata', p))

sm = StockManager.instance()
sm.init(base_param, block_param, kdata_param, preload_param, hku_param)


#==============================================================================
#
# 引入blocka、blocksh、blocksz、blockg全局变量，便于交互式环境下使用
#
#==============================================================================

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
 
        
#==============================================================================
#
# 设置默认绘图引擎
#
#==============================================================================            

use_draw_engine('matplotlib')


#==============================================================================
#
# 增加临时的实时数据更新函数 realtimeUpdate
#
#==============================================================================

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
    import tushare as ts
    
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
        record.transAmount = float(df.ix[i, 'amount'])
        record.transCount = float(df.ix[i, 'volume'])
        
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
        record.transAmount = float(df.ix[i, 'amount'])
        
        if (last_record.closePrice != record.closePrice 
                or last_record.highPrice != record.highPrice 
                or last_record.lowPrice != record.lowPrice
                or last_record.openPrice != record.openPrice):
            from datetime import date
            d = date.today()
            record.datetime = Datetime(d)
            stock.realtimeUpdate(record)

def realtimeUpdate_inner(source='tushare'):
    if source == 'sina' or source == 'qq':
        realtimeUpdate_from_sina_qq(source)
    elif source == 'tushare':
        realtimeUpdate_from_tushare()
    else:
        print(source, ' not support!')
        
        
def realtimeUpdateWrap():
    pre_update_time = None
    def realtimeUpdate_closure(source='tushare', delta=60):
        """
        更新实时日线数据
        参数：
            source: 数据源（'sina' | 'qq' | 'tushare'）
            delta: 更新间隔时间
        """
        from datetime import timedelta, datetime
        nonlocal pre_update_time
        now_update_time = datetime.now()
        if (pre_update_time is None) or (now_update_time - pre_update_time) > timedelta(0, delta, 0):
            realtimeUpdate_inner(source)
            pre_update_time = datetime.now()
            print("更新完毕！", pre_update_time)
        else:
            print("更新间隔小于" + str(delta) + "秒，未更新")
            print("上次更新时间: ", pre_update_time)  
    return realtimeUpdate_closure

realtimeUpdate = realtimeUpdateWrap()
            

