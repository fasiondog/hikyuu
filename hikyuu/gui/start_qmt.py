#!/usr/bin/python3
# -*- coding: utf-8 -*-


from hikyuu.fetcher.stock.zh_stock_a_qmt import parse_one_result_qmt, get_spot
from hikyuu.gui.spot_server import release_nng_senders, start_send_spot, end_send_spot, send_spot


def callback(datas):
    records = []
    for stock_code, data in datas.items():
        records.append(parse_one_result_qmt(stock_code, data))
    # print(len(records))

    if records:
        start_send_spot()
        send_spot(records)
        end_send_spot()


if __name__ == "__main__":
    import os
    import configparser

    from hikyuu.data.hku_config_template import generate_default_config
    from hikyuu import *

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

    # 不加载历史财务信息及权息数据
    hku_param["load_history_finance"] = False
    hku_param["load_stock_weight"] = False

    base_param = Parameter()
    base_info_config = ini.options('baseinfo')
    for p in base_info_config:
        base_param[p] = ini.get('baseinfo', p)

    block_param = Parameter()
    block_config = ini.options('block')
    for p in block_config:
        block_param[p] = ini.get('block', p)

    # 不使用配置文件中的预加载参数
    preload_param = Parameter()
    if p in Query.get_all_ktype():
        preload_param[p] = True
        minute = Query.get_ktype_in_min()
        if minute >= 240:
            preload_param[f'{p.lower()}_max'] = 1
        else:
            preload_param[f'{p.lower()}_max'] = 240 // minute

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

    hku_info("waiting all data loaded ...")
    while not sm.data_ready:
        import time
        time.sleep(100)

    def get_full():
        stk_list = [s for s in sm if s.valid and s.type in (
            constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_ETF,
            constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)]
        start_send_spot()
        records = get_spot(stk_list, None, None, send_spot)
        end_send_spot()

    def timer_func():
        import threading
        today = Datetime.today()
        if today.day_of_week() not in (0, 6) and not sm.is_holiday(today):
            get_full()
        tomorrow = Datetime.today().next_day()
        delta = Datetime.today().next_day() + TimeDelta(0, 9, 30) - Datetime.now()
        timer = threading.Timer(delta.total_seconds(), timer_func)
        timer.start()
        return timer

    # 每日9:30先获取一次当天全部数据，以便生成分钟级别数据
    # 后续订阅更新因为只更新存在变化的数据，内部分钟级别数据可能时不连续的（如果分钟内不存在变化, 不会触发hikyuu更新）
    timer = timer_func()

    hku_info("start xtquant")
    code_list = [f'{s.code}.{s.market}' for s in sm if s.valid and s.type in (
        constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_ETF,
        constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)]
    from xtquant import xtdata
    xtdata.subscribe_whole_quote(code_list, callback)

    try:
        xtdata.run()
    except Exception as e:
        hku_error(e)
    finally:
        # 退出释放资源
        release_nng_senders()
