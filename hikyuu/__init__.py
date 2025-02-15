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

import traceback
import sys
import pickle
import os

try:
    from .util import *
    from .extend import *
    from .indicator import *
    from .trade_manage import *
    from .trade_sys import *
    from .analysis import *
    from .hub import *
    from .draw import *
except Exception as e:
    info = sys.exc_info()
    # hku_fatal("{}: {}".format(info[0].__name__, info[1]))
    # hku_fatal(traceback.format_exc())
    print("{}: {}".format(info[0].__name__, info[1]))
    print(traceback.format_exc())
    print("""请使用 pipdeptree -p hikyuu 检查是否存在缺失的依赖包。
如果没有问题可以在 https://gitee.com/fasiondog/hikyuu 或 https://github.com/fasiondog/hikyuu 
上提交 issue，同时附上 "用户目录/.hikyuu" 下的 hikyuu_py.log 和 hikyuu.log 日志文件 """)
    raise e


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

__version__ = get_version()

sm = StockManager.instance()


class iodog:
    # Only for compatibility with old code
    @staticmethod
    def open():
        open_ostream_to_python()

    @staticmethod
    def close():
        close_ostream_to_python()


if in_interactive_session():
    hku_info("runing in interactive session")
    set_python_in_interactive(True)


# 如果是在 jupyter 环境中运行，重定向C++ stdout/stderr输出至python
if in_ipython_frontend():
    set_python_in_jupyter(True)
    hku_info("running in jupyter")
    iodog.open()


use_draw_engine('matplotlib')


def hku_save(var, filename):
    """
    序列化，将hikyuu内建类型的变量（如Stock、TradeManager等）保存在指定的文件中，格式为XML。

    :param var: hikyuu内建类型的变量
    :param str filename: 指定的文件名
    """
    with open(filename, 'wb') as f:
        pickle.dump(var, f)


def hku_load(filename):
    """
    将通过 hku_save 保存的变量，读取到var中。

    :param str filename: 待载入的序列化文件。
    :return: 之前被序列化保存的文件    
    """
    with open(filename, 'rb') as f:
        out = pickle.load(f)
    return out


def set_global_context(stk, query):
    """
    设置全局的 K 线上下文

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
    """
    获取当前的全局 K 线上下文

    :rtype: KData
    """
    return C.getContext()


# 预定义block变量的只有load_hikyuu后，才会生效
blocka = None  # 全部A股
zsbk_a = None  # 指数板块A股
blocksh = None  # 全部沪市
zsbk_sh = None  # 指数板块上证
blocksz = None
zsbk_sz = None
blockbj = None
zsbk_bj = None
blockg = None
zsbk_cyb = None
blockstart = None
blockzxb = None
zsbk_zxb = None
zsbk_sh50 = None
zsbk_sh180 = None
zsbk_hs300 = None
zsbk_zz100 = None


def load_hikyuu(**kwargs):
    """
    初始化加载 hikyuu 数据库，并初始化全局变量。

    示例:

    # 仅预加载 sh000001 日线数据，关闭行情接收
    options = {
        "stock_list": ["sh000001"],
        "ktype_list": ["day"],
        "load_history_finance": False,
        "load_weight": False,
        "start_spot": False,
        "spot_worker_num": 1,
    }
    load_hikyuu(**options)

    参数:
        config_file (str): 配置文件路径，默认为 ~/.hikyuu/hikyuu.ini

        stock_list (list): 指定加载的股票列表，默认为全部A股('all'), 如：['sh000001', 'sz399001']
        ktype_list (list): 指定加载的K线类型列表，默认按配置文件设置加载. 如: ['day', 'week', 'month']
                    支持的K线类型有:
                    'day', 'week', 'month', 'quarter', 'halfyear', 'year', 'min', 'min5',
                    'min15', 'min30', 'min60', 'hour2'
        load_history_finance (boolean): 预加载历史财务数至内存，默认为 True
        load_weight (boolean): 加载权息数据，默认为 True

        start_spot (boolean): 启动行情接收，默认为 True
        spot_worker_num (int): 行情接收数据处理线程数，默认为 1
    """
    if 'config_file' in kwargs:
        config_file = kwargs['config_file']
        if not os.path.exists(config_file):
            hku_fatal("配置文件不存在: {}".format(config_file))
            return
    else:
        from .data.hku_config_template import generate_default_config
        config_file = os.path.expanduser('~') + "/.hikyuu/hikyuu.ini"
        if not os.path.exists(config_file):
            # 创建默认配置
            hku_info("创建默认配置文件")
            generate_default_config()

    import configparser
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
    # 兼容原有通过环境变量设置加载选项的方式（优先级低）
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

    # 优先使用传入参数作为加载上下文
    if 'stock_list' in kwargs:
        context.stock_list = kwargs['stock_list']
    if 'ktype_list' in os.environ:
        context.ktype_list = kwargs['ktype_list']
    if 'load_history_finance' in kwargs:
        hku_param.set("load_history_finance", kwargs['load_history_finance'])
    if 'load_weight' in kwargs:
        hku_param.set("load_stock_weight", kwargs['load_weight'])

    sm.init(base_param, block_param, kdata_param, preload_param, hku_param, context)

    start_spot = False
    if 'HKU_START_SPOT' in os.environ:
        spot_str = os.environ['HKU_START_SPOT'].upper()
        start_spot = spot_str in ('1', 'TRUE')
    spot_worker_num = 1
    if 'HKU_SPOT_WORKER_NUM' in os.environ:
        spot_worker_num = int(os.environ['HKU_SPOT_WORKER_NUM'])

    if 'start_spot' in kwargs:
        start_spot = kwargs['start_spot']
    if 'spot_worker_num' in kwargs:
        spot_worker_num = kwargs['spot_worker_num']

    # 启动行情接收代理
    if start_spot:
        start_spot_agent(False, spot_worker_num)

    global blocka
    global zsbk_a
    global blocksh
    global zsbk_sh
    global blocksz
    global zsbk_sz
    global blockbj
    global zsbk_bj
    global blockg
    global zsbk_cyb
    global blockstart
    global blockzxb
    global zsbk_zxb
    global zsbk_sh50
    global zsbk_sh180
    global zsbk_hs300
    global zsbk_zz100

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

    zsbk_sh50 = sm.get_block("指数板块", "上证50")
    zsbk_sh180 = sm.get_block("指数板块", "上证180")
    zsbk_hs300 = sm.get_block("指数板块", "沪深300")
    zsbk_zz100 = sm.get_block("指数板块", "中证100")

    set_global_context(sm['sh000001'], Query(-150))


# ==============================================================================
#
# 设置关键类型简称
#
# ==============================================================================
O = OPEN()
C = CLOSE()
H = HIGH()
L = LOW()
A = AMO()
V = VOL()
D = Datetime
K = None
Q = Query

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
    x = select(C / REF(C, 1) - 1 >= 0.0995)

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

        k = s.get_kdata(q)
        if len(k) == 0 or k[-1].datetime != d[-1]:
            continue

        cond.set_context(k)
        if len(cond) > 0 and not isnan(cond[-1]) and cond[-1] > 0:
            result.append(s)
            if print_out:
                print(d[-1], s)

    return result


def select2(inds, start=Datetime(201801010000), end=Datetime.now(), stks=None):
    """导出最后时刻指定证券的所有指定指标值

    如：
        select2([CLOSE(), VOLUME()], stks=blocka)

    返回一个DataFrame, 列名是指标名称, 行是证券代码和证券名称:

        证券代码  证券名称  CLOSE  VOLUME
        SH600000 浦发银行  14.09   1000
        SH600001 中国平安  13.09   2000
        SZ000001 平安银行  13.09   3000
        ...

    :param Indicator inds: 指标列表
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期（不包括该日期）
    :param list stks: 指定的证券列表
    :rtype: pandas.DataFrame
    """
    q = Query(start, end)
    d = sm.get_trading_calendar(q, 'SH')
    if len(d) == 0:
        return

    if stks is None:
        stks = sm

    ind_cols = {'证券代码': [], '证券名称': []}
    for ind in inds:
        ind_cols[ind.name] = []

    for s in stks:
        if not s.valid:
            continue

        k = s.get_kdata(q)
        if len(k) == 0 or k[-1].datetime != d[-1]:
            continue

        ind_cols['证券代码'].append(s.market_code)
        ind_cols['证券名称'].append(s.name)
        for ind in inds:
            x = ind(k)
            ind_cols[ind.name].append(x[-1])

    df = pd.DataFrame(ind_cols)
    return df


# ==============================================================================
#
# 增加临时的实时数据更新函数 realtime_update
#
# ==============================================================================


def realtime_update_inner(source='qq', stk_list=None):
    if stk_list is None:
        if source == 'qmt':
            stk_list = [s for s in sm if s.valid and s.type in (
                constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_ETF,
                constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)]
        else:
            stk_list = [
                stk.market_code.lower() for stk in sm if stk.valid and stk.type in
                (constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_GEM,
                 constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)
            ]

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
