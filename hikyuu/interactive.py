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

from hikyuu.data.hku_config_template import generate_default_config
from .draw import *
__copyright__ = """
MIT License

Copyright (c) 2010-2017 fasiondog

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import urllib
import sys
import os
import configparser

from hikyuu.data.hku_config_template import generate_default_config
from hikyuu import *

# 重定向C++ stdout/stderr输出至python
iodog = OstreamRedirect()
iodog.open()

# ==============================================================================
# 引入扯线木偶
# ==============================================================================
# Puppet是一套以同花顺交易客户端为核心的完整的闭环实盘交易系统框架。
# 来自："睿瞳深邃(https://github.com/Raytone-D" 感谢睿瞳深邃的大度共享 :-)
# 可以用：tm.regBroker(crtRB(Puppet())) 的方式注册进tm实例，实现实盘下单
if sys.platform == 'win32':
    from .puppet import *

# ==============================================================================
#
# 读取配置信息，并初始化
#
# ==============================================================================
config_file = os.path.expanduser('~') + "/.hikyuu/hikyuu.ini"
if not os.path.exists(config_file):
    # 创建默认配置
    hku_info("创建默认配置文件")
    generate_default_config()

ini = configparser.ConfigParser()
ini.read(config_file, encoding='utf-8')
hku_param = Parameter()
hku_param["tmpdir"] = ini.get('hikyuu', 'tmpdir')
hku_param["datadir"] = ini.get('hikyuu', 'datadir')
if ini.has_option('hikyuu', 'logger'):
    hku_param["logger"] = ini['hikyuu']['logger']
if ini.has_option('hikyuu', 'quotation_server'):
    hku_param["quotation_server"] = ini['hikyuu']['quotation_server']

base_param = Parameter()
base_info_config = ini.options('baseinfo')
for p in base_info_config:
    base_param[p] = ini.get('baseinfo', p)

block_param = Parameter()
block_config = ini.options('block')
for p in block_config:
    block_param[p] = ini.get('block', p)

preload_param = Parameter()
preload_config = ini.options('preload')
for p in preload_config:
    if p in ('day', 'week', 'month', 'quarter', 'halfyear', 'year', 'min', 'min5', 'min15', 'min30', 'min60', 'hour2'):
        preload_param[p] = ini.getboolean('preload', p)
    else:
        preload_param[p] = ini.getint('preload', p)

kdata_param = Parameter()
kdata_config = ini.options('kdata')
for p in kdata_config:
    if p == "convert":
        kdata_param[p] = ini.getboolean('kdata', p)
        continue
    kdata_param[p] = ini.get('kdata', p)

# set_log_level(LOG_LEVEL.INFO)
# sm = StockManager.instance()
sm.init(base_param, block_param, kdata_param, preload_param, hku_param)
set_log_level(LOG_LEVEL.INFO)

# 启动行情接收代理
start_spot_agent(False)

# ==============================================================================
#
# 引入blocka、blocksh、blocksz、blockg全局变量，便于交互式环境下使用
#
# ==============================================================================
blocka = Block("A", "ALL")
for s in sm:
    if s.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_GEM):
        blocka.add(s)
zsbk_a = blocka

blocksh = Block("A", "SH")
for s in blocka:
    if s.market == "SH":
        blocksh.add(s)
zsbk_sh = blocksh

blocksz = Block("A", "SZ")
for s in blocka:
    if s.market == "SZ":
        blocksz.add(s)
zsbk_sz = blocksz

blockbj = Block("A", "BJ")
for s in blocka:
    if s.market == "BJ":
        blockbj.add(s)
zsbk_bj = blockbj

blockg = Block("G", "创业板")
for s in sm:
    if s.type == constant.STOCKTYPE_GEM:
        blockg.add(s)
zsbk_cyb = blockg

blockstart = Block("START", "科创板")
for s in sm:
    if s.type == constant.STOCKTYPE_START:
        blockstart.add(s)

blockzxb = Block("A", "中小板")
for s in blocksz:
    if s.code[:3] == "002":
        blockzxb.add(s)
zsbk_zxb = blockzxb

zsbk_sz50 = sm.get_block("指数板块", "上证50")
zsbk_sz180 = sm.get_block("指数板块", "上证180")
zsbk_hs300 = sm.get_block("指数板块", "沪深300")
zsbk_zz100 = sm.get_block("指数板块", "中证100")


def set_global_context(stk, query):
    """设置全局的 context

        :param Stock stk: 指定的全局Stock
        :param Query query: 指定的查询条件
    """
    global K, O, C, H, L, A, V
    K = stk.get_kdata(query)
    O.set_context(K)
    C.set_context(K)
    H.set_context(K)
    L.set_context(K)
    A.set_context(K)
    V.set_context(K)


def get_global_context():
    """获取当前的全局上下文

        :rtype: KData
    """
    return C.getContext()


set_global_context(sm['sh000001'], Query(-150))

# ==============================================================================
#
# 设置默认绘图引擎
#
# ==============================================================================

use_draw_engine('matplotlib')

# ==============================================================================
#
# 粗略的选股函数
#
# ==============================================================================


def select(cond, start=Datetime(201801010000), end=Datetime.now(), print_out=True):
    """
    示例：
    #选出涨停股
    C = CLOSE()
    x = select(C / REF(C, 1) - 1 >= 0.0995))

    :param Indicator cond: 条件指标
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :param bool print_out: 打印选中的股票
    :rtype: 选中的股票列表
    """
    q = Query(start, end)
    d = sm.get_trading_calendar(q, 'SH')
    if len(d) == 0:
        return

    result = []
    for s in blocka:
        if not s.valid:
            continue

        q = Query(start, end)
        k = s.get_kdata(q)
        cond.set_context(k)
        if len(cond) > 0 and cond[-1] != constant.null_price and cond[-1] > 0 and len(k
                                                                                      ) > 0 and k[-1].datetime == d[-1]:
            result.append(s)
            if print_out:
                print(d[-1], s)

    return result


# ==============================================================================
#
# 增加临时的实时数据更新函数 realtime_update
#
# ==============================================================================


def UpdateOneRealtimeRecord_from_sina(tmpstr):
    try:
        if len(tmpstr) > 3 and tmpstr[:3] == 'var':
            a = tmpstr.split(',')
            if len(a) < 9:
                return

            open, close, high, low = float(a[1]), float(a[3]), float(a[4]), float(a[5])
            transamount = float(a[9])
            transcount = float(a[8])

            try:
                d = Datetime(a[-3])
            except:
                d = Datetime(a[-4])
            temp = (open, high, low, close)
            if 0 in temp:
                return

            stockstr = a[0].split('=')
            stock = sm[stockstr[0][-8:]]

            record = KRecord()
            record.date = d
            record.open = open
            record.high = high
            record.low = low
            record.close = close
            record.amount = transamount
            record.volume = transcount / 100

            stock.realtime_update(record)

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
            record.date = d
            record.open = open
            record.high = high
            record.low = low
            record.close = close
            record.amount = transamount
            record.volume = transcount / 100

            stock.realtime_update(record)

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


def realtime_update_from_sina_qq(source):
    if source == 'sina':
        hku_error("sina已不支持获取实时数据")
        return
        # queryStr = "http://hq.sinajs.cn/list="
        # update_func = realtimePartUpdate_from_sina
        # max_size = 140
    elif source == 'qq':
        queryStr = "http://qt.gtimg.cn/q="
        update_func = realtimePartUpdate_from_qq
        max_size = 60
    else:
        print('Not support!')
        return

    count = 0
    # urls = []
    tmpstr = queryStr
    for stock in sm:
        if stock.valid and stock.type in (
            constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_ETF, constant.STOCKTYPE_GEM
        ):
            tmpstr += ("%s,") % (stock.market_code.lower())
            count = count + 1
            if count >= max_size:
                # urls.append(tmpstr)
                update_func(tmpstr)
                count = 0
                tmpstr = queryStr

    if tmpstr != queryStr:
        # urls.append(tmpstr)
        update_func(tmpstr)

    # 不用并行，防止过快，ip被网站屏蔽
    # from multiprocessing import Pool
    # from multiprocessing.dummy import Pool as ThreadPool
    # pool = ThreadPool()
    # if source == 'sina':
    #    pool.map(realtimePartUpdate_from_sina, urls)
    # else:
    #    pool.map(realtimePartUpdate_from_qq, urls)
    # pool.close()
    # pool.join()


def realtime_update_from_tushare():
    import tushare as ts

    # 更新股票行情
    df = ts.get_today_all()
    for i in range(len(df)):
        if df.ix[i, 'open'] == 0:
            continue  # 停牌

        code = df.ix[i][0]
        stock = get_stock('sh' + code)

        if stock.isNull() == True or stock.type != constant.STOCKTYPE_A:
            stock = get_stock('sz' + code)
        if stock.isNull() == True:
            continue

        record = KRecord()
        record.open = df.ix[i, 'open']
        record.high = df.ix[i, 'high']
        record.lowe = df.ix[i, 'low']
        record.close = df.ix[i, 'trade']
        record.amount = float(df.ix[i, 'amount'])
        record.volume = float(df.ix[i, 'volume'])

        from datetime import date
        d = date.today()
        record.date = Datetime(d)
        stock.realtime_update(record)

    # 更新指数行情
    df = ts.get_index()
    for i in range(len(df)):
        code = df.ix[i][0]
        stock = get_stock('sh' + code)
        if stock.isNull() == True or stock.type != constant.STOCKTYPE_INDEX:
            stock = get_stock('sz' + code)
        if stock.isNull() == True:
            continue

        total = stock.getCount(Query.DAY)
        if total == 0:
            continue

        last_record = stock.getKRecord(total - 1)

        record = KRecord()
        record.open = df.ix[i, 'open']
        record.high = df.ix[i, 'high']
        record.low = df.ix[i, 'low']
        record.close = df.ix[i, 'close']
        record.volume = float(df.ix[i, 'volume'])
        record.amount = float(df.ix[i, 'amount'])

        if (
            last_record.close != record.close or last_record.high != record.high or last_record.low != record.low
            or last_record.open != record.open
        ):
            from datetime import date
            d = date.today()
            record.date = Datetime(d)
            stock.realtime_update(record)


def realtime_update_inner(source='sina'):
    if source == 'sina' or source == 'qq':
        realtime_update_from_sina_qq(source)
    elif source == 'tushare':
        realtime_update_from_tushare()
    else:
        print(source, ' not support!')


def realtime_update_wrap():
    pre_update_time = None

    def realtime_update_closure(source='qq', delta=60):
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
            realtime_update_inner(source)
            pre_update_time = datetime.now()
            print("更新完毕！", pre_update_time)
        else:
            print("更新间隔小于" + str(delta) + "秒，未更新")
            print("上次更新时间: ", pre_update_time)

    return realtime_update_closure


realtime_update = realtime_update_wrap()
