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
import threading

import hikyuu.flat as fb

from hikyuu.util import *
from hikyuu.fetcher.stock.zh_stock_a_sina_qq import get_spot
from hikyuu import hikyuu_init, StockManager, constant, Datetime, TimeDelta


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
    fb.SpotAddVolume(builder, record['volume'])
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

    if flatbuffers.__version__ >= '2.0':
        inv = builder.EndVector()
    else:
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
    print(spot.volume())
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


@hku_catch()
def parse_phase(phase):
    return [TimeDelta(hours=int(a[0]), minutes=int(a[1])) for a in [x.split(':') for x in phase.split('-')]]


def next_delta(start_time, interval, phase1_delta, phase2_delta, ignore_weekend):
    interval_delta = TimeDelta(seconds=interval)
    phase1_start_delta, phase1_end_delta = phase1_delta
    phase2_start_delta, phase2_end_delta = phase2_delta
    today = Datetime.today()
    phase1_start, phase1_end = today + phase1_start_delta, today + phase1_end_delta
    phase2_start, phase2_end = today + phase2_start_delta, today + phase2_end_delta

    current_time = Datetime.now()
    maybe_time = current_time + interval_delta
    if phase1_start_delta == phase1_end_delta and phase2_start_delta == phase2_end_delta:
        # 两个时间周期各自的起止时间相等，则认为未做限制
        delta = interval_delta - (current_time - start_time)
    elif maybe_time > phase2_end:
        # 如果预计的下一次时间大于周期2的结束日期, 则取下一日的周期1起始时间计算
        next_time = today + TimeDelta(1) + phase1_start_delta
        if ignore_weekend and next_time.day_of_week() in (0, 6):
            next_time = today.next_week() + phase1_start_delta
        delta = next_time - start_time
    elif maybe_time in (phase1_start, phase1_end, phase2_start, phase2_end):
        # 如果下一次时间刚好等于时间周期的起止点，则直接返回预计的时间间隔
        delta = interval_delta
    elif start_time < phase1_start:
        # 如果本次的时间小于周期1的起始时间，则取phase1起始时间计算
        delta = phase1_start - start_time
    elif phase1_end < maybe_time < phase2_start:
        delta = phase2_start - start_time
    else:
        delta = interval_delta - (current_time - start_time)
    return delta


# 多线程时锁住申请
g_nng_sender_lock = threading.Lock()
g_nng_ipc_sender = None
g_nng_tcp_sender = None
g_spot_topic = ':spot:'
g_ipc_addr = 'ipc:///tmp/hikyuu_real.ipc'
g_tcp_addr = 'tcp://*:9200'


@hku_catch(trace=True)
def get_nng_ipc_sender():
    global g_nng_ipc_sender
    if g_nng_ipc_sender is None:
        g_nng_ipc_sender = pynng.Pub0()
        g_nng_ipc_sender.listen(g_ipc_addr)
    return g_nng_ipc_sender


@hku_catch(trace=True)
def get_nng_tcp_sender():
    global g_nng_tcp_sender
    if g_nng_tcp_sender is None:
        g_nng_tcp_sender = pynng.Pub0()
        g_nng_tcp_sender.listen(g_tcp_addr)
    return g_nng_tcp_sender


def get_nng_senders():
    g_nng_sender_lock.acquire()
    ipc_sender = get_nng_ipc_sender()
    tcp_sender = get_nng_tcp_sender()
    g_nng_sender_lock.release()
    return ipc_sender, tcp_sender


def release_nng_senders():
    global g_nng_ipc_sender, g_nng_tcp_sender
    g_nng_sender_lock.acquire()
    if g_nng_ipc_sender is not None:
        g_nng_ipc_sender.close()
        g_nng_ipc_sender = None
    if g_nng_tcp_sender is not None:
        g_nng_tcp_sender.close()
        g_nng_tcp_sender = None
    g_nng_sender_lock.release()
    hku_info("release pynng socket")


def start_send_spot():
    senders = get_nng_senders()
    for sender in senders:
        sender.send("{}{}".format(g_spot_topic, '[start spot]').encode('utf-8'))


def end_send_spot():
    senders = get_nng_senders()
    for sender in senders:
        sender.send('{}{}'.format(g_spot_topic, '[end spot]').encode('utf-8'))


def send_spot(records):
    spot = bytearray(g_spot_topic.encode('utf-8'))
    buf = create_fb_spot(records)
    spot.extend(buf)
    senders = get_nng_senders()
    for sender in senders:
        sender.send(bytes(spot))


def collect(server, use_proxy, source, seconds, phase1, phase2, ignore_weekend):
    global g_tcp_addr
    if len(server) >= 3 and server[:3] == 'tcp':
        g_tcp_addr = server

    phase1_delta = parse_phase(phase1)
    if phase1_delta is None or len(phase1_delta) != 2:
        hku_error("无效参数 phase1: {}".format(phase1))
        exit(1)
    if phase1_delta[0] > phase1_delta[1]:
        hku_error("无效参数 phase1: {}, 结束时间应大于等于起始时间".format(phase1))
        exit(1)

    phase2_delta = parse_phase(phase2)
    if phase2_delta is None or len(phase2_delta) != 2:
        hku_error("无效参数 phase2: {}".format(phase2))
        exit(1)
    if phase2_delta[0] > phase2_delta[1]:
        hku_error("无效参数 phase2: {}, 结束时间应大于等于起始时间".format(phase2))
        exit(1)
    if phase1_delta[1] > phase2_delta[0]:
        hku_error("无效参数 phase1: {}, phase2: {}, phase2 起始时间应大于等于 phase1 结束时间".format(phase1, phase2))
        exit(1)

    hku_logger.info("采集时间段1：{}".format(phase1))
    hku_logger.info("采集时间段2：{}".format(phase2))

    config_file = os.path.expanduser('~') + '/.hikyuu/hikyuu.ini'
    if not os.path.exists(config_file):
        print("未找到配置文件，请先运行 HikyuuTDX 进行配置与数据导入")
        exit(1)

    hikyuu_init(config_file, ignore_preload=True)

    sm = StockManager.instance()
    stk_list = [
        stk.market_code.lower() for stk in sm if stk.valid and stk.type in
        (constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_GEM,
         constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)
    ]

    _ = get_nng_senders()

    start_time = Datetime.now()
    delta = next_delta(start_time, seconds, phase1_delta, phase2_delta, ignore_weekend)
    next_time = start_time + delta
    hku_info("启动采集时间：{}".format(next_time))
    time.sleep(delta.total_seconds())
    while True:
        try:
            start_time = Datetime.now()
            start_send_spot()
            records = get_spot(stk_list, source, use_proxy, send_spot)
            hku_info("{}:{}:{} 采集数量: {}".format(start_time.hour, start_time.minute, start_time.second, len(records)))
            # pub_sock.send('{}{}'.format(spot_topic, '[end spot]').encode('utf-8'))
            end_send_spot()
            delta = next_delta(start_time, seconds, phase1_delta, phase2_delta, ignore_weekend)
            hku_info("sleep {}'s".format(delta.total_seconds()))
            if delta.total_seconds() > 0:
                time.sleep(delta.total_seconds())
            else:
                pass
        except KeyboardInterrupt:
            print("Ctrl-C 终止")
            break
        except Exception as e:
            hku_error(e)
            time.sleep(10)
    release_nng_senders()


@click.command()
@click.option('-server', '--server', default='tcp://*:9200')
@click.option('-use_proxy', '--use_proxy', is_flag=True, help='是否使用代理，须自行申请芝麻http代理并加入ip白名单')
@click.option('-source', '--source', default='qq', type=click.Choice(['sina', 'qq']), help='数据来源')
@click.option('-seconds', '--seconds', default=10)
@click.option('-phase1', '--phase1', default='9:00-12:00')
@click.option('-phase2', '--phase2', default='13:00-15:00')
@click.option('-ignore_weekend', '--ignore_weekend', is_flag=True)
def run(server, use_proxy, source, seconds, phase1, phase2, ignore_weekend):
    collect(server, use_proxy, source, seconds, phase1, phase2, ignore_weekend)


if __name__ == '__main__':
    try:
        print("采集程序运行中，可使用 Ctrl-C 终止！")
        run()
    except KeyboardInterrupt:
        exit(1)
