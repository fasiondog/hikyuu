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

from hikyuu import Datetime, UTCOffset, Days
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


def ProgressBar(cur, total):
    percent = "{:.0%}".format(cur / total)
    sys.stdout.write("\r")
    sys.stdout.write("[%-50s] %s" % ("=" * int(math.floor(cur * 50 / total)), percent))
    sys.stdout.flush()


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
        "select cast(stockid as bigint), market, code from hku_base.n_stock where type={}".format(
            STOCKTYPE.INDEX
        )
    )
    a = [v for v in cur]
    oldStockDict = {}
    for oldstock in a:
        oldstockid = oldstock[0]
        oldcode = f"{oldstock[1]}{oldstock[2]}"
        oldStockDict[oldcode] = oldstockid

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    now_id = int(datetime.datetime.now().timestamp() * 1000)
    count = 0
    for index in index_list:
        market_code = index["market_code"].upper()
        stockid = oldStockDict[market_code] if market_code in oldStockDict else now_id + count
        # print(stockid)

        market = market_code[:2].upper()
        sql = (
            "insert into hku_base.n_stock (stockid, market, code, name, type, valid, startDate, endDate) \
                       values (%s, '%s', '%s', '%s', %s, %s, %s, %s)"
            % (
                stockid,
                market,
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
        "select cast(stockid as bigint), market, code, name, type, valid, startDate, endDate from hku_base.n_stock where market='{}' and type in {}".format(
            market.upper(), stktype_list
        )
    )
    a = [v for v in cur]
    oldStockDict = {}
    for oldstock in a:
        oldstockid, oldmarket, oldcode, oldname, oldtype, oldvalid, oldstartDate, oldendDate = oldstock
        oldStockDict[oldcode] = oldstockid

        # 新的代码表中无此股票，则置为无效
        if (oldvalid == 1) and ((oldcode not in newStockDict) or oldcode in deSet):
            cur.execute(
                f"insert into hku_base.n_stock (stockid, market, code, name, type, valid, startDate, endDate) values ({oldstockid}, '{oldmarket}', '{oldcode}', '{oldname}', {oldtype}, 0, {oldstartDate}, {oldendDate})"
            )

        # 股票名称发生变化，更新股票名称;如果原无效，则置为有效
        if oldcode in newStockDict:
            if oldname != newStockDict[oldcode]:
                cur.execute(
                    f"insert into hku_base.n_stock (stockid, market, code, name, type, valid, startDate, endDate) values ({oldstockid}, '{oldmarket}', '{oldcode}', '{newStockDict[oldcode]}', {oldtype}, {oldvalid}, {oldstartDate}, {oldendDate})")
            if oldvalid == 0:
                cur.execute(
                    f"insert into hku_base.n_stock (stockid, market, code, name, type, valid, startDate, endDate) values ({oldstockid}, '{oldmarket}', '{oldcode}', '{oldname}', {oldtype}, 1, {oldstartDate}, 99999999)")

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
                        "insert into `hku_base`.`n_stock` (stockid, market, code, name, type, valid, startDate, endDate) \
                           values (%s, '%s', '%s', '%s', %s, %s, %s, %s)"
                        % (
                            now_id + count,
                            market,
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
        step = (today - datetime.date(last_y, last_m, last_d)).days + 1
        if step > 800:
            n = 1
            step = 800

    return (n, step)


def guess_1min_n_step(last_datetime):
    last_date = int(last_datetime // 10000)
    today = datetime.date.today()

    last_y = last_date // 10000
    last_m = last_date // 100 - last_y * 100
    last_d = last_date - (last_y * 10000 + last_m * 100)

    n = int(((today - datetime.date(last_y, last_m, last_d)).days * 240 + 1) // 800)
    step = 800
    if n < 1:
        step = (today - datetime.date(last_y, last_m, last_d)).days * 240 + 1
    elif n > 99:
        n = 99

    return (n, step)


def guess_5min_n_step(last_datetime):
    last_date = int(last_datetime // 10000)
    today = datetime.date.today()

    last_y = last_date // 10000
    last_m = last_date // 100 - last_y * 100
    last_d = last_date - (last_y * 10000 + last_m * 100)

    n = int(((today - datetime.date(last_y, last_m, last_d)).days * 48 + 1) // 800)
    step = 800
    if n < 1:
        step = (today - datetime.date(last_y, last_m, last_d)).days * 48 + 1
    elif n > 99:
        n = 99

    return (n, step)


def import_one_stock_data(
    connect, api, market, ktype, stock_record, startDate=199012191500
):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    # stockid, market, code, name, type, valid, startDate, endDate
    stockid, market, code, name, stktype, valid, stk_startDate, stk_endDate = stock_record
    hku_debug("{}{}".format(market, code))
    table = get_table(connect, market, code, ktype)

    last_krecord = get_last_krecord(connect, table)
    last_datetime = startDate if last_krecord is None else last_krecord[0].ymdhm

    today = datetime.date.today()
    if ktype == "DAY":
        nktype = "day"
        n, step = guess_day_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_RI_K
        today_datetime = (today.year * 10000 + today.month * 100 + today.day) * 10000

    elif ktype == "1MIN":
        nktype = "min"
        n, step = guess_1min_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_1MIN
        today_datetime = (
            today.year * 10000 + today.month * 100 + today.day
        ) * 10000 + 1500

    elif ktype == "5MIN":
        nktype = "min5"
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
        if ktype == 'DAY':
            days = (Datetime.today() - last_krecord[0]).days
            num = days + 1
        elif ktype == '1MIN':
            days = (Datetime.today() - last_krecord[0].start_of_day()).days
            num = days*240+1
        elif ktype == '5MIN':
            days = (Datetime.today() - last_krecord[0].start_of_day()).days
            num = days*48+1
        if num >= 1:
            bars = get_bars(pytdx_kline_type, pytdx_market, code, 0, num)
            if not bars:
                return 0
            bar = bars[-1]
            if ktype == 'DAY':
                bardate = Datetime(bar["year"], bar["month"], bar["day"])
            else:
                bardate = Datetime(bar["year"], bar["month"], bar["day"], bar['hour'], bar['minute'])
            if last_krecord[0] == bardate:
                if abs(last_krecord[1] - bar["open"]) / last_krecord[1] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[1]}, bar: {bar['open']}")
                    return 0
                if abs(last_krecord[2] - bar["high"]) / last_krecord[2] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[2]}, bar: {bar['high']}")
                    return 0
                if abs(last_krecord[3] - bar["low"]) / last_krecord[3] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[3]}, bar: {bar['low']}")
                    return 0
                if abs(last_krecord[4] - bar["close"]) / last_krecord[4] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[4]}, bar: {bar['close']}")
                    return 0
                if ktype == 'DAY' and last_krecord[5] != 0.0 and abs(last_krecord[5] - bar["amount"]*0.001) / last_krecord[5] > 0.1:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[5]}, bar: {bar['amount']*0.001}")
                    return 0
                if ktype == 'DAY' and last_krecord[5] != 0.0 and abs(last_krecord[6] - bar["vol"]) / last_krecord[6] > 0.1:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[6]}, bar: {bar['vol']}")
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

            if last_krecord is not None and bar_datetime == last_datetime:
                if abs(last_krecord[1] - bar["open"]) / last_krecord[1] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[1]}, bar: {bar['open']}")
                    return 0
                if abs(last_krecord[2] - bar["high"]) / last_krecord[2] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[2]}, bar: {bar['high']}")
                    return 0
                if abs(last_krecord[3] - bar["low"]) / last_krecord[3] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[3]}, bar: {bar['low']}")
                    return 0
                if abs(last_krecord[4] - bar["close"]) / last_krecord[4] > 0.02:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[4]}, bar: {bar['close']}")
                    return 0
                if ktype == 'DAY' and last_krecord[5] != 0.0 and abs(last_krecord[5] - bar["amount"]*0.001) / last_krecord[5] > 0.1:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[5]}, bar: {bar['amount']*0.001}")
                    return 0
                if ktype == 'DAY' and last_krecord[5] != 0.0 and abs(last_krecord[6] - bar["vol"]) / last_krecord[6] > 0.1:
                    hku_error(
                        f"fetch data from tdx error! {bardate} {ktype} {market}{code} last_krecord: {last_krecord[6]}, bar: {bar['vol']}")
                    return 0
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
        rawsql = f"INSERT INTO {table} using {nktype}_data.kdata TAGS ('{market.upper()}', '{code}') VALUES "
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
            # stockid, market, code, name, type, valid, startDate, endDate
            cur.execute(f"select FIRST(date) from {table}")
            a = [v for v in cur]
            first_date = Datetime(a[0][0]).ymd
            sql = f"INSERT INTO hku_base.n_stock (stockid, market, code, name, type, valid, startDate, endDate) VALUES ({stockid}, '{market.upper()}', '{code}', '{name}', {stktype}, 1, {first_date}, {stk_endDate})"
            # print(sql)
            cur.execute(sql)

            # 记录最新更新日期
            if (
                (code == "000001" and market == MARKET.SH)
                or (code == "399001" and market == MARKET.SZ)
                or (code == "830799" and market == MARKET.BJ)
            ):
                cur.execute(f"select cast(id as bigint) from hku_base.n_market where market='{market.upper()}'")
                a = [v for v in cur]
                id = a[0][0]
                sql = f"INSERT INTO hku_base.n_market (id, lastdate) VALUES ({id}, {buf[-1][0]//10000})"
                # print(sql)
                cur.execute(sql)
        cur.close()

    return len(buf)


@hku_catch(trace=True, re_raise=True)
def import_data(
    connect,
    market,
    ktype,
    quotations,
    api,
    dest_dir=None,
    startDate=199012190000,
    progress=ProgressBar,
):
    """导入通达信指定盘后数据路径中的K线数据。注：只导入基础信息数据库中存在的股票。

    :param connect   : sqlit3链接
    :param market    : 'SH' | 'SZ'
    :param ktype     : 'DAY' | '1MIN' | '5MIN'
    :param quotations: 'stock' | 'fund' | 'bond'
    :param src_dir   : 盘后K线数据路径，如上证5分钟线：D:\\Tdx\\vipdoc\\sh\\fzline
    :param dest_dir  : HDF5数据文件所在目录
    :param progress  : 进度显示函数
    :return: 导入记录数
    """
    add_record_count = 0
    market = market.upper()

    # stockid, marketid, code, name, type, valid, startDate, endDate
    stock_list = get_stock_list(connect, market, quotations)

    total = len(stock_list)
    for i, stock in enumerate(stock_list):
        if stock[5] == 0 or len(stock[2]) != 6:
            if progress:
                progress(i, total)
            continue

        this_count = import_one_stock_data(
            connect, api, market, ktype, stock, startDate
        )
        add_record_count += this_count
        if this_count > 0:
            if ktype == "DAY":
                update_extern_data(connect, market.upper(), stock[2], "DAY")
            elif ktype == "5MIN":
                update_extern_data(connect, market.upper(), stock[2], "5MIN")

        if progress:
            progress(i, total)

    connect.commit()
    return add_record_count


@hku_catch(trace=True)
def import_on_stock_trans(connect, api, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    # stockid, marketid, code, name, type, valid, startDate, endDate
    stockid, marketid, code, name, stktype, valid = stock_record[:6]
    hku_debug("{}{}".format(market, code))
    table = get_table(connect, market, code, 'transdata')
    last_datetime = get_lastdatetime(connect, table)

    today = Datetime.today()
    if last_datetime is not None:
        # yyyymmddHHMMSS
        last_date = last_datetime.start_of_day()
        need_days = (today - last_date).days
    else:
        need_days = max_days

    date_list = []
    for i in range(need_days):
        cur_date = today - Days(i)
        if cur_date.day_of_week not in (0, 6):
            date_list.append(cur_date.ymd)
    date_list.reverse()

    trans_buf = []
    for cur_date in date_list:
        buf = pytdx_get_day_trans(api, pytdx_market, code, cur_date)
        if not buf:
            continue

        second = 2
        pre_minute = 900

        for record in buf:
            try:
                minute = int(record["time"][0:2]) * 100 + int(record["time"][3:])
                if minute != pre_minute:
                    second = 0 if minute == 1500 else 2
                    pre_minute = minute
                else:
                    second += 3
                if second > 59:
                    continue

                trans_buf.append(
                    (
                        cur_date * 1000000 + minute * 100 + second,
                        record["price"],
                        record["vol"],
                        record["buyorsell"],
                    )
                )
            except Exception as e:
                hku_error("Failed trans to record! {}", e)

    if trans_buf:
        cur = connect.cursor()
        rawsql = f"INSERT INTO {table} using transdata_data.transdata TAGS ('{market.upper()}', '{code}') VALUES "
        sql = rawsql
        for i, r in enumerate(trans_buf):
            sql += f"({(Datetime(r[0])-UTCOffset()).timestamp()}, {r[1]}, {r[2]}, {r[3]})"
            if i > 0 and i % 15000 == 0:
                cur.execute(sql)
                sql = rawsql
        if sql != rawsql:
            cur.execute(sql)
        cur.close()
    return len(trans_buf)


def import_trans(
    connect, market, quotations, api, dest_dir=None, max_days=30, progress=ProgressBar
):
    add_record_count = 0
    market = market.upper()

    stock_list = get_stock_list(connect, market, quotations)
    total = len(stock_list)
    a_stktype_list = get_a_stktype_list()
    for i, stock in enumerate(stock_list):
        # stockid, marketid, code, name, type, valid, startDate, endDate
        if stock[5] == 0 or len(stock[2]) != 6 or stock[4] not in a_stktype_list:
            if progress:
                progress(i, total)
            continue

        this_count = import_on_stock_trans(connect, api, market, stock, max_days)
        add_record_count += this_count
        if progress:
            progress(i, total)

    return add_record_count


@hku_catch(trace=True)
def import_on_stock_time(connect, api, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    # stockid, marketid, code, name, type, valid, startDate, endDate
    stockid, marketid, code, name, stktype, valid = stock_record[:6]
    hku_debug("{}{}".format(market, code))
    table = get_table(connect, market, code, 'timeline')
    last_datetime = get_lastdatetime(connect, table)

    today = Datetime.today()
    if last_datetime is not None:
        # yyyymmddHHMM
        last_date = last_datetime.start_of_day()
        need_days = (today - last_date).days
    else:
        need_days = max_days

    date_list = []
    for i in range(need_days):
        cur_date = today - Days(i)
        if cur_date.day_of_week() not in (0, 6):
            date_list.append(cur_date.ymd)
    date_list.reverse()

    time_buf = []
    for cur_date in date_list:
        buf = api.get_history_minute_time_data(pytdx_market, code, cur_date)
        if buf is None or len(buf) != 240:
            # print(cur_date, "获取的分时线长度不为240!", stock_record[1], stock_record[2])
            continue
        this_date = cur_date * 10000
        time = 930
        for record in buf:
            if time == 960:
                time = 1000
            elif time == 1060:
                time = 1100
            elif time == 1130:
                time = 1300
            elif time == 1360:
                time = 1400
            try:
                time_buf.append((this_date + time, record['price'], record['vol']))
                time += 1
            except Exception as e:
                hku_error("Failed trans record {}! {}".format(record, e))

    if time_buf:
        cur = connect.cursor()
        rawsql = f"INSERT INTO {table} using timeline_data.timeline TAGS ('{market.upper()}', '{code}') VALUES "
        sql = rawsql
        for i, r in enumerate(time_buf):
            sql += f"({(Datetime(r[0])-UTCOffset()).timestamp()}, {r[1]}, {r[2]})"
            if i > 0 and i % 16000 == 0:
                cur.execute(sql)
                sql = rawsql
        if sql != rawsql:
            cur.execute(sql)
        cur.close()

    return len(time_buf)


def import_time(connect, market, quotations, api, dest_dir=None, max_days=9000, progress=ProgressBar):
    add_record_count = 0
    market = market.upper()

    stock_list = get_stock_list(connect, market, quotations)
    total = len(stock_list)
    for i, stock in enumerate(stock_list):
        # stockid, marketid, code, name, type, valid, startDate, endDate
        if stock[5] == 0 or len(stock[2]) != 6:
            if progress:
                progress(i, total)
            continue

        this_count = import_on_stock_time(connect, api, market, stock, max_days)
        add_record_count += this_count
        if progress:
            progress(i, total)

    return add_record_count


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

    from hikyuu.data.common_taos import get_taos
    connect = get_taos().connect(
        user=user, password=password, host=host, port=port)

    # import_index_name(connect)

    # import_stock_name(connect, None, MARKET.SH, None)

    quotations = ["stock", "fund"]

    stock_list = get_stock_list(connect, "SH", quotations)
    stock_record = None
    for r in stock_list:
        if r[2] == "000001":
            stock_record = r
            break
    print(stock_record)

    import_one_stock_data(connect, api, "SH", "5MIN", stock_record, startDate=199012191500)

    # print("\n导入上证日线数据")
    # add_count = import_data(connect, "SH", "DAY", quotations, api, progress=ProgressBar)
    # print("\n导入数量：", add_count)

    # import_time(connect, "SH", quotations, api, max_days=30)

    api.disconnect()
    connect.close()
