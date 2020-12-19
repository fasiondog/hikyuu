# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2020 hikyuu.org
#
# Create on: 2020-12-13
#    Author: fasiondog

import os
import time
import flatbuffers
import pynng
import click

import hikyuu.flat as fb

from hikyuu.util import *
from hikyuu.fetcher.stock.zh_stock_a_sina_qq import get_spot_parallel
from hikyuu import hikyuu_init, StockManager, constant


def create_fb_spot_record(builder, record):
    market = builder.CreateString(record['market'])
    code = builder.CreateString(record['code'])
    name = builder.CreateString(record['name'])
    datetime = builder.CreateString(str(record['datetime']))

    fb.SpotStart(builder)
    fb.SpotAddMarket(builder, market)
    fb.SpotAddCode(builder, code)
    fb.SpotAddName(builder, name)
    fb.SpotAddDatetime(builder, datetime)
    fb.SpotAddYesterdayClose(builder, record['yesterday_close'])
    fb.SpotAddOpen(builder, record['open'])
    fb.SpotAddHigh(builder, record['high'])
    fb.SpotAddLow(builder, record['low'])
    fb.SpotAddClose(builder, record['close'])
    fb.SpotAddAmount(builder, record['amount'])
    fb.SpotAddVolumn(builder, record['volumn'])
    fb.SpotAddBid1(builder, record['bid1'])
    fb.SpotAddBid1Amount(builder, record['bid1_amount'])
    fb.SpotAddBid2(builder, record['bid2'])
    fb.SpotAddBid2Amount(builder, record['bid2_amount'])
    fb.SpotAddBid3(builder, record['bid3'])
    fb.SpotAddBid3Amount(builder, record['bid3_amount'])
    fb.SpotAddBid4(builder, record['bid4'])
    fb.SpotAddBid4Amount(builder, record['bid4_amount'])
    fb.SpotAddBid5(builder, record['bid5'])
    fb.SpotAddBid5Amount(builder, record['bid5_amount'])
    fb.SpotAddAsk1(builder, record['ask1'])
    fb.SpotAddAsk1Amount(builder, record['ask1_amount'])
    fb.SpotAddAsk2(builder, record['ask2'])
    fb.SpotAddAsk2Amount(builder, record['ask2_amount'])
    fb.SpotAddAsk3(builder, record['ask3'])
    fb.SpotAddAsk3Amount(builder, record['ask3_amount'])
    fb.SpotAddAsk4(builder, record['ask4'])
    fb.SpotAddAsk4Amount(builder, record['ask4_amount'])
    fb.SpotAddAsk5(builder, record['ask5'])
    fb.SpotAddAsk5Amount(builder, record['ask5_amount'])
    return fb.SpotEnd(builder)


def create_fb_spot(records):
    builder = flatbuffers.Builder(0)
    end = None
    spot_list = []
    for record in records:
        spot_list.append(create_fb_spot_record(builder, record))
    total = len(spot_list)
    fb.SpotListStartSpotVector(builder, total)
    for spot in spot_list:
        builder.PrependUOffsetTRelative(spot)
    inv = builder.EndVector(total)

    fb.SpotListStart(builder)
    fb.SpotListAddSpot(builder, inv)
    end = fb.SpotListEnd(builder)

    builder.Finish(end)
    return builder.Output()


def print_spot(spot):
    print(spot.Market().decode())
    print(spot.Code().decode())
    print(spot.Name().decode())
    print(spot.Datetime().decode())
    print(spot.YesterdayClose())
    print(spot.Open())
    print(spot.High())
    print(spot.Low())
    print(spot.Close())
    print(spot.Amount())
    print(spot.Volumn())
    print(spot.Bid1())
    print(spot.Bid1Amount())
    print(spot.Bid2())
    print(spot.Bid2Amount())
    print(spot.Bid3())
    print(spot.Bid3Amount())
    print(spot.Bid4())
    print(spot.Bid4Amount())
    print(spot.Bid5())
    print(spot.Bid5Amount())
    print(spot.Ask1())
    print(spot.Ask1Amount())
    print(spot.Ask2())
    print(spot.Ask2Amount())
    print(spot.Ask3())
    print(spot.Ask3Amount())
    print(spot.Ask4())
    print(spot.Ask4Amount())
    print(spot.Ask5())
    print(spot.Ask5Amount())


def print_spot_list(buf):
    spot_list = fb.SpotList.GetRootAsSpotList(buf, 0)
    print(spot_list.SpotLength())
    for i in range(spot_list.SpotLength()):
        print_spot(spot_list.Spot(i))


@click.command()
@click.option('-use_proxy', '--use_proxy', is_flag=True, help='是否使用代理，须自行申请芝麻http代理并加入ip白名单')
@click.option('-source', '--source', default='sina', type=click.Choice(['sina', 'qq']), help='数据来源')
def run(use_proxy, source):
    config_file = os.path.expanduser('~') + '/.hikyuu/hikyuu.ini'
    if not os.path.exists(config_file):
        print("未找到配置文件，请先运行 HikyuuTDX 进行配置与数据导入")
        exit(1)

    hikyuu_init(config_file, ignore_preload=True)

    print("采集程序运行中，可使用 Ctrl-C 终止！")

    sm = StockManager.instance()
    stk_list = [
        stk.market_code.lower() for stk in sm if stk.valid and stk.type in
        (constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_GEM)
    ]

    def batch_func(records):
        spot = bytearray(b'spot')
        buf = create_fb_spot(records)
        spot.extend(buf)
        pub_sock.send(bytes(spot))

    address = "ipc:///tmp/hikyuu_real_pub.ipc"
    pub_sock = pynng.Pub0()
    pub_sock.listen(address)
    try:
        while True:
            records = get_spot_parallel(stk_list, source, use_proxy, batch_func)
            print(len(records))
            time.sleep(10)
    except KeyboardInterrupt:
        exit(1)


if __name__ == '__main__':
    run()