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
import sqlite3
from pytdx.hq import TDXParams

from hikyuu.util.mylog import get_default_logger, hku_error, hku_debug

from hikyuu.data.common import *
from hikyuu.data.common_pytdx import to_pytdx_market, pytdx_get_day_trans
from hikyuu.data.common_sqlite3 import (
    get_codepre_list, create_database, get_marketid, get_last_date, get_stock_list, update_last_date
)
from hikyuu.data.common_h5 import (
    H5Record, H5Index, open_h5file, get_h5table, update_hdf5_extern_data, open_trans_file, get_trans_table,
    update_hdf5_trans_index, open_time_file, get_time_table
)
from hikyuu.data.weight_to_sqlite import qianlong_import_weight


def ProgressBar(cur, total):
    percent = '{:.0%}'.format(cur / total)
    sys.stdout.write('\r')
    sys.stdout.write("[%-50s] %s" % ('=' * int(math.floor(cur * 50 / total)), percent))
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
    a = cur.execute("select stockid, marketid, code from stock where type={}".format(STOCKTYPE.INDEX))
    a = a.fetchall()
    oldStockDict = {}
    for oldstock in a:
        oldstockid = oldstock[0]
        oldcode = "{}{}".format(MARKET.SH if oldstock[1] == MARKETID.SH else MARKET.SZ, oldstock[2])
        oldStockDict[oldcode] = oldstockid

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    for index in index_list:
        if index['market_code'] in oldStockDict:
            cur.execute(
                "update stock set valid=1, name='%s' where stockid=%i" %
                (index['name'], oldStockDict[index['market_code']])
            )
        else:
            marketid = MARKETID.SH if index['market_code'][:2] == MARKET.SH else MARKETID.SZ
            sql = "insert into Stock(marketid, code, name, type, valid, startDate, endDate) \
                           values (%s, '%s', '%s', %s, %s, %s, %s)" \
                          % (marketid, index['market_code'][2:], index['name'], STOCKTYPE.INDEX, 1, today, 99999999)
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

    newStockDict = {}
    stk_list = get_stk_code_name_list(market)
    if not stk_list:
        hku_error("获取 {} 股票代码表失败", market)
        return 0

    if not quotations or 'fund' in [v.lower() for v in quotations]:
        stk_list.extend(get_fund_code_name_list(market))
    for stock in stk_list:
        newStockDict[str(stock['code'])] = stock['name']
    if market == MARKET.SH:
        df = ak.stock_info_sh_delist()
        l = df[['公司代码', '公司简称']].to_dict(orient='records') if not df .empty else []
        for stock in l:
            newStockDict[str(stock['公司代码'])] = stock['公司简称']
    elif market == MARKET.SZ:
        for t in ['暂停上市公司', '终止上市公司']:
            df = ak.stock_info_sz_delist(t)
            l = df[['证券代码', '证券简称']].to_dict(orient='records') if not df.empty else []
            for stock in l:
                newStockDict[str(stock['证券代码'])] = stock['证券简称']

    marketid = get_marketid(connect, market)

    stktype_list = get_stktype_list(quotations)
    stktype_list = list(stktype_list)
    stktype_list.remove(STOCKTYPE.INDEX)  # 移除指数类型
    stktype_list = tuple(stktype_list)
    a = cur.execute(
        "select stockid, code, name, valid from stock where marketid={} and type in {}".format(marketid, stktype_list)
    )
    a = a.fetchall()
    oldStockDict = {}
    for oldstock in a:
        oldstockid, oldcode, oldname, oldvalid = oldstock[0], oldstock[1], oldstock[2], int(oldstock[3])
        oldStockDict[oldcode] = oldstockid

        # 新的代码表中无此股票，则置为无效
        if (oldvalid == 1) and (oldcode not in newStockDict):
            cur.execute("update stock set valid=0 where stockid=%i" % oldstockid)

        # 股票名称发生变化，更新股票名称;如果原无效，则置为有效
        if oldcode in newStockDict:
            if oldname != newStockDict[oldcode]:
                cur.execute("update stock set name='%s' where stockid=%i" % (newStockDict[oldcode], oldstockid))
            if oldvalid == 0:
                cur.execute("update stock set valid=1, endDate=99999999 where stockid=%i" % oldstockid)

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
                    sql = "insert into Stock(marketid, code, name, type, valid, startDate, endDate) \
                           values (%s, '%s', '%s', %s, %s, %s, %s)" \
                          % (marketid, code, newStockDict[code], codepre[1], 1, today, 99999999)
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


def import_one_stock_data(connect, api, h5file, market, ktype, stock_record, startDate=199012191500):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    stockid, marketid, code, valid, stktype = stock_record[0], stock_record[1], stock_record[2], stock_record[3], \
        stock_record[4]

    hku_debug("{}{} {}".format(market, code, ktype))
    table = get_h5table(h5file, market, code)
    if table is None:
        hku_error("Can't get table({}{})".format(market, code))
        return 0

    last_datetime = table[-1]['datetime'] if table.nrows > 0 else startDate

    today = datetime.date.today()
    if ktype == 'DAY':
        n, step = guess_day_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_RI_K
        today_datetime = (today.year * 10000 + today.month * 100 + today.day) * 10000

    elif ktype == '1MIN':
        n, step = guess_1min_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_1MIN
        today_datetime = (today.year * 10000 + today.month * 100 + today.day) * 10000 + 1500

    elif ktype == '5MIN':
        n, step = guess_5min_n_step(last_datetime)
        pytdx_kline_type = TDXParams.KLINE_TYPE_5MIN
        today_datetime = (today.year * 10000 + today.month * 100 + today.day) * 10000 + 1500
    else:
        return 0

    if today_datetime <= last_datetime:
        return 0

    get_bars = api.get_index_bars if stktype == STOCKTYPE.INDEX else api.get_security_bars

    add_record_count = 0

    row = table.row
    while n >= 0:
        bar_list = get_bars(pytdx_kline_type, pytdx_market, code, n * 800, step)
        n -= 1
        if bar_list is None:
            # print(code, "invalid!!")
            continue

        for bar in bar_list:
            try:
                tmp = datetime.date(bar['year'], bar['month'], bar['day'])
                bar_datetime = (tmp.year * 10000 + tmp.month * 100 + tmp.day) * 10000
                if ktype != 'DAY':
                    bar_datetime += bar['hour'] * 100 + bar['minute']
            except Exception as e:
                hku_error("Failed translate datetime: {}! {}".format(bar, e))
                continue

            if today_datetime >= bar_datetime > last_datetime \
                    and bar['high'] >= bar['open'] >= bar['low'] > 0 \
                    and bar['high'] >= bar['close'] >= bar['low'] > 0 \
                    and int(bar['vol']) >= 0 and int(bar['amount']*0.001) >= 0:
                try:
                    row['datetime'] = bar_datetime
                    row['openPrice'] = bar['open'] * 1000
                    row['highPrice'] = bar['high'] * 1000
                    row['lowPrice'] = bar['low'] * 1000
                    row['closePrice'] = bar['close'] * 1000
                    row['transAmount'] = round(bar['amount'] * 0.001)
                    row['transCount'] = round(bar['vol'])
                    row.append()
                    add_record_count += 1
                except Exception as e:
                    hku_error("Can't trans record: {}! {}".format(bar, e))
                last_datetime = bar_datetime

    if add_record_count > 0:
        table.flush()
        # print(market, stock_record)

        if ktype == 'DAY':
            # 更新基础信息数据库中股票对应的起止日期及其有效标志
            # if valid == 0:
            cur = connect.cursor()
            cur.execute(
                "update stock set valid=1, startdate=%i, enddate=%i where stockid=%i" %
                (table[0]['datetime'] / 10000, 99999999, stockid)
            )
            connect.commit()
            cur.close()

            # 记录最新更新日期
            if (code == '000001' and marketid == MARKETID.SH) \
                    or (code == '399001' and marketid == MARKETID.SZ) \
                    or (code == '830799' and marketid == MARKETID.BJ):
                update_last_date(connect, marketid, table[-1]['datetime'] / 10000)

    elif table.nrows == 0:
        # print(market, stock_record)
        table.remove()

    # table.close()
    return add_record_count


def import_data(connect, market, ktype, quotations, api, dest_dir, startDate=199012190000, progress=ProgressBar):
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
    h5file = open_h5file(dest_dir, market, ktype)

    stock_list = get_stock_list(connect, market, quotations)
    if not stock_list:
        return 0

    total = len(stock_list)
    for i, stock in enumerate(stock_list):
        if stock[3] == 0 or len(stock[2]) != 6:
            if progress:
                progress(i, total)
            continue

        this_count = import_one_stock_data(connect, api, h5file, market, ktype, stock, startDate)
        add_record_count += this_count
        if this_count > 0:
            if ktype == 'DAY':
                update_hdf5_extern_data(h5file, market.upper() + stock[2], 'DAY')
            elif ktype == '5MIN':
                update_hdf5_extern_data(h5file, market.upper() + stock[2], '5MIN')
        if progress:
            progress(i, total)

    connect.commit()
    h5file.close()
    return add_record_count


def import_on_stock_trans(connect, api, h5file, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    stockid, marketid, code, valid, stktype = stock_record[0], stock_record[1], stock_record[2], stock_record[3], \
        stock_record[4]
    hku_debug("{}{}".format(market, code))
    table = get_trans_table(h5file, market, code)
    if table is None:
        hku_error("Failed get_trans_table({}{})!".format(market, code))
        return 0

    today = datetime.date.today()
    if table.nrows > 0:
        try:
            last_datetime = int(table[-1]['datetime'] // 1000000)
            last_y = int(last_datetime // 10000)
            last_m = int(last_datetime // 100 - last_y * 100)
            last_d = int(last_datetime - (last_y * 10000 + last_m * 100))
            last_date = datetime.date(last_y, last_m, last_d)
            need_days = (today - last_date).days
        except Exception as e:
            hku_error("Failed get last date from hdf5({}{}), remove this table! {}".format(market, code, e))
            table.remove()
            return 0
    else:
        need_days = max_days

    date_list = []
    for i in range(need_days):
        cur_date = today - datetime.timedelta(i)
        if cur_date.weekday() not in (5, 6):
            date_list.append(cur_date.year * 10000 + cur_date.month * 100 + cur_date.day)
    date_list.reverse()

    add_record_count = 0

    row = table.row
    for cur_date in date_list:
        buf = pytdx_get_day_trans(api, pytdx_market, stock_record[2], cur_date)
        if not buf:
            continue

        second = 2
        pre_minute = 900

        for record in buf:
            try:
                minute = int(record['time'][0:2]) * 100 + int(record['time'][3:])
                if minute != pre_minute:
                    second = 0 if minute == 1500 else 2
                    pre_minute = minute
                else:
                    second += 3
                if second > 59:
                    continue
                row['datetime'] = cur_date * 1000000 + minute * 100 + second
                row['price'] = int(record['price'] * 1000)
                row['vol'] = record['vol']
                row['buyorsell'] = record['buyorsell']
                row.append()
                add_record_count += 1
            except Exception as e:
                hku_error("Failed trans to record! {}", e)

    if add_record_count > 0:
        table.flush()
    elif table.nrows == 0:
        table.remove()

    return add_record_count


def import_trans(connect, market, quotations, api, dest_dir, max_days=30, progress=ProgressBar):
    add_record_count = 0
    market = market.upper()
    h5file = open_trans_file(dest_dir, market)

    stock_list = get_stock_list(connect, market, quotations)

    total = len(stock_list)
    a_stktype_list = get_a_stktype_list()
    for i, stock in enumerate(stock_list):
        if stock[3] == 0 or len(stock[2]) != 6 or stock[4] not in a_stktype_list:
            if progress:
                progress(i, total)
            continue

        this_count = import_on_stock_trans(connect, api, h5file, market, stock, max_days)
        add_record_count += this_count
        # if add_record_count > 0:
        #    update_hdf5_trans_index(h5file, market.upper() + stock[2])
        if progress:
            progress(i, total)

    connect.commit()
    h5file.close()
    return add_record_count


def import_on_stock_time(connect, api, h5file, market, stock_record, max_days):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    stockid, marketid, code, valid, stktype = stock_record[0], stock_record[1], stock_record[2], stock_record[3], \
        stock_record[4]
    hku_debug("{}{}".format(market, code))
    table = get_time_table(h5file, market, code)
    if table is None:
        hku_error("Can't get table({}{})!".format(market, code))
        return 0

    today = datetime.date.today()
    if table.nrows > 0:
        try:
            last_datetime = int(table[-1]['datetime'] // 10000)
            last_y = int(last_datetime // 10000)
            last_m = int(last_datetime // 100 - last_y * 100)
            last_d = int(last_datetime - (last_y * 10000 + last_m * 100))
            last_date = datetime.date(last_y, last_m, last_d)
            need_days = (today - last_date).days
        except Exception as e:
            hku_error("Failed get last date from hdf5({}{}), remove this table! {}".format(market, code, e))
            table.remove()
            return 0
    else:
        need_days = max_days

    date_list = []
    for i in range(need_days):
        cur_date = today - datetime.timedelta(i)
        if cur_date.weekday() not in (5, 6):
            date_list.append(cur_date.year * 10000 + cur_date.month * 100 + cur_date.day)
    date_list.reverse()

    add_record_count = 0

    row = table.row
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
                row['datetime'] = this_date + time
                row['price'] = int(record['price'] * 1000)
                row['vol'] = record['vol']
                row.append()
                time += 1
                add_record_count += 1
            except Exception as e:
                hku_error("Failed trans record {}! {}".format(record, e))

    if add_record_count > 0:
        table.flush()
    elif table.nrows == 0:
        table.remove()

    return add_record_count


def import_time(connect, market, quotations, api, dest_dir, max_days=9000, progress=ProgressBar):
    add_record_count = 0
    market = market.upper()
    h5file = open_time_file(dest_dir, market)

    stock_list = get_stock_list(connect, market, quotations)

    total = len(stock_list)
    for i, stock in enumerate(stock_list):
        if stock[3] == 0 or len(stock[2]) != 6:
            if progress:
                progress(i, total)
            continue

        this_count = import_on_stock_time(connect, api, h5file, market, stock, max_days)
        add_record_count += this_count
        if progress:
            progress(i, total)

    connect.commit()
    h5file.close()
    return add_record_count


if __name__ == '__main__':
    import os
    import time
    starttime = time.time()

    dest_dir = "d:\\stock"
    tdx_server = '119.147.212.81'
    tdx_port = 7709
    quotations = ['stock', 'fund']

    connect = sqlite3.connect(dest_dir + "\\stock.db")
    create_database(connect)

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    add_count = 0
    """
    print("导入指数代码表")
    add_count = import_index_name(connect)
    print("指数个数：", add_count)
    """
    print("导入股票代码表")
    add_count = import_stock_name(connect, api, 'SH', quotations)
    # add_count += import_stock_name(connect, api, 'SZ', quotations)
    print("新增股票数：", add_count)
    """
    print("\n导入上证日线数据")
    add_count = import_data(connect, 'SH', 'DAY', quotations, api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)

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

    print("\n导入上证分笔数据")
    #add_count = import_trans(connect, 'SH', quotations, api, dest_dir, max_days=2, progress=ProgressBar)
    print("\n导入数量：", add_count)


    print("\n导入上证分时数据")
    add_count = import_time(connect, 'SZ', quotations, api, dest_dir, max_days=3, progress=ProgressBar)
    print("\n导入数量：", add_count)
    """

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
