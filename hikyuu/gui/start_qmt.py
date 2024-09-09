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
        preload_param[p] = False

    kdata_param = Parameter()
    kdata_config = ini.options('kdata')
    for p in kdata_config:
        if p == "convert":
            kdata_param[p] = ini.getboolean('kdata', p)
            continue
        kdata_param[p] = ini.get('kdata', p)

    context = StrategyContext(["all"])
    context.ktype_list = ["day"]

    sm.init(base_param, block_param, kdata_param, preload_param, hku_param, context)

    # 后续希望每次先主动获取一次全部的tick, 这里需要等待所有数据加载完毕，以便保证全部证券收到第一次tick通知
    hku_info("waiting all data loaded ...")
    while not sm.data_ready:
        import time
        time.sleep(100)

    stk_list = [s for s in sm if s.valid and s.type in (
        constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_ETF,
        constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)]

    hku_info("start xtquant")
    code_list = [f'{s.code}.{s.market}' for s in stk_list]
    from xtquant import xtdata
    xtdata.subscribe_whole_quote(code_list, callback)

    # 每日 9:30 时，主动读取行情一次，以便 hikyuu 生成当日首个分钟线
    while True:
        try:
            today = Datetime.today()
            if today.day_of_week() not in (0, 6) and not sm.is_holiday(today):
                hku_info("get full tick ...")
                start_send_spot()
                records = get_spot(stk_list, None, None, send_spot)
                end_send_spot()
            now = Datetime.now()
            today_open = today + TimeDelta(0, 9, 30)
            if now < today_open:
                delta = today_open - Datetime.now()
            else:
                delta = today_open + Days(1) - Datetime.now()
            hku_info(f"start timer: {delta}s")
            time.sleep(delta.total_seconds())
        except KeyboardInterrupt:
            print("Ctrl-C 终止")
            break
        except Exception as e:
            hku_error(e)
            time.sleep(10)

    release_nng_senders()

    # try:
    #     xtdata.run()
    # except Exception as e:
    #     hku_error(e)
    # finally:
    #     # 退出释放资源
    #     release_nng_senders()
    #     exit(0)
