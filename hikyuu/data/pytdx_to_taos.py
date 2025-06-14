#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-06-13
#    Author: fasiondog

import sys
import math
import datetime
from pytdx.hq import TDXParams

from hikyuu.util.mylog import hku_error, hku_debug

import taos

from hikyuu import Datetime, UTCOffset
from hikyuu.data.common import *
from hikyuu.data.common_pytdx import to_pytdx_market, pytdx_get_day_trans
from hikyuu.data.common_taos import (
    create_database,
    get_marketid,
    get_codepre_list,
    get_stock_list,
    get_table,
    get_lastdatetime,
    get_last_krecord,
    update_extern_data,
)


@hku_catch(ret=0, trace=True)
def import_index_name(connect):
    """
    导入所有指数代码表

    :param connect: taos 连接对象
    :return: 指数个数
    """
    index_list = get_index_code_name_list()
    if not index_list:
        return 0

    cur = connect.cursor()
    cur.execute(
        "select cast(stockid as bigint), marketid, code from hku_base.n_stock where type={}".format(
            STOCKTYPE.INDEX
        )
    )
    a = [v for v in cur]
    oldStockDict = {}
    for oldstock in a:
        oldstockid = oldstock[0]
        oldcode = "{}{}".format(
            MARKET.SH if oldstock[1] == MARKETID.SH else MARKET.SZ, oldstock[2]
        )
        oldStockDict[oldcode] = oldstockid

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    now_id = int(datetime.datetime.now().timestamp() * 1000)
    count = 0
    for index in index_list:
        stockid = oldStockDict[index["market_code"]] if index["market_code"] in oldStockDict else now_id + count
        # print(stockid)

        marketid = (
            MARKETID.SH if index["market_code"][:2] == MARKET.SH else MARKETID.SZ
        )
        sql = (
            "insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) \
                       values (%s, %s, '%s', '%s', %s, %s, %s, %s)"
            % (
                stockid,
                marketid,
                index["market_code"][2:],
                index["name"],  # .encode('utf8').decode('utf8', 'ignore'),
                STOCKTYPE.INDEX,
                1,
                today,
                99999999,
            )
        )
        count += 1
        cur.execute(sql)
    cur.close()
    return len(index_list)


@hku_catch(ret=0, trace=True)
def import_stock_name(connect, api, market, quotations=None):
    """更新每只股票的名称、当前是否有效性、起始日期及结束日期
    如果导入的代码表中不存在对应的代码，则认为该股已失效

    :param connect: sqlite3实例
    :param api: pytdx接口，必须在函数外进行连接
    :param market: 'SH' | 'SZ'
    :param quotations: 待导入的行情类别，空为导入全部 'stock' | 'fund' | 'bond' | None
    """
    cur = connect.cursor()

    deSet = set()  # 记录退市证券
    if market == MARKET.SH:
        df = ak.stock_info_sh_delist()
        l = df[['公司代码', '公司简称']].to_dict(orient='records') if not df .empty else []
        for stock in l:
            code = str(stock['公司代码'])
            deSet.add(code)
    elif market == MARKET.SZ:
        for t in ['暂停上市公司', '终止上市公司']:
            df = ak.stock_info_sz_delist(t)
            l = df[['证券代码', '证券简称']].to_dict(orient='records') if not df.empty else []
            for stock in l:
                code = str(stock['证券代码'])
                deSet.add(code)

    newStockDict = {}
    stk_list = get_stk_code_name_list(market)
    if not stk_list:
        hku_error("获取 {} 股票代码表失败", market)
        return 0

    if not quotations or "fund" in [v.lower() for v in quotations]:
        stk_list.extend(get_fund_code_name_list(market))
    for stock in stk_list:
        code = str(stock["code"])
        if code not in deSet:
            newStockDict[code] = stock["name"]

    marketid = get_marketid(connect, market)

    stktype_list = get_stktype_list(quotations)
    stktype_list = list(stktype_list)
    stktype_list.remove(STOCKTYPE.INDEX)  # 移除指数类型
    stktype_list = tuple(stktype_list)
    cur.execute(
        "select cast(stockid as bigint), marketid, code, name, type, valid, startDate, endDate from hku_base.n_stock where marketid={} and type in {}".format(
            marketid, stktype_list
        )
    )
    a = [v for v in cur]
    oldStockDict = {}
    for oldstock in a:
        oldstockid, oldmarketid, oldcode, oldname, oldtype, oldvalid, oldstartDate, oldendDate = oldstock
        oldStockDict[oldcode] = oldstockid

        # 新的代码表中无此股票，则置为无效
        if (oldvalid == 1) and ((oldcode not in newStockDict) or oldcode in deSet):
            cur.execute(
                f"insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) values ({oldstockid}, {oldmarketid}, '{oldcode}', '{oldname}', {oldtype}, 0, {oldstartDate}, {oldendDate})"
            )

        # 股票名称发生变化，更新股票名称;如果原无效，则置为有效
        if oldcode in newStockDict:
            if oldname != newStockDict[oldcode]:
                cur.execute(
                    f"insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) values ({oldstockid}, {oldmarketid}, '{oldcode}', '{newStockDict[oldcode]}', {oldtype}, {oldvalid}, {oldstartDate}, {oldendDate})")
            if oldvalid == 0:
                cur.execute(
                    f"insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) values ({oldstockid}, {oldmarketid}, '{oldcode}', '{oldname}', {oldtype}, 1, {oldstartDate}, 99999999)")

    # 处理新出现的股票
    codepre_list = get_codepre_list(connect, marketid, quotations)

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    now_id = int(datetime.datetime.now().timestamp() * 1000)
    count = 0
    for code in newStockDict:
        if code not in oldStockDict:
            for codepre in codepre_list:
                length = len(codepre[0])
                if code[:length] == codepre[0]:
                    # print(market, code, newStockDict[code], codepre)
                    sql = (
                        "insert into `hku_base`.`n_stock` (stockid, marketid, code, name, type, valid, startDate, endDate) \
                           values (%s, %s, '%s', '%s', %s, %s, %s, %s)"
                        % (
                            now_id + count,
                            marketid,
                            code,
                            newStockDict[code],
                            codepre[1],
                            1,
                            today,
                            99999999,
                        )
                    )
                    cur.execute(sql)
                    count += 1
                    break

    # print('%s新增股票数：%i' % (market.upper(), count))
    cur.close()
    return count


def guess_day_n_step(last_datetime):
    last_date = int(last_datetime // 10000)
    today = datetime.date.today()

    last_y = last_date // 10000
    n = int((today.year - last_y + 1) * 250 // 800)

    step = 800
    if n < 1:
        last_m = last_date // 100 - last_y * 100
        last_d = last_date - (last_y * 10000 + last_m * 100)
        step = (today - datetime.date(last_y, last_m, last_d)).days

    return (n, step)


def guess_1min_n_step(last_datetime):
    last_date = int(last_datetime // 10000)
    today = datetime.date.today()

    last_y = last_date // 10000
    last_m = last_date // 100 - last_y * 100
    last_d = last_date - (last_y * 10000 + last_m * 100)

    n = int((today - datetime.date(last_y, last_m, last_d)).days * 240 // 800)
    step = 800
    if n < 1:
        step = (today - datetime.date(last_y, last_m, last_d)).days * 240
    elif n > 99:
        n = 99

    return (n, step)


def guess_5min_n_step(last_datetime):
    last_date = int(last_datetime // 10000)
    today = datetime.date.today()

    last_y = last_date // 10000
    last_m = last_date // 100 - last_y * 100
    last_d = last_date - (last_y * 10000 + last_m * 100)

    n = int((today - datetime.date(last_y, last_m, last_d)).days * 48 // 800)
    step = 800
    if n < 1:
        step = (today - datetime.date(last_y, last_m, last_d)).days * 48
    elif n > 99:
        n = 99

    return (n, step)


def import_one_stock_data(
    connect, api, market, ktype, stock_record, startDate=199012191500
):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    # stockid, marketid, code, name, type, valid, startDate, endDate
    stockid, marketid, code, name, stktype, valid, stk_startDate, stk_endDate = stock_record
    hku_debug("{}{}".format(market, code))
    table = get_table(connect, market, code, ktype)

    last_krecord = get_last_krecord(connect, table)
    last_datetime = startDate if last_krecord is None else last_krecord[0].ymdhm

    today = datetime.date.today()
    if ktype == "DAY":
        n, step = guess_day_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_RI_K
        today_datetime = (today.year * 10000 + today.month * 100 + today.day) * 10000

    elif ktype == "1MIN":
        n, step = guess_1min_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_1MIN
        today_datetime = (
            today.year * 10000 + today.month * 100 + today.day
        ) * 10000 + 1500

    elif ktype == "5MIN":
        n, step = guess_5min_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_5MIN
        today_datetime = (
            today.year * 10000 + today.month * 100 + today.day
        ) * 10000 + 1500
    else:
        return 0

    if today_datetime <= last_datetime:
        return 0

    get_bars = (
        api.get_index_bars if stktype == STOCKTYPE.INDEX else api.get_security_bars
    )

    if last_krecord is not None:
        bars = get_bars(pytdx_kline_type, pytdx_market, code, 0, 1)
        if not bars:
            return 0
        bar = bars[0]
        if last_krecord[0] != Datetime(bar["year"], bar["month"], bar["day"]) \
                or last_krecord[1] != bar["open"] or last_krecord[2] != bar["high"] \
                or last_krecord[3] != bar["low"] or last_krecord[4] != bar["close"] \
                or abs(last_krecord[5] - bar["amount"]*0.001) > 0.01 or last_krecord[6] != bar["vol"]:
            hku_error("fetch data from tdx error!")
            print(last_krecord[0], Datetime(bar["year"], bar["month"], bar["day"]))
            print(last_krecord[1], bar["open"])
            print(last_krecord[2], bar["high"])
            print(last_krecord[3], bar["low"])
            print(last_krecord[4], bar["close"])
            print(last_krecord[5], bar["amount"])
            print(last_krecord[6], bar["vol"])
            return 0

    buf = []
    while n >= 0:
        bar_list = get_bars(pytdx_kline_type, pytdx_market, code, n * 800, step)
        n -= 1
        if bar_list is None:
            # print(code, "invalid!!")
            continue

        for bar in bar_list:
            try:
                if ktype == "DAY":
                    tmp = datetime.date(bar["year"], bar["month"], bar["day"])
                    bar_datetime = (tmp.year * 10000 + tmp.month * 100 + tmp.day) * 10000
                else:
                    tmp = datetime.datetime(bar["year"], bar["month"], bar["day"], bar['hour'], bar['minute'])
                    bar_datetime = (tmp.year * 10000 + tmp.month * 100 + tmp.day) * \
                        10000 + bar["hour"] * 100 + bar["minute"]
            except Exception as e:
                hku_error("Failed translate datetime: {}, from {}! {}".format(bar, api.ip, e))
                continue

            if (
                today_datetime >= bar_datetime > last_datetime
                and bar["high"] >= bar["open"] >= bar["low"] > 0
                and bar["high"] >= bar["close"] >= bar["low"] > 0
                and bar["vol"] >= 0
                and bar["amount"] >= 0
            ):
                try:
                    buf.append(
                        (
                            bar_datetime,
                            bar["open"],
                            bar["high"],
                            bar["low"],
                            bar["close"],
                            bar["amount"] * 0.001,
                            bar["vol"]
                            # bar['vol'] if stktype == 2 else round(bar['vol'] * 0.01)
                        )
                    )
                except Exception as e:
                    hku_error("Can't trans record({}), {}".format(bar, e))
                last_datetime = bar_datetime

    if len(buf) > 0:
        cur = connect.cursor()
        rawsql = f"INSERT INTO {table} using hku_data.kdata TAGS ('{market.lower()}', '{code}', '{ktype.lower()}') VALUES "
        sql = rawsql
        for i, r in enumerate(buf):
            sql += f"({(Datetime(r[0])-UTCOffset()).timestamp()}, {r[1]}, {r[2]}, {r[3]}, {r[4]}, {r[5]}, {r[6]})"
            if i > 0 and i % 8000 == 0:
                cur.execute(sql)
                sql = rawsql
        if sql != rawsql:
            cur.execute(sql)

        if ktype == "DAY":
            # 更新基础信息数据库中股票对应的起止日期及其有效标志
            # stockid, marketid, code, name, type, valid, startDate, endDate
            cur.execute(f"select FIRST(id) from {table}")
            a = [v for v in cur]
            first_date = Datetime(a[0][0]).ymd
            sql = f"INSERT INTO hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) VALUES ({stockid}, {marketid}, '{code}', '{name}', {stktype}, 1, {first_date}, {stk_endDate})"
            print(sql)
            cur.execute(sql)

            # 记录最新更新日期
            if (
                (code == "000001" and marketid == MARKETID.SH)
                or (code == "399001" and marketid == MARKETID.SZ)
                or (code == "830799" and marketid == MARKETID.BJ)
            ):
                cur.execute(f"select cast(id as bigint) from hku_base.n_market where marketid={marketid}")
                a = [v for v in cur]
                id = a[0][0]
                sql = f"INSERT INTO hku_base.n_market (id, marketid, lastdate) VALUES ({id}, {marketid}, {buf[-1][0]//10000})"
                # print(sql)
                cur.execute(sql)
        cur.close()

    return len(buf)


if __name__ == '__main__':
    import os
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    db = 'taos54'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    tdx_server = '180.101.48.170'
    tdx_port = 7709

    from pytdx.hq import TdxHq_API, TDXParams

    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    connect = taos.connect(
        user=user, password=password, host=host, port=port)

    import_index_name(connect)

    # import_stock_name(connect, None, MARKET.SZ, None)

    quotations = ["stock", "fund"]
    stock_list = get_stock_list(connect, "SH", quotations)
    stock_record = None
    for r in stock_list:
        if r[2] == "000001":
            stock_record = r
            break
    print(stock_record)
    # print(stock_list)

    import_one_stock_data(connect, api, "SH", "DAY", stock_record, startDate=199012191500)

    api.close()
    connect.close()
