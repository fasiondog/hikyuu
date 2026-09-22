#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import atexit
import time
import locale
from pathlib import Path
import pickle
import traceback
__copyright__ = """
Apache License Version 2.0

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

import sys
import os
BASE_DIR = os.path.dirname(__file__)


if sys.platform == 'win32':
    # add_dll_directory() sometimes does not take effect
    os.add_dll_directory(os.path.join(os.path.dirname(__file__), 'cpp'))
    # Add the directory path where the dynamic library is located
    current_path = os.environ.get('PATH', '')
    dll_directory = os.path.join(BASE_DIR, 'cpp')
    new_path = f"{dll_directory};{current_path}"
    os.environ['PATH'] = new_path
else:
    current_path = os.environ.get('LD_LIBRARY_PATH', '')
    dll_directory = os.path.join(BASE_DIR, 'cpp')
    new_path = f"{dll_directory}:{current_path}" if current_path else dll_directory
    os.environ['LD_LIBRARY_PATH'] = new_path


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
    print("""Please use pipdeptree -p hikyuu to check whether any dependency package is missing.
If everything is fine, you can submit an issue at https://gitee.com/fasiondog/hikyuu or
https://github.com/fasiondog/hikyuu, attaching the hikyuu_py.log and hikyuu.log files under
"~/.hikyuu" """)
    raise e


__version__ = get_version()


sm = StockManager.instance()


def hku_cleanup():
    # On macosx, clickhouse must wait for exit in python; there is no problem on other platforms
    local_sm = StockManager.instance()
    local_sm.cancel_load()
    while not local_sm.data_ready:
        time.sleep(0.1)

    # Release all K-line data, to prevent the python-implemented extended K-line conversion function from raising an exception at exit due to the GIL
    release_extra_ktype()


atexit.register(hku_cleanup)


# Try to find the hikyuu_plugin python package and get its directory
try:
    import hikyuu_plugin
    plugin_path = os.path.dirname(hikyuu_plugin.__file__)
    plugin_version = hikyuu_plugin.__version__

    # Get the major version (in the format x.x.x)
    def get_major_version(version_str):
        parts = version_str.split('.')
        return '.'.join(parts[:3]) if len(parts) >= 3 else version_str

    # Compare the versions of hikyuu and hikyuu_plugin
    hikyuu_major_version = get_major_version(__version__)
    plugin_major_version = get_major_version(plugin_version)

    if hikyuu_major_version != plugin_major_version:
        hku_warn("The hikyuu version ({}) does not match the hikyuu_plugin version ({}), please make sure to use compatible versions!".format(__version__, plugin_version))

except ImportError:
    plugin_path = Path.home() / '.hikyuu' / 'plugin'
    if plugin_path.exists():
        plugin_path = str(plugin_path)
    else:
        plugin_path = os.path.join(os.path.dirname(__file__), 'plugin')

try:
    from hikyuu_plugin_private import *
except ImportError:
    pass


sm.set_plugin_path(plugin_path)
# print(f"current plugin path: {plugin_path}")

sm.set_language_path(f'{BASE_DIR}/cpp/i18n')


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


# If running in a jupyter environment, redirect the C++ stdout/stderr output to python
if in_ipython_frontend():
    set_python_in_jupyter(True)
    hku_info("running in jupyter")
    iodog.open()

# When executed in a windows terminal, the C++ output cannot display Chinese properly
if sys.platform == 'win32' and not in_ipython_frontend():
    lang, _ = locale.getdefaultlocale()
    if lang and 'zh' in lang.lower():
        os.system('chcp 65001 > nul')

use_draw_engine('matplotlib')


def hku_save(var, filename):
    """
    Serialization: save a variable of a hikyuu built-in type (such as Stock, TradeManager, etc.)
    to the specified file in binary format.

    :param var: a variable of a hikyuu built-in type
    :param str filename: the specified file name
    """
    with open(filename, 'wb') as f:
        pickle.dump(var, f)


def hku_load(filename):
    """
    Read the variable saved by hku_save into var.

    :param str filename: the serialization file to be loaded.
    :return: the variable previously saved by serialization
    """
    with open(filename, 'rb') as f:
        out = pickle.load(f)
    return out


def set_global_context(stk, query):
    """
    Set the global K-line context

    :param Stock stk: the specified global Stock
    :param Query query: the specified query condition
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
    Get the current global K-line context

    :rtype: KData
    """
    return C.getContext()


# The predefined block variables take effect only after load_hikyuu
blocka = None  # All A-share
blocka_shsz = None  # Shanghai-Shenzhen A-share
zsbk_a = None  # Index block A-share
blocksh = None  # All Shanghai market
zsbk_sh = None  # Index block Shanghai
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
blocketf = None


def load_hikyuu(**kwargs):
    """
    Initialize and load the hikyuu database, and initialize the global variables.

    Example:

    # Only preload the sh000001 daily data, with quote receiving disabled
    options = {
        "stock_list": ["sh000001"],
        "ktype_list": ["day"],
        "preload_num": {"day_max": 100000},
        "load_history_finance": False,
        "load_weight": False,
        "start_spot": False,
        "spot_worker_num": 1,
        "reload_time": "00:00",
        "lazy_preload": False,
    }
    load_hikyuu(**options)

    Parameters:
        config_file (str): the configuration file path, defaults to ~/.hikyuu/hikyuu.ini

        stock_list (list): the stocks to load, defaults to all A-shares ('all'), e.g. ['sh000001', 'sz399001']
        ktype_list (list): the K-line types to load, defaults to the configuration file settings. e.g. ['day', 'week', 'month']
                    Supported K-line types:
                    'day', 'week', 'month', 'quarter', 'halfyear', 'year', 'min', 'min5',
                    'min15', 'min30', 'min60', 'hour2', 'timeline', 'trans'
        preload_num (dict): {'day_max': 100000, 'week_max': 100000, 'month_max': 100000, ...}
        load_history_finance (boolean): preload the historical finance data into memory, defaults to True
        load_weight (boolean): load the weight data, defaults to True

        start_spot (boolean): start quote receiving, defaults to True
        spot_worker_num (int): the number of quote data processing threads, defaults to 1
        reload_time (str): the data reload time (hour:minute), in the format HH:MM, defaults to 00:00
        lazy_preload (boolean): preload the levels below daily with the lazy loading mode, defaults to False

        use_shm_server (boolean): whether this process connects to an existing shm data service as a client, defaults to False.
            It runs in standalone mode by default; set it to True when a service with the same datadir already exists and the data snapshot needs to be shared, then this process will connect automatically.
        shm_server_wait_timeout (int): the total time budget (seconds) for the client connection negotiation, including the connection probing and the readiness
            waiting; 0 means waiting indefinitely, defaults to 600, and it degrades to standalone mode after the timeout.
    """
    if 'config_file' in kwargs:
        config_file = kwargs['config_file']
        if not os.path.exists(config_file):
            hku_fatal("The configuration file does not exist: {}".format(config_file))
            return
    else:
        from .data.hku_config_template import generate_default_config
        config_file = os.path.expanduser('~') + "/.hikyuu/hikyuu.ini"
        if not os.path.exists(config_file):
            # Create the default configuration
            hku_info("Creating the default configuration file")
            generate_default_config()

    import configparser
    ini = configparser.ConfigParser()
    ini.read(config_file, encoding='utf-8')
    hku_param = Parameter()
    hku_param["tmpdir"] = ini.get('hikyuu', 'tmpdir')
    hku_param["datadir"] = ini.get('hikyuu', 'datadir')
    if ini.has_option('hikyuu', 'quotation_server'):
        hku_param["quotation_server"] = ini['hikyuu']['quotation_server']
    hku_param["load_history_finance"] = ini.getboolean("hikyuu", "load_history_finance", fallback=True)
    hku_param["load_stock_weight"] = ini.getboolean("hikyuu", "load_stock_weight", fallback=True)
    if ini.has_option('hikyuu', 'plugindir'):
        hku_param["plugindir"] = ini.get('hikyuu', 'plugindir')
    else:
        hku_param["plugindir"] = os.path.join(os.path.dirname(__file__), "plugin")
    hku_param["reload_time"] = ini.get('hikyuu', 'reload_time', fallback="00:00")
    hku_param["lazy_preload"] = ini.getboolean("hikyuu", "lazy_preload", fallback=False)
    # shm data service configuration ([hikyuu] section, the defaults are used when not explicitly configured): disabled by default, the process runs in standalone mode;
    # to connect to an existing service as a client, use_shm_server=True must be explicitly enabled in the configuration or the load_hikyuu parameters
    hku_param["use_shm_server"] = ini.getboolean("hikyuu", "use_shm_server", fallback=False)
    hku_param["shm_server_wait_timeout"] = ini.getint("hikyuu", "shm_server_wait_timeout", fallback=600)

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
        if p in ('day', 'week', 'month', 'quarter', 'halfyear', 'year', 'min', 'min5', 'min15', 'min30', 'min60', 'hour2', 'timeline', 'trans'):
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
    # Compatible with the original way of setting the load options through environment variables (lower priority)
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

    # Prefer the passed-in parameters as the load context
    if 'stock_list' in kwargs:
        context.stock_list = kwargs['stock_list']
    if 'ktype_list' in kwargs:
        context.ktype_list = kwargs['ktype_list']
    if 'preload_num' in kwargs:
        context.preload_num = kwargs['preload_num']
    if 'load_history_finance' in kwargs:
        hku_param.set("load_history_finance", kwargs['load_history_finance'])
    if 'load_weight' in kwargs:
        hku_param.set("load_stock_weight", kwargs['load_weight'])
    if 'use_shm_server' in kwargs:
        hku_param.set("use_shm_server", kwargs['use_shm_server'])
    if 'shm_server_wait_timeout' in kwargs:
        hku_param.set("shm_server_wait_timeout", kwargs['shm_server_wait_timeout'])

    sm.init(base_param, block_param, kdata_param, preload_param, hku_param, context)

    # Do not start quote receiving by default, to prevent the quotes from the dataserver also being missing when starting after the market opens due to the missing self-synthesis
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

    # Start the quote receiving agent
    if start_spot:
        start_spot_agent(False, spot_worker_num)

    global blocka
    global zsbk_a
    global blocka_shsz
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
    global blocketf

    blocka = sm.get_block("A", "ALL")  # All A-share, including the Beijing Stock Exchange
    zsbk_a = blocka

    blocka_shsz = sm.get_block("A", "沪深")

    blocksh = sm.get_block("A", "SH")
    zsbk_sh = blocksh

    blocksz = sm.get_block("A", "SZ")
    zsbk_sz = blocksz

    blockbj = sm.get_block("A", "BJ")
    zsbk_bj = blockbj

    blockg = sm.get_block("G", "创业板")
    zsbk_cyb = blockg

    blockstart = sm.get_block("START", "科创板")

    blockzxb = sm.get_block("A", "中小板")
    zsbk_zxb = blockzxb

    zsbk_sh50 = sm.get_block("指数板块", "上证50")
    zsbk_sh180 = sm.get_block("指数板块", "上证180")
    zsbk_hs300 = sm.get_block("指数板块", "沪深300")
    zsbk_zz100 = sm.get_block("指数板块", "中证100")

    blocketf = sm.get_block("ETF", "ALL")

    set_global_context(sm['sh000001'], Query(-150))


# ==============================================================================
#
# Set the abbreviations of the key types
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
# A rough stock-picking function
#
# ==============================================================================


def select(cond, start=Datetime(201801010000), end=Datetime.now(), print_out=True):
    """
    Example:
    # Select the limit-up stocks
    C = CLOSE()
    x = select(C / REF(C, 1) - 1 >= 0.0995)

    :param Indicator cond: the condition indicator
    :param Datetime start: the start date
    :param Datetime end: the end date
    :param bool print_out: print the selected stocks
    :rtype: the list of the selected stocks
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
    """Export all the specified indicator values of the specified securities at the last moment

    E.g.:
        select2([CLOSE(), VOLUME()], stks=blocka)

    Return a DataFrame, the column names are the indicator names, the rows are the code and the name of the securities:

        证券代码  证券名称  CLOSE  VOLUME
        SH600000 浦发银行  14.09   1000
        SH600001 中国平安  13.09   2000
        SZ000001 平安银行  13.09   3000
        ...

    :param Indicator inds: the indicator list
    :param Datetime start: the start date
    :param Datetime end: the end date (excluding this date)
    :param list stks: the specified securities list
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
# Add the temporary realtime data update function realtime_update
#
# ==============================================================================


def realtime_update_inner(source='qq', stk_list=None):
    if stk_list is None:
        stk_list = [s for s in sm if s.valid and s.type in (
            constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_ETF,
            constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)]

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
        Update the realtime daily data
        :param str source: the data source ('qq' | 'qmt')
        :param int delta: the minimum update interval, to prevent updating too frequently
        :param sequence stk_list: the stock list to update; if None, update all
        """
        from datetime import timedelta, datetime
        nonlocal pre_update_time
        now_update_time = datetime.now()
        if (source == 'qmt') or (pre_update_time is None) or (now_update_time - pre_update_time) > timedelta(0, delta, 0):
            realtime_update_inner(source, stk_list)
            pre_update_time = datetime.now()
            print(f"Update completed! Update time: {pre_update_time}")
        else:
            print(f"The update interval is less than {str(delta)} seconds, not updated")
            print(f"Last update time: {pre_update_time}")

    return realtime_update_closure


realtime_update = realtime_update_wrap()


def auto_sync_globals(func):
    """
    Decorator: automatically synchronize the global variables to the namespace of the caller
    """
    def wrapper(*args, **kwargs):
        # Call the original function
        result = func(*args, **kwargs)

        # Synchronize the global variables automatically
        try:
            import sys
            import inspect

            # Get the call stack
            frame = inspect.currentframe()
            try:
                # Search up the call stack
                caller_frame = frame.f_back
                while caller_frame:
                    caller_globals = caller_frame.f_globals
                    # Check whether it is a global variable imported from hikyuu whose value is still None
                    var_names = ['blocka', 'blocka_shsz', 'zsbk_a', 'blocksh', 'zsbk_sh', 'blocksz', 'zsbk_sz',
                                 'blockbj', 'zsbk_bj', 'blockg', 'zsbk_cyb', 'blockstart', 'blockzxb', 'blocketf',
                                 'zsbk_zxb', 'zsbk_sh50', 'zsbk_sh180', 'zsbk_hs300', 'zsbk_zz100']

                    updated_vars = []
                    for var_name in var_names:
                        if (var_name in caller_globals and
                            caller_globals[var_name] is None and
                            var_name in globals() and
                                globals()[var_name] is not None):
                            # Update the global variables of the caller
                            caller_globals[var_name] = globals()[var_name]
                            updated_vars.append(var_name)

                    # if updated_vars:
                    #     print(f"Auto-synced global variables: {', '.join(updated_vars)}")

                    caller_frame = caller_frame.f_back
            finally:
                del frame
        except Exception:
            # Silently ignore the errors
            pass

        return result
    return wrapper


# Apply the auto-sync decorator to the load_hikyuu function
load_hikyuu = auto_sync_globals(load_hikyuu)
