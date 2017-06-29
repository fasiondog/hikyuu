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

#Puppet是一套以同花顺交易客户端为核心的完整的闭环实盘交易系统框架。
#来自："睿瞳深邃(https://github.com/Raytone-D" 感谢睿瞳深邃的共享 :-)
#可以用：tm.regBroker(crtRB(Puppet())) 的方式注册进tm实例，实现实盘下单
from .puppet import *

from . import volume as vl
from . import elder as el
from . import kaufman as kf

import urllib

import sys
import os
import configparser

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

System.plot = sysplot

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
            

class OrderBrokerWrap(OrderBrokerBase):
    """
    订单代理包装类，用户可以参考自定义自己的订单代理，加入额外的处理
    :param bool real: 下单前是否重新实时获取实时分笔数据
    :param float slip: 如果当前的卖一价格和指示买入的价格绝对差值不超过slip则下单，
                        否则忽略; 对卖出操作无效，立即以当前价卖出
    """
    def __init__(self, broker, real=True, slip=0.03):
        super(OrderBrokerWrap, self).__init__()
        self._broker = broker
        self._real = real
        self._slip=slip
        
    def _buy(self, code, price, num):
        if self._real:
            import tushare as ts
            df = ts.get_realtime_quotes(code)
            new_price = df.ix[0]['ask']
            if (abs(new_price - price) <= self._slip):
                self._broker.buy(code, new_price, num)
            else:
                print("out of slip, not buy!!!!!!!!!!")
        else:
            self._broker.buy(code, price, num)
        
    def _sell(self, code, price, num):
        if self._real:
            import tushare as ts
            df = ts.get_realtime_quotes(code)
            new_price = df.ix[0]['bid']
            self._broker.sell(code, new_price, num)
        else:
            self._broker.sell(code, price, num)
        
class TestOrderBroker:
    def __init__(self):
        pass
    
    def buy(self, code, price, num):
        print("买入：%s  %.3f  %i" % (code, price, num))
    
    def sell(self, code, price, num):
        print("卖出：%s  %.3f  %i" % (code, price, num))
    
        
def crtRB(broker, real=True, slip=0.03):
    """
    快速生成订单代理包装对象
    :param broker: 订单代理示例，必须拥有buy和sell方法，并且参数为 code, price, num
    :param bool real: 下单前是否重新实时获取实时分笔数据
    :param float slip: 如果当前的卖一价格和指示买入的价格绝对差值不超过slip则下单，
                        否则忽略; 对卖出操作无效，立即以当前价卖出
    """
    return OrderBrokerWrap(broker)