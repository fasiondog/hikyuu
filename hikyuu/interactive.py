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
import os
import configparser

from hikyuu.data.hku_config_template import generate_default_config
from hikyuu import *


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

context = StrategyContext(["all"])
if 'HKU_STOCK_LIST' in os.environ:
    context.stock_list = os.environ['HKU_STOCK_LIST'].split(";")
if 'HKU_KTYPE_LIST' in os.environ:
    context.ktype_list = os.environ['HKU_KTYPE_LIST'].split(";")
if 'HKU_LOAD_HISTORY_FINANCE' in os.environ:
    load_str = os.environ['HKU_LOAD_HISTORY_FINANCE'].upper()
    load_finance = load_str in ("1", "TRUE")
    hku_param.set("load_history_finance", load_finance)
if 'HKU_LOAD_STOCK_WEIGHT' in os.environ:
    load_str = os.environ['HKU_LOAD_STOCK_WEIGHT'].upper()
    load_stk_weight = load_str in ("1", "TRUE")
    hku_param.set("load_stock_weight", load_stk_weight)

sm.init(base_param, block_param, kdata_param, preload_param, hku_param, context)
# set_log_level(LOG_LEVEL.INFO)

start_spot = False
if 'HKU_START_SPOT' in os.environ:
    spot_str = os.environ['HKU_START_SPOT'].upper()
    start_spot = spot_str in ('1', 'TRUE')
spot_worker_num = os.cpu_count()
if 'HKU_SPOT_WORKER_NUM' in os.environ:
    spot_worker_num = int(os.environ['HKU_SPOT_WORKER_NUM'])

# 启动行情接收代理
start_spot_agent(False, spot_worker_num)

# ==============================================================================
#
# 引入blocka、blocksh、blocksz、blockg全局变量，便于交互式环境下使用
#
# ==============================================================================
blocka = Block("A", "ALL")
for s in sm:
    if s.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_A_BJ):
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


def realtime_update_inner(source='qq', stk_list=None):
    if stk_list is None:
        stk_list = sm
    if source == 'qq':
        from hikyuu.fetcher.stock.zh_stock_a_sina_qq import get_spot
        stk_list = [s.market_code.lower() for s in stk_list]
        records = get_spot(stk_list, 'qq')
    elif source == 'qmt':
        from hikyuu.fetcher.stock.zh_stock_a_qmt import get_spot
        records = get_spot(stk_list)
    else:
        hku_error(f'Not support website source: {source}!')
        return

    for r in records:
        stk = sm[f'{r["market"]}{r["code"]}']
        if stk.is_null():
            continue
        k = KRecord()
        k.datetime = Datetime(r['datetime']).start_of_day()
        k.open = r['open']
        k.high = r['high']
        k.low = r['low']
        k.close = r['close']
        k.volume = r['volume']
        k.amount = r['amount']
        stk.realtime_update(k)


def realtime_update_wrap():
    pre_update_time = None

    def realtime_update_closure(source='qq', delta=60, stk_list=None):
        """
        更新实时日线数据
        :param str source: 数据源 ('qq' | 'qmt')
        :param int delta: 最小更新间隔时间, 防止更新过于频繁
        :param sequence stk_list: 待更新的stock列表, 如为 None 则更新全部
        """
        from datetime import timedelta, datetime
        nonlocal pre_update_time
        now_update_time = datetime.now()
        if (source == 'qmt') or (pre_update_time is None) or (now_update_time - pre_update_time) > timedelta(0, delta, 0):
            realtime_update_inner(source, stk_list)
            pre_update_time = datetime.now()
            print(f"更新完毕！更新时间: {pre_update_time}")
        else:
            print(f"更新间隔小于 {str(delta)} 秒，未更新")
            print(f"上次更新时间: {pre_update_time}")

    return realtime_update_closure


realtime_update = realtime_update_wrap()
