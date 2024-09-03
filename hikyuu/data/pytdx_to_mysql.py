# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
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

import sys
import math
import datetime
from pytdx.hq import TDXParams

from hikyuu.util.mylog import hku_error, hku_debug

import mysql.connector

from .common import *
from .common_pytdx import to_pytdx_market, pytdx_get_day_trans
from .common_mysql import (
    create_database,
    get_marketid,
    get_codepre_list,
    get_stock_list,
    get_table,
    get_lastdatetime,
    update_extern_data,
)
from .weight_to_mysql import qianlong_import_weight


def ProgressBar(cur, total):
    percent = "{:.0%}".format(cur / total)
    sys.stdout.write("\r")
    sys.stdout.write("[%-50s] %s" % ("=" * int(math.floor(cur * 50 / total)), percent))
    sys.stdout.flush()


@hku_catch(ret=0, trace=True)
def import_index_name(connect):
    """
    导入所有指数代码表

    :param connect: sqlite3实例
    :return: 指数个数
    """
    index_list = get_index_code_name_list()
    if not index_list:
        return 0

    cur = connect.cursor()
    cur.execute(
        "select stockid, marketid, code from `hku_base`.`stock` where type={}".format(
            STOCKTYPE.INDEX
        )
    )
    a = cur.fetchall()
    oldStockDict = {}
    for oldstock in a:
        oldstockid = oldstock[0]
        oldcode = "{}{}".format(
            MARKET.SH if oldstock[1] == MARKETID.SH else MARKET.SZ, oldstock[2]
        )
        oldStockDict[oldcode] = oldstockid

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    for index in index_list:
        if index["market_code"] in oldStockDict:
            cur.execute(
                "update `hku_base`.`stock` set valid=1, name='%s' where stockid=%i"
                % (index["name"], oldStockDict[index["market_code"]])
            )
        else:
            marketid = (
                MARKETID.SH if index["market_code"][:2] == MARKET.SH else MARKETID.SZ
            )
            sql = (
                "insert into `hku_base`.`stock` (marketid, code, name, type, valid, startDate, endDate) \
                           values (%s, '%s', '%s', %s, %s, %s, %s)"
                % (
                    marketid,
                    index["market_code"][2:],
                    index["name"],
                    STOCKTYPE.INDEX,
                    1,
                    today,
                    99999999,
                )
            )
            cur.execute(sql)
    connect.commit()
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
        "select stockid, code, name, valid from `hku_base`.`stock` where marketid={} and type in {}".format(
            marketid, stktype_list
        )
    )
    a = cur.fetchall()
    oldStockDict = {}
    for oldstock in a:
        oldstockid, oldcode, oldname, oldvalid = (
            oldstock[0],
            oldstock[1],
            oldstock[2],
            int(oldstock[3]),
        )
        oldStockDict[oldcode] = oldstockid

        # 新的代码表中无此股票，则置为无效
        # if (oldvalid == 1) and (oldcode not in newStockDict):
        if (oldvalid == 1) and ((oldcode not in newStockDict) or oldcode in deSet):
            cur.execute(
                "update `hku_base`.`stock` set valid=0 where stockid=%i" % oldstockid
            )

        # 股票名称发生变化，更新股票名称;如果原无效，则置为有效
        if oldcode in newStockDict:
            if oldname != newStockDict[oldcode]:
                cur.execute(
                    "update `hku_base`.`stock` set name='%s' where stockid=%i"
                    % (newStockDict[oldcode], oldstockid)
                )
            if oldvalid == 0:
                cur.execute(
                    "update `hku_base`.`stock` set valid=1, endDate=99999999 where stockid=%i"
                    % oldstockid
                )

    # 处理新出现的股票
    codepre_list = get_codepre_list(connect, marketid, quotations)

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    count = 0
    for code in newStockDict:
        if code not in oldStockDict:
            for codepre in codepre_list:
                length = len(codepre[0])
                if code[:length] == codepre[0]:
                    count += 1
                    # print(market, code, newStockDict[code], codepre)
                    sql = (
                        "insert into `hku_base`.`stock` (marketid, code, name, type, valid, startDate, endDate) \
                           values (%s, '%s', '%s', %s, %s, %s, %s)"
                        % (
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
                    break

    # print('%s新增股票数：%i' % (market.upper(), count))
    connect.commit()
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

    stockid, marketid, code, valid, stktype = stock_record[:5]
    hku_debug("{}{}".format(market, code))
    table = get_table(connect, market, code, ktype)
    last_datetime = get_lastdatetime(connect, table)
    if last_datetime is None:
        last_datetime = startDate

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

    buf = []
    while n >= 0:
        bar_list = get_bars(pytdx_kline_type, pytdx_market, code, n * 800, step)
        n -= 1
        if bar_list is None:
            # print(code, "invalid!!")
            continue

        for bar in bar_list:
            try:
                tmp = datetime.date(bar["year"], bar["month"], bar["day"])
                bar_datetime = (tmp.year * 10000 + tmp.month * 100 + tmp.day) * 10000
                if ktype != "DAY":
                    bar_datetime += bar["hour"] * 100 + bar["minute"]
            except:
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
        sql = (
            "INSERT INTO {tablename} (date, open, high, low, close, amount, count) "
            "VALUES (%s, %s, %s, %s, %s, %s, %s)".format(tablename=table)
        )
        cur.executemany(sql, buf)
        # connect.commit()
        # print(market, stock_record)

        if ktype == "DAY":
            # 更新基础信息数据库中股票对应的起止日期及其有效标志
            sql = (
                "update `hku_base`.`stock` set valid=1, "
                "startdate=(select min(date)/10000 from {table}), "
                "enddate=99999999 "
                "where stockid={stockid}".format(table=table, stockid=stockid)
            )
            cur.execute(sql)

            # 记录最新更新日期
            if (
                (code == "000001" and marketid == MARKETID.SH)
                or (code == "399001" and marketid == MARKETID.SZ)
                or (code == "830799" and marketid == MARKETID.BJ)
            ):
                sql = "update `hku_base`.`market` set lastdate=(select max(date)/10000 from {table})  where marketid={marketid}".format(
                    table=table, marketid=marketid
                )
                try:
                    cur.execute(sql)
                except:
                    print(sql)

        connect.commit()
        cur.close()

    return len(buf)


@hku_catch(trace=True, re_raise=True)
def import_data(
    connect,
    market,
    ktype,
    quotations,
    api,
    dest_dir,
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

    stock_list = get_stock_list(connect, market, quotations)

    total = len(stock_list)
    for i, stock in enumerate(stock_list):
        if stock[3] == 0 or len(stock[2]) != 6:
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


@hku_catch(trace=True, re_raise=True)
def get_trans_table(connect, market, code):
    cur = connect.cursor()
    schema = "{market}_trans".format(market=market).lower()
    cur.execute(
        "SELECT 1 FROM information_schema.SCHEMATA where SCHEMA_NAME='{}'".format(
            schema
        )
    )
    a = cur.fetchone()
    if not a:
        cur.execute("CREATE SCHEMA `{}`".format(schema))
        connect.commit()

    tablename = code.lower()
    cur.execute(
        "SELECT 1 FROM information_schema.tables "
        "where table_schema='{schema}' and table_name='{name}'".format(
            schema=schema, name=tablename
        )
    )
    a = cur.fetchone()
    if not a:
        sql = """
                CREATE TABLE `{schema}`.`{name}` (
                    `date` BIGINT(20) UNSIGNED NOT NULL,
                    `price` DOUBLE NOT NULL,
                    `vol` DOUBLE NOT NULL,
                    `buyorsell` INT NOT NULL,
                    PRIMARY KEY (`date`)
                )
                COLLATE='utf8mb4_general_ci'
                ENGINE=MyISAM
                ;
              """.format(
            schema=schema, name=tablename
        )
        cur.execute(sql)
        connect.commit()

    cur.close()
    return "`{schema}`.`{name}`".format(schema=schema, name=tablename)


@hku_catch(trace=True)
def import_on_stock_trans(connect, api, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    stockid, marketid, code, valid, stktype = stock_record[:5]
    hku_debug("{}{}".format(market, code))
    table = get_trans_table(connect, market, code)
    last_datetime = get_lastdatetime(connect, table)

    today = datetime.date.today()
    if last_datetime is not None:
        # yyyymmddHHMMSS
        last_y = int(last_datetime // 10000000000)
        last_m = int(last_datetime // 100000000 - last_y * 100)
        last_d = int(last_datetime // 1000000 - (last_y * 10000 + last_m * 100))
        last_date = datetime.date(last_y, last_m, last_d)
        need_days = (today - last_date).days
    else:
        need_days = max_days

    date_list = []
    for i in range(need_days):
        cur_date = today - datetime.timedelta(i)
        if cur_date.weekday() not in (5, 6):
            date_list.append(
                cur_date.year * 10000 + cur_date.month * 100 + cur_date.day
            )
    date_list.reverse()

    trans_buf = []
    for cur_date in date_list:
        buf = pytdx_get_day_trans(api, pytdx_market, stock_record[2], cur_date)
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
        sql = (
            f"INSERT INTO {table} (date, price, vol, buyorsell) VALUES (%s, %s, %s, %s)"
        )
        cur.executemany(sql, trans_buf)
        connect.commit()
        cur.close()
    return len(trans_buf)


def import_trans(
    connect, market, quotations, api, dest_dir, max_days=30, progress=ProgressBar
):
    add_record_count = 0
    market = market.upper()

    stock_list = get_stock_list(connect, market, quotations)
    total = len(stock_list)
    a_stktype_list = get_a_stktype_list()
    for i, stock in enumerate(stock_list):
        if stock[3] == 0 or len(stock[2]) != 6 or stock[4] not in a_stktype_list:
            if progress:
                progress(i, total)
            continue

        this_count = import_on_stock_trans(connect, api, market, stock, max_days)
        add_record_count += this_count
        if progress:
            progress(i, total)

    return add_record_count


def get_time_table(connect, market, code):
    cur = connect.cursor()
    schema = "{market}_time".format(market=market).lower()
    cur.execute(
        "SELECT 1 FROM information_schema.SCHEMATA where SCHEMA_NAME='{}'".format(
            schema
        )
    )
    a = cur.fetchone()
    if not a:
        cur.execute("CREATE SCHEMA `{}`".format(schema))
        connect.commit()

    tablename = code.lower()
    cur.execute(
        "SELECT 1 FROM information_schema.tables "
        "where table_schema='{schema}' and table_name='{name}'".format(
            schema=schema, name=tablename
        )
    )
    a = cur.fetchone()
    if not a:
        sql = """
                CREATE TABLE `{schema}`.`{name}` (
                    `date` BIGINT(20) UNSIGNED NOT NULL,
                    `price` DOUBLE NOT NULL,
                    `vol` DOUBLE NOT NULL,
                    PRIMARY KEY (`date`)
                )
                COLLATE='utf8mb4_general_ci'
                ENGINE=MyISAM
                ;
              """.format(
            schema=schema, name=tablename
        )
        cur.execute(sql)
        connect.commit()

    cur.close()
    return "`{schema}`.`{name}`".format(schema=schema, name=tablename)


@hku_catch(trace=True)
def import_on_stock_time(connect, api, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    stockid, marketid, code, valid, stktype = stock_record[:5]
    hku_debug("{}{}".format(market, code))
    table = get_time_table(connect, market, code)
    last_datetime = get_lastdatetime(connect, table)

    today = datetime.date.today()
    if last_datetime is not None:
        # yyyymmddHHMM
        last_y = int(last_datetime // 100000000)
        last_m = int(last_datetime // 1000000 - last_y * 100)
        last_d = int(last_datetime // 10000 - (last_y * 10000 + last_m*100))
        last_date = datetime.date(last_y, last_m, last_d)
        need_days = (today - last_date).days
    else:
        need_days = max_days

    date_list = []
    for i in range(need_days):
        cur_date = today - datetime.timedelta(i)
        if cur_date.weekday() not in (5, 6):
            date_list.append(cur_date.year * 10000 + cur_date.month * 100 + cur_date.day)
    date_list.reverse()

    time_buf = []
    for cur_date in date_list:
        buf = api.get_history_minute_time_data(pytdx_market, stock_record[2], cur_date)
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
        sql = (
            f"INSERT INTO {table} (date, price, vol) VALUES (%s, %s, %s)"
        )
        cur.executemany(sql, time_buf)
        connect.commit()
        cur.close()

    return len(time_buf)


def import_time(connect, market, quotations, api, dest_dir, max_days=9000, progress=ProgressBar):
    add_record_count = 0
    market = market.upper()

    stock_list = get_stock_list(connect, market, quotations)
    total = len(stock_list)
    for i, stock in enumerate(stock_list):
        if stock[3] == 0 or len(stock[2]) != 6:
            if progress:
                progress(i, total)
            continue

        this_count = import_on_stock_time(connect, api, market, stock, max_days)
        add_record_count += this_count
        if progress:
            progress(i, total)

    return add_record_count


if __name__ == "__main__":
    import os
    import time

    starttime = time.time()

    host = "127.0.0.1"
    port = 3306
    usr = "root"
    pwd = ""
    tdx_server = "119.147.212.81"
    tdx_port = 7709
    quotations = ["stock", "fund"]

    connect = mysql.connector.connect(user=usr, password=pwd, host=host, port=port)
    create_database(connect)

    from pytdx.hq import TdxHq_API, TDXParams

    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    add_count = 0

    print("导入股票代码表")
    # add_count = import_stock_name(connect, api, 'SH', quotations)
    # add_count += import_stock_name(connect, api, 'SZ', quotations)
    print("新增股票数：", add_count)

    print("\n导入上证日线数据")
    add_count = import_data(connect, "SH", "DAY", quotations, api, progress=ProgressBar)
    print("\n导入数量：", add_count)
    """
    print("\n导入深证日线数据")
    add_count = import_data(connect, 'SZ', 'DAY', quotations, api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)

    print("\n导入上证5分钟线数据")
    add_count = import_data(connect, 'SH', '5MIN', quotations, api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)

    print("\n导入深证5分钟线数据")
    add_count = import_data(connect, 'SZ', '5MIN', quotations, api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)

    print("\n导入上证分钟线数据")
    add_count = import_data(connect, 'SH', '1MIN', quotations, api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)

    print("\n导入深证分钟线数据")
    add_count = import_data(connect, 'SZ', '1MIN', quotations, api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)

    print("\n导入权息数据")
    print("正在下载权息数据...")
    import urllib.request
    net_file = urllib.request.urlopen('http://www.qianlong.com.cn/download/history/weight.rar', timeout=60)
    dest_filename = dest_dir + '/weight.rar'
    with open(dest_filename, 'wb') as file:
        file.write(net_file.read())

    print("下载完成，正在解压...")
    os.system('unrar x -o+ -inul {} {}'.format(dest_filename, dest_dir))

    print("解压完成，正在导入...")
    add_count = qianlong_import_weight(connect, dest_dir + '/weight', 'SH')
    add_count += qianlong_import_weight(connect, dest_dir + '/weight', 'SZ')
    print("导入数量：", add_count)
    """

    # for i in range(10):
    #    x = api.get_history_transaction_data(TDXParams.MARKET_SZ, '000001', (9-i)*2000, 2000, 20181112)
    # x = api.get_transaction_data(TDXParams.MARKET_SZ, '000001', (9-i)*800, 800)
    #    if x is not None and len(x) > 0:
    #        print(i, len(x), x[0], x[-1])

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
