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
import os
import math
import datetime
from pytdx.hq import TDXParams
from configparser import ConfigParser

from hikyuu.util import hku_error, hku_debug, hku_run_ignore_exception

from hikyuu import Datetime, roundEx, is_valid_license, KDataToClickHouseImporter
from hikyuu.data.common import *
from hikyuu.data.common_pytdx import to_pytdx_market, pytdx_get_day_trans
from hikyuu.data.common_clickhouse import (
    create_database,
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

    :param connect: sqlite3实例
    :return: 指数个数
    """
    index_list = get_index_code_name_list()
    if not index_list:
        return 0

    sql = f"select market, code, name, type, valid, startDate, endDate from hku_base.stock where type={STOCKTYPE.INDEX}"
    a = connect.query(sql)
    a = a.result_rows
    oldStockDict = {}
    for oldstock in a:
        oldcode = f"{oldstock[0]}{oldstock[1]}"
        oldStockDict[oldcode] = oldstock

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    insert_records = []
    for index in index_list:
        market_code = index["market_code"]
        market, code = market_code[:2], market_code[2:]
        if market_code in oldStockDict:
            old = oldStockDict[market_code]
            if old[4] == 0:
                # connect.command(
                #     f"alter table hku_base.stock update valid=1, name='{index['name']}' where market='{market}' and code='{code}'")
                connect.command(
                    f"delete from hku_base.stock where market='{market}' and code='{code}'")
                insert_records.append((market, code, index["name"], STOCKTYPE.INDEX, 1, today, 99999999))
        else:
            insert_records.append((market, code, index["name"], STOCKTYPE.INDEX, 1, today, 99999999))

    if insert_records:
        ic = connect.create_insert_context(table='stock', database='hku_base',
                                           column_names=['market', 'code', 'name',
                                                         'type', 'valid', 'startDate', 'endDate'],
                                           data=insert_records)
        connect.insert(context=ic)
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

    # 号码前缀
    codepre_list = get_codepre_list(connect, market, quotations)

    stktype_list = get_stktype_list(quotations)
    stktype_list = list(stktype_list)
    stktype_list.remove(STOCKTYPE.INDEX)  # 移除指数类型
    stktype_list = tuple(stktype_list)
    sql = f"select market, code, name, type, valid, startDate, endDate from hku_base.stock where market='{market}' and type in {stktype_list}"
    a = connect.query(sql)
    a = a.result_rows
    oldStockDict = {}
    insert_records = []
    for oldstock in a:
        oldcode = oldstock[1]
        oldStockDict[oldcode] = oldstock

        oldname, oldtype, oldvalid, oldstartDate, oldendDate = oldstock[2], oldstock[3], oldstock[4], oldstock[5], oldstock[6]

        # 检测是否类型发生变化
        newtype = oldtype
        for codepre in codepre_list:
            length = len(codepre[0])
            if oldcode[:length] == codepre[0]:
                if oldtype != codepre[1]:
                    newtype = codepre[1]
                break

        # 新的代码表中无此股票，则置为无效
        if (oldvalid == 1) and ((oldcode not in newStockDict) or oldcode in deSet):
            sql = f"delete from hku_base.stock where market='{market}' and code='{oldcode}'"
            connect.command(sql)
            insert_records.append((market, oldcode, oldname, newtype, 0, oldstartDate, oldendDate))

        # 股票名称发生变化，更新股票名称;如果原无效，则置为有效
        if oldcode in newStockDict:
            if oldname != newStockDict[oldcode] or oldvalid == 0 or oldtype != newtype:
                sql = f"delete from hku_base.stock where market='{market}' and code='{oldcode}'"
                connect.command(sql)
                insert_records.append((market, oldcode, newStockDict[oldcode], newtype, 1, oldstartDate, 99999999))

    # 处理新出现的股票
    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    count = 0
    for code in newStockDict:
        if code not in oldStockDict:
            for codepre in codepre_list:
                length = len(codepre[0])
                if code[:length] == codepre[0]:
                    count += 1
                    insert_records.append((market, code, newStockDict[code], codepre[1], 1, today, 99999999))
                    break

    if insert_records:
        ic = connect.create_insert_context(table='stock', database='hku_base',
                                           column_names=['market', 'code', 'name',
                                                         'type', 'valid', 'startDate', 'endDate'],
                                           data=insert_records)
        connect.insert(context=ic)
    # print('%s新增股票数：%i' % (market.upper(), count))
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

    market, code, valid, stktype = stock_record[:5]
    hku_debug("{}{}".format(market, code))
    table = get_table(connect, market, code, ktype)
    last_krecord = get_last_krecord(connect, table)
    last_datetime = startDate if last_krecord is None else last_krecord[0]

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
        return (0, True, Datetime(last_datetime))

    if today_datetime <= last_datetime:
        return (0, True, Datetime(last_datetime))

    get_bars = (
        api.get_index_bars if stktype == STOCKTYPE.INDEX else api.get_security_bars
    )

    buf = []
    # 用于去重的集合，记录已处理的 (market, code, date) 组合
    seen_keys = set()

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

            if 'open' not in bar or 'high' not in bar or 'low' not in bar or 'close' not in bar or 'vol' not in bar or 'amount' not in bar:
                hku_warn(f"Ignore invalid bar: {bar_datetime}, missing fields!")
                continue

            if last_krecord is not None and bar_datetime == last_datetime:
                if abs(last_krecord[1] - bar["open"]) / last_krecord[1] > 0.01:
                    hku_error(
                        f"fetch data from tdx error! {bar_datetime} {ktype} {market}{code} last_krecord open: {last_krecord[1]}, bar: {bar['open']}")
                    return (0, False, Datetime(last_datetime))
                if abs(last_krecord[2] - bar["high"]) / last_krecord[2] > 0.01:
                    hku_error(
                        f"fetch data from tdx error! {bar_datetime} {ktype} {market}{code} last_krecord high: {last_krecord[2]}, bar: {bar['high']}")
                    return (0, False,  Datetime(last_datetime))
                if abs(last_krecord[3] - bar["low"]) / last_krecord[3] > 0.01 and abs(last_krecord[3] - bar["low"]) > 0.01:
                    hku_error(
                        f"fetch data from tdx error! {bar_datetime} {ktype} {market}{code} last_krecord low: {last_krecord[3]}, bar: {bar['low']}")
                    return (0, False, Datetime(last_datetime))
                if abs(last_krecord[4] - bar["close"]) / last_krecord[4] > 0.01:
                    hku_error(
                        f"fetch data from tdx error! {bar_datetime} {ktype} {market}{code} last_krecord close: {last_krecord[4]}, bar: {bar['close']}")
                    return (0, False, Datetime(last_datetime))
                if ktype == 'DAY' and last_krecord[5] != 0.0:
                    # 成交额检查：根据数值大小分档次进行相对百分比比较
                    amount_diff_ratio = abs(last_krecord[5] - bar["amount"]) / last_krecord[5]
                    amount_threshold = 0.5 if last_krecord[5] >= 1e8 else (0.8 if last_krecord[5] >= 1e6 else 1.0)
                    if amount_diff_ratio > amount_threshold:
                        hku_error(
                            f"fetch data from tdx error! {bar_datetime} {ktype} {market}{code} last_krecord amount: {last_krecord[5]}, bar: {bar['amount']}, diff_ratio: {amount_diff_ratio:.4f}")
                        return (0, False, Datetime(last_datetime))
                if ktype == 'DAY' and last_krecord[6] != 0.0:
                    # 成交量检查：根据数值大小分档次进行相对百分比比较
                    vol_diff_ratio = abs(last_krecord[6] - bar["vol"]) / last_krecord[6]
                    vol_threshold = 0.5 if last_krecord[6] >= 1e7 else (0.8 if last_krecord[6] >= 1e5 else 1.0)
                    if vol_diff_ratio > vol_threshold:
                        hku_error(
                            f"fetch data from tdx error! {bar_datetime} {ktype} {market}{code} last_krecord count: {last_krecord[6]}, bar: {bar['vol']}, diff_ratio: {vol_diff_ratio:.4f}")
                        return (0, False, Datetime(last_datetime))
                continue

            # 不产生实际交易的数据不保留，计算时根据需要对齐更方便
            # 日线忽略成交金额为0的(量可为0，额不为0)，通常为停牌或错误，停牌数据不保留计算时可以根据时间需要进行对齐计算
            # 日线以下成交量和成交量只保留都不为0的数据
            # 日线（手 + 万元）：只要成交额 > 0 就保留
            # 分钟线（股 + 元): 量、额必须都大于0
            if (
                bar_datetime not in seen_keys
                    and today_datetime >= bar_datetime > last_datetime
                    and bar["high"] >= bar["open"] >= bar["low"] > 0
                    and bar["high"] >= bar["close"] >= bar["low"] > 0
                    and bar["vol"] >= 0
                    and bar["amount"] >= 0
            ):
                if ktype == "DAY":
                    if int(roundEx(bar["amount"], 0)) == 0:
                        continue
                else:
                    if int(roundEx(bar["amount"], 0)) == 0 or int(roundEx(bar["vol"], 3)*1000) == 0:
                        continue
                try:
                    buf.append(
                        (
                            table[1], table[2],
                            Datetime(bar_datetime).timestamp_utc()//1000000,
                            int(roundEx(bar["open"], 3)*1000),
                            int(roundEx(bar["high"], 3)*1000),
                            int(roundEx(bar["low"], 3)*1000),
                            int(roundEx(bar["close"], 3)*1000),
                            int(roundEx(bar["amount"], 0)),
                            int(roundEx(bar["vol"], 3)*1000)
                            # bar['vol'] if stktype == 2 else round(bar['vol'] * 0.01)
                        )
                    )
                    # 标记为已处理
                    seen_keys.add(bar_datetime)
                except Exception as e:
                    hku_error("Can't trans record({}), {}".format(bar, e))
                last_datetime = bar_datetime

    if len(buf) > 0:
        ic = connect.create_insert_context(table=table[0],
                                           data=buf)
        connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})

    return (len(buf), True, Datetime(last_datetime))


def update_stock_info(connect, market):
    sql = f"SELECT a.code, a.valid, a.startDate, a.endDate, b.min_date, b.max_date FROM hku_base.stock a JOIN (SELECT market, code, toInt32(min(date)) AS min_date, toInt32(max(date)) AS max_date FROM hku_data.day_k WHERE market = '{market}' GROUP BY market, code) b ON a.market = b.market AND a.code = b.code"
    # hku_info(sql)
    a = connect.query(sql)
    a = a.result_rows
    ticks = 1000000
    for v in a:
        code, valid, startDate, endDate, min_date, max_date = v
        now_start = Datetime.from_timestamp_utc(min_date*ticks).ymd
        now_end = Datetime.from_timestamp_utc(max_date*ticks).ymd
        if valid == 1 and now_start != startDate:
            sql = f"alter table hku_base.stock update startDate={now_start}, endDate=99999999 where market='{market}' and code='{code}'"
            connect.command(sql)
        elif valid == 0 and now_end != endDate:
            sql = f"alter table hku_base.stock update startDate={now_start}, endDate={now_end} where market='{market}' and code='{code}'"
            connect.command(sql)
        if ((code == "000001" and market == MARKET.SH)
            or (code == "399001" and market == MARKET.SZ)
                or (code == "830799" and market == MARKET.BJ)):
            sql = f"alter table hku_base.market update lastDate={now_end} where market='{market}'"
            connect.command(sql)


def clear_extern_data(connect, market, data_type):
    if data_type == 'DAY':
        index_list = ('week', 'month', 'quarter', 'halfyear', 'year')
        lastdate = connect.command(
            f"select toInt32(max(date)) from hku_data.day_k where market='SH' and code='000001'")
        for index_type in index_list:
            sql = f"delete from hku_data.{index_type}_k where market='{market}' and date>={lastdate}"
            # hku_info(
            #     f"delete from hku_data.{index_type}_k where market='{market}' and date>={Datetime.from_timestamp_utc(lastdate*1000000)}")
            connect.command(sql, settings={"mutations_sync": 0})
            # hku_run_ignore_exception(connect.command, f"OPTIMIZE TABLE hku_data.{index_type}_k", settings={
            #                          "mutations_sync": 0})
        hku_info(f"清理 {market} {data_type} 线扩展数据完毕")

    else:
        index_list = ('min15', 'min30', 'min60', 'hour2')
        lastdate = connect.command(
            f"select toInt32(max(date)) from hku_data.min5_k where market='SH' and code='000001'")
        lastdate = Datetime.from_timestamp_utc(int(lastdate*1000000)).start_of_day()
        last_timestamp = Datetime(lastdate).timestamp_utc()//1000000
        for index_type in index_list:
            sql = f"delete from hku_data.{index_type}_k where market='{market}' and date>={last_timestamp}"
            # hku_info(f"delete from hku_data.{index_type}_k where market='{market}' and date>={lastdate}")
            connect.command(sql, settings={"mutations_sync": 0})
            # hku_run_ignore_exception(connect.command, f"OPTIMIZE TABLE hku_data.{index_type}_k", settings={
            #                          "mutations_sync": 0})
        hku_info(f"清理 {market} {data_type} 线扩展数据完毕")


@hku_catch(ret=None)
def get_clickhouse_importer():
    filename = os.path.expanduser('~') + '/.hikyuu/hikyuu.ini'
    config = ConfigParser()
    config.read(filename, encoding='utf-8')
    importer = KDataToClickHouseImporter()
    return importer if importer.set_config(config.get("kdata", "host"), config.getint("kdata", "port", fallback=9000),
                                           config.get("kdata", "usr"), config.get("kdata", "pwd")) else None


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

    if ktype in ("DAY", "5MIN"):
        clear_extern_data(connect, market, ktype)
        if ktype == "DAY":
            index_list = ('week', 'month', 'quarter', 'halfyear', 'year')
            update_data = {}
            for index_type in index_list:
                update_data[index_type] = []
        elif ktype == "5MIN":
            index_list = ('min15', 'min30', 'min60', 'hour2')
            update_data = {}
            for index_type in index_list:
                update_data[index_type] = []

    failed_limit = 20
    failed_count = 0
    failed_list = []
    total = len(stock_list)
    # market, code, valid, type
    for i, stock in enumerate(stock_list):
        if stock[2] == 0 or len(stock[1]) != 6:
            if progress:
                progress(i, total)
            continue

        this_count, success, lastdate = import_one_stock_data(
            connect, api, market, ktype, stock, startDate
        )
        if not success:
            failed_count += 1
            failed_list.append((market, stock[1], lastdate))
        if failed_count >= failed_limit:
            # hku_error(f"{market} {ktype} 连续失败20个股票，已停止导入, 建议重新导入")
            break
        add_record_count += this_count
        if ktype in ("DAY", "5MIN"):
            if ktype == "DAY":
                index_data = update_extern_data(connect, market, stock[1], "DAY")
            elif ktype == "5MIN":
                index_data = update_extern_data(connect, market, stock[1], "5MIN")
            for index_type in index_list:
                update_data[index_type].extend(index_data[index_type])
                if len(update_data[index_type]) > 200000:
                    index_table = get_table(connect, market, stock[1], index_type)
                    # hku_info(f"写入 {market} {index_table[0]} 扩展数据: {len(update_data[index_type])} ...")
                    ic = connect.create_insert_context(table=index_table[0],
                                                       data=update_data[index_type])
                    connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})
                    update_data[index_type].clear()

        if progress:
            progress(i, total)

    if total > 0 and progress:
        progress(total, total)

    if ktype in ("DAY", "5MIN"):
        for index_type in index_list:
            if len(update_data[index_type]) > 0:
                index_table = get_table(connect, market, stock[1], index_type)
                # hku_info(f"写入 {market} {index_table[0]} 扩展数据: {len(update_data[index_type])} ...")
                ic = connect.create_insert_context(table=index_table[0],
                                                   data=update_data[index_type])
                connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})
                update_data[index_type].clear()
        update_data.clear()

    if 0 < failed_count < failed_limit and is_valid_license():
        # 删除最后记录
        ktype_dict = {
            'DAY': 'DAY',
            '1MIN': 'MIN',
            '5MIN': 'MIN5'
        }
        nktype = ktype_dict[ktype]
        ch_importer = get_clickhouse_importer()
        if ch_importer is not None:
            for r in failed_list:
                hku_warn(f"清理 {r[0]} {r[1]} {nktype} {r[2].start_of_day()}")
                ch_importer.remove(r[0], r[1], nktype, r[2].start_of_day())
            hku_warn(f"已清理 {market} {failed_count} 个失败股票的最后记录，建议重新导入")

    if failed_count >= failed_limit:
        hku_error(f"{market} {ktype} 连续失败20个股票，已停止导入, 建议重新导入")
        return add_record_count

    # 导入完成后检查重复数据（仅警告，不删除）- 在 OPTIMIZE 之前执行
    try:
        table_info = get_table(connect, market, '000001', ktype)
        full_table_name = table_info[0]  # 完整表名，如 'hku_data.day_k'

        # 只检查当前 market 和 ktype 的股票是否有重复（仅检查最近 5 天）
        check_sql = f"""
            SELECT
                code,
                count(distinct_dates) as duplicate_date_count,
                sum(dup_count) as total_duplicate_records
            FROM (
                SELECT
                    code,
                    `date` as distinct_dates,
                    count(*) as dup_count
                FROM {full_table_name}
                WHERE market = '{market}'
                  AND `date` >= toInt32(toUnixTimestamp(now() - INTERVAL 5 DAY))
                GROUP BY code, `date`
                HAVING count() > 1
            )
            GROUP BY code
            ORDER BY duplicate_date_count DESC, total_duplicate_records DESC
        """

        result = connect.query(check_sql)
        dup_list = result.result_rows

        if len(dup_list) > 0:
            total_dups = sum(row[2] for row in dup_list)
            hku_warn(f"\n{'='*80}")
            hku_warn(f"【重复数据警告】{market}市场 {ktype}K 线存在重复数据！（近 5 天）")
            hku_warn(f"共发现 {len(dup_list)} 只股票存在重复")
            hku_warn(f"总计 {total_dups} 条重复记录")
            hku_warn(f"\n前 20 只重复股票列表:")
            for i, row in enumerate(dup_list[:20], 1):
                hku_warn(f"  {i:2d}. {market}{row[0]}: {row[1]}个重复日期，{row[2]}条记录")
            if len(dup_list) > 20:
                hku_warn(f"  ... 还有 {len(dup_list) - 20} 只股票未显示")
            hku_warn(f"{'='*80}\n")
        else:
            hku_debug(f"{market}市场 {ktype}K 线近 5 天未发现重复数据")

    except Exception as e:
        hku_error(f"检查重复数据失败：{e}")

    cur_year = Datetime.today().year
    if ktype == "DAY":
        update_stock_info(connect, market)
        hku_info(f"更新 {market} 股票信息完毕")
        connect.command(f"OPTIMIZE TABLE hku_data.year_k PARTITION '{market}' FINAL", settings={
                        "mutations_sync": 0})
        connect.command(f"OPTIMIZE TABLE hku_data.halfyear_k PARTITION '{market}' FINAL", settings={
                        "mutations_sync": 0})
        connect.command(f"OPTIMIZE TABLE hku_data.quarter_k PARTITION '{market}' FINAL", settings={
                        "mutations_sync": 0})
        connect.command(f"OPTIMIZE TABLE hku_data.month_k PARTITION '{market}' FINAL", settings={
                        "mutations_sync": 0})
        connect.command(f"OPTIMIZE TABLE hku_data.week_k PARTITION '{market}' FINAL", settings={
                        "mutations_sync": 0})
        connect.command(f"OPTIMIZE TABLE hku_data.day_k PARTITION '{market}' FINAL", settings={
                        "mutations_sync": 0})
        hku_info(f"优化 {market} 日线及扩展表数据完毕")
    if ktype == "5MIN":
        hku_run_ignore_exception(connect.command, f"OPTIMIZE TABLE hku_data.hour2_k PARTITION '{market}' FINAL", settings={
            "mutations_sync": 0, "optimize_skip_merged_partitions": 1})
        hku_run_ignore_exception(connect.command,
                                 f"OPTIMIZE TABLE hku_data.min30_k PARTITION ('{market}', {cur_year - cur_year % 10}) FINAL", settings={"mutations_sync": 0, "optimize_skip_merged_partitions": 1})
        hku_run_ignore_exception(connect.command,
                                 f"OPTIMIZE TABLE hku_data.min15_k PARTITION ('{market}', {cur_year - cur_year % 10}) FINAL", settings={"mutations_sync": 0, "optimize_skip_merged_partitions": 1})
        hku_run_ignore_exception(connect.command,
                                 f"OPTIMIZE TABLE hku_data.min5_k PARTITION ('{market}', {cur_year - cur_year % 10}) FINAL", settings={"mutations_sync": 0, "optimize_skip_merged_partitions": 1})
        hku_run_ignore_exception(connect.command, f"OPTIMIZE TABLE hku_data.min60_k PARTITION '{market}' FINAL", settings={
            "mutations_sync": 0, "optimize_skip_merged_partitions": 1})
        hku_info(f"优化 {market} 5分钟线及扩展表完毕")
    if ktype == "1MIN":
        hku_run_ignore_exception(connect.command,
                                 f"OPTIMIZE TABLE hku_data.min_k PARTITION ('{market}', {cur_year - cur_year % 10}) FINAL", settings={"mutations_sync": 0, "optimize_skip_merged_partitions": 1})
        hku_info(f"优化 {market} 1 分钟线表数据完毕")

    return add_record_count


@hku_catch(trace=True, re_raise=True)
def get_trans_table(connect, market, code):
    return get_table(connect, market, code, 'transdata')


@hku_catch(ret=[], trace=True)
def import_on_stock_trans(connect, api, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    market, code, valid, stktype = stock_record
    hku_debug("{}{}".format(market, code))
    table = get_trans_table(connect, market, code)
    last_datetime = get_lastdatetime(connect, table)

    today = datetime.date.today()
    if last_datetime is not None:
        # yyyymmddHHMMSS
        last_y = last_datetime.year
        last_m = last_datetime.month
        last_d = last_datetime.day
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
    # 用于去重的集合，记录已处理的 datetime
    seen_datetimes = set()

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

                # 构建完整的 datetime 值作为去重键值
                bar_datetime = cur_date * 1000000 + minute * 100 + second

                if bar_datetime not in seen_datetimes and record['price'] > 0.0 and record['vol'] >= 0.0:
                    trans_buf.append(
                        (
                            market, code,
                            Datetime(bar_datetime).timestamp_utc()//1000000,
                            int(roundEx(record["price"], 3) * 1000.0),
                            int(record["vol"]),
                            record["buyorsell"],
                        )
                    )
                    # 标记为已处理
                    seen_datetimes.add(bar_datetime)
            except Exception as e:
                hku_error("Failed trans to record! {}", e)

    return trans_buf


def import_trans(
    connect, market, quotations, api, dest_dir, max_days=30, progress=ProgressBar
):
    add_record_count = 0
    market = market.upper()

    stock_list = get_stock_list(connect, market, quotations)
    total = len(stock_list)
    a_stktype_list = get_a_stktype_list()
    buf = []
    for i, stock in enumerate(stock_list):
        market, code, valid, stype = stock
        if valid == 0 or len(code) != 6 or stype not in a_stktype_list:
            if progress:
                progress(i, total)
            continue

        data = import_on_stock_trans(connect, api, market, stock, max_days)
        add_record_count += len(data)
        if len(data) > 0:
            buf.extend(data)
            if len(buf) > 500000:
                table = get_trans_table(connect, market, code)
                ic = connect.create_insert_context(table=table[0], data=buf)
                connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})
                buf.clear()
                # hku_info(f"写入 {market} {table[0]} 分笔数据: {len(buf)} ...")

        if progress:
            progress(i, total)

    if len(buf) > 0:
        ic = connect.create_insert_context(table='hku_data.transdata', data=buf)
        connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})
        buf.clear()
        # hku_info(f"写入 {market} hku_data.transdata 分笔数据：{len(buf)} ...")

    # 导入完成后检查重复数据（仅警告，不删除）- 在 OPTIMIZE 之前执行
    try:
        table_info = get_trans_table(connect, market, '000001')
        full_table_name = table_info[0]  # 完整表名，如 'hku_data.transdata'

        # 只检查当前 market 的股票是否有重复（仅检查最近 5 天）
        check_sql = f"""
            SELECT
                code,
                count(distinct_dates) as duplicate_datetime_count,
                sum(dup_count) as total_duplicate_records
            FROM (
                SELECT
                    code,
                    `date` as distinct_dates,
                    count(*) as dup_count
                FROM {full_table_name}
                WHERE market = '{market}'
                  AND `date` >= toInt32(toUnixTimestamp(now() - INTERVAL 5 DAY))
                GROUP BY code, `date`
                HAVING count() > 1
            )
            GROUP BY code
            ORDER BY duplicate_datetime_count DESC, total_duplicate_records DESC
        """

        result = connect.query(check_sql)
        dup_list = result.result_rows

        if len(dup_list) > 0:
            total_dups = sum(row[2] for row in dup_list)
            hku_warn(f"\n{'='*80}")
            hku_warn(f"【重复数据警告】{market}市场 分笔数据存在重复数据！（近 5 天）")
            hku_warn(f"共发现 {len(dup_list)} 只股票存在重复")
            hku_warn(f"总计 {total_dups} 条重复记录")
            hku_warn(f"\n前 20 只重复股票列表:")
            for i, row in enumerate(dup_list[:20], 1):
                hku_warn(f"  {i:2d}. {market}{row[0]}: {row[1]}个重复时间点，{row[2]}条记录")
            if len(dup_list) > 20:
                hku_warn(f"  ... 还有 {len(dup_list) - 20} 只股票未显示")
            hku_warn(f"{'='*80}\n")
        else:
            hku_debug(f"{market}市场 分笔数据近 5 天未发现重复数据")

    except Exception as e:
        hku_error(f"检查重复数据失败：{e}")

    cur_year = Datetime.today().year
    hku_run_ignore_exception(connect.command,
                             f"OPTIMIZE TABLE hku_data.transdata PARTITION ('{market}', {cur_year}) FINAL", settings={"mutations_sync": 0, "optimize_skip_merged_partitions": 1})
    return add_record_count


def get_time_table(connect, market, code):
    return get_table(connect, market, code, 'timeline')


@hku_catch(ret=[], trace=True)
def import_on_stock_time(connect, api, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    market, code, valid, type = stock_record
    hku_debug("{}{}".format(market, code))
    table = get_time_table(connect, market, code)
    last_datetime = get_lastdatetime(connect, table)

    today = datetime.date.today()
    if last_datetime is not None:
        # yyyymmddHHMM
        last_y = last_datetime.year
        last_m = last_datetime.month
        last_d = last_datetime.day
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

    ticks = 1000000
    time_buf = []
    # 用于去重的集合，记录已处理的 datetime
    seen_datetimes = set()

    for cur_date in date_list:
        buf = api.get_history_minute_time_data(pytdx_market, code, cur_date)
        if buf is None or len(buf) != 240:
            # print(cur_date, "获取的分时线长度不为 240!", stock_record[1], stock_record[2])
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
                if record['price'] > 0.0 and record['vol'] >= 0.0:
                    # 构建完整的 datetime 值作为去重键值
                    bar_datetime = this_date + time

                    # 检查是否重复
                    if bar_datetime not in seen_datetimes:
                        time_buf.append((market, code, Datetime(bar_datetime).timestamp_utc() //
                                         ticks, int(roundEx(record['price'], 3) * 1000.0), int(record['vol'])))
                        # 标记为已处理
                        seen_datetimes.add(bar_datetime)
                time += 1
            except Exception as e:
                hku_error("Failed trans record {}! {}".format(record, e))

    return time_buf


def import_time(connect, market, quotations, api, dest_dir, max_days=9000, progress=ProgressBar):
    add_record_count = 0
    market = market.upper()

    # market, code, valid, type
    stock_list = get_stock_list(connect, market, quotations)
    total = len(stock_list)
    buf = []
    for i, stock in enumerate(stock_list):
        market, code, valid, stype = stock
        if valid == 0 or len(code) != 6:
            if progress:
                progress(i, total)
            continue

        data = import_on_stock_time(connect, api, market, stock, max_days)
        add_record_count += len(data)
        if len(data) > 0:
            buf.extend(data)
            if len(buf) > 500000:
                table = get_time_table(connect, market, code)
                ic = connect.create_insert_context(table=table[0], data=buf)
                connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})
                buf.clear()
                # hku_info(f"写入 {market} {table[0]} 分时数据: {len(buf)} ...")

        if progress:
            progress(i, total)

    if len(buf) > 0:
        ic = connect.create_insert_context(table='hku_data.timeline', data=buf)
        connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})
        buf.clear()
        # hku_info(f"写入 {market} hku_data.timeline 分时数据：{len(buf)} ...")

    # 导入完成后检查重复数据（仅警告，不删除）- 在 OPTIMIZE 之前执行
    try:
        table_info = get_time_table(connect, market, '000001')
        full_table_name = table_info[0]  # 完整表名，如 'hku_data.timeline'

        check_sql = f"""
            SELECT 
                code, 
                count(distinct_dates) as duplicate_datetime_count,
                sum(dup_count) as total_duplicate_records
            FROM (
                SELECT 
                    code, 
                    `date` as distinct_dates,
                    count(*) as dup_count
                FROM {full_table_name}
                WHERE market = '{market}'
                  AND `date` >= toInt32(toUnixTimestamp(now() - INTERVAL 5 DAY))
                GROUP BY code, `date`
                HAVING count() > 1
            )
            GROUP BY code
            ORDER BY duplicate_datetime_count DESC, total_duplicate_records DESC
        """

        result = connect.query(check_sql)
        dup_list = result.result_rows

        if len(dup_list) > 0:
            total_dups = sum(row[2] for row in dup_list)
            hku_warn(f"\n{'='*80}")
            hku_warn(f"【重复数据警告】{market}市场 分时数据存在重复数据！（近 5 天）")
            hku_warn(f"共发现 {len(dup_list)} 只股票存在重复")
            hku_warn(f"总计 {total_dups} 条重复记录")
            hku_warn(f"\n前 20 只重复股票列表:")
            for i, row in enumerate(dup_list[:20], 1):
                hku_warn(f"  {i:2d}. {market}{row[0]}: {row[1]}个重复时间点，{row[2]}条记录")
            if len(dup_list) > 20:
                hku_warn(f"  ... 还有 {len(dup_list) - 20} 只股票未显示")
            hku_warn(f"{'='*80}\n")
        else:
            hku_debug(f"{market}市场 分时数据近 5 天未发现重复数据")

    except Exception as e:
        hku_error(f"检查重复数据失败：{e}")

    cur_year = Datetime.today().year
    hku_run_ignore_exception(connect.command,
                             f"OPTIMIZE TABLE hku_data.timeline PARTITION ('{market}', {cur_year - cur_year % 10}) FINAL", settings={"mutations_sync": 0, "optimize_skip_merged_partitions": 1})
    return add_record_count


if __name__ == "__main__":
    import os
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    db = 'clickhouse54-http'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    import clickhouse_connect
    client = clickhouse_connect.get_client(
        host=host, username=user, password=password)

    tdx_server = "180.101.48.170"
    tdx_port = 7709
    quotations = ["stock", "fund"]

    create_database(client)

    from pytdx.hq import TdxHq_API, TDXParams

    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    import time
    starttime = time.time()

    add_count = 0

    # print("导入股票代码表")
    # # add_count = import_index_name(client)
    # add_count = import_stock_name(client, api, 'SH', quotations)
    # add_count += import_stock_name(client, api, 'SZ', quotations)
    # add_count += import_stock_name(client, api, 'BJ', quotations)
    # print("新增股票数：", add_count)

    print("\n导入上证日线数据")
    # add_count = import_data(client, "SH", "DAY", quotations, api, "", progress=ProgressBar)
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
    client.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
