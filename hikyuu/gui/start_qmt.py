#!/usr/bin/python3
# -*- coding: utf-8 -*-


from hikyuu.fetcher.stock.zh_stock_a_qmt import parse_one_result_qmt
from hikyuu.gui.spot_server import release_nng_senders, start_send_spot, end_send_spot, send_spot


def callback(datas):
    records = []
    for stock_code, data in datas.items():
        # print(stock_code, data)
        records.append(parse_one_result_qmt(stock_code, data))

    if records:
        start_send_spot()
        send_spot(records)
        end_send_spot()


if __name__ == "__main__":
    from hikyuu.interactive import *
    from xtquant import xtdata

    code_list = [f'{s.code}.{s.market}' for s in sm if s.valid]
    # code_list = ['000001.SZ']

    xtdata.subscribe_whole_quote(code_list, callback)

    try:
        xtdata.run()
    except Exception as e:
        hku_error(e)
    finally:
        # 退出释放资源
        release_nng_senders()
