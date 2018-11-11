# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog/hikyuu
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

from common import MARKETID, STOCKTYPE, get_stktype_list
from sqlite3_common import (get_codepre_list, create_database,
                            get_marketid, get_last_date,
                            get_stock_list, update_last_date)
from h5_common import (H5Record, H5Index,
                       open_h5file, get_h5table,
                       update_hdf5_extern_data)


def ProgressBar(cur, total):
    percent = '{:.0%}'.format(cur / total)
    sys.stdout.write('\r')
    sys.stdout.write("[%-50s] %s" % ('=' * int(math.floor(cur * 50 / total)),percent))
    sys.stdout.flush()


def to_pytdx_market(market):
    """转换为pytdx的market"""
    pytdx_market = {'SH': TDXParams.MARKET_SH, 'SZ': TDXParams.MARKET_SZ}
    return pytdx_market[market.upper()]


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
    pytdx_market = to_pytdx_market(market.upper())
    stk_count = api.get_security_count(pytdx_market)

    for i in range(int(stk_count/1000)+1):
        stock_list = api.get_security_list(pytdx_market, i * 1000)
        for stock in stock_list:
            newStockDict[stock['code']] = stock['name']

    marketid = get_marketid(connect, market)

    stktype_list = get_stktype_list(quotations)
    a = cur.execute("select stockid, code, name, valid from stock where marketid={} and type in {}"
                    .format(marketid, stktype_list))
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
                cur.execute("update stock set name='%s' where stockid=%i" %
                            (newStockDict[oldcode], oldstockid))
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
                    #print(market, code, newStockDict[code], codepre)
                    sql = "insert into Stock(marketid, code, name, type, valid, startDate, endDate) \
                           values (%s, '%s', '%s', %s, %s, %s, %s)" \
                          % (marketid, code, newStockDict[code], codepre[1], 1, today, 99999999)
                    cur.execute(sql)
                    break

    #print('%s新增股票数：%i' % (market.upper(), count))
    connect.commit()
    cur.close()


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


def import_one_stock_data(connect, api, h5file, market, ktype, stock_record):
    market = market.upper()
    pytdx_market = to_pytdx_market(market)

    stockid, marketid, code, valid, stktype = stock_record[0], stock_record[1], stock_record[2], stock_record[3], \
                                              stock_record[4]

    table = get_h5table(h5file, market, code)
    last_datetime = table[-1]['datetime'] if table.nrows > 0 else 199012191500

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
            #print(code, "invalid!!")
            continue

        for bar in bar_list:
            try:
                tmp = datetime.date(bar['year'], bar['month'], bar['day'])
                bar_datetime = (tmp.year * 10000 + tmp.month * 100 + tmp.day) * 10000
                if ktype != 'DAY':
                    bar_datetime += bar['hour'] * 100 + bar['minute']
            except:
                continue

            if today_datetime >= bar_datetime > last_datetime \
                    and bar['high'] >= bar['open'] >= bar['low'] > 0 \
                    and bar['high'] >= bar['close'] >= bar['low'] > 0 \
                    and bar['vol'] != 0 and bar['amount'] != 0:
                row['datetime'] = bar_datetime
                row['openPrice'] = bar['open'] * 1000
                row['highPrice'] = bar['high'] * 1000
                row['lowPrice'] = bar['low'] * 1000
                row['closePrice'] = bar['close'] * 1000
                row['transAmount'] = bar['amount'] * 0.001
                row['transCount'] = bar['vol']
                row.append()
                add_record_count += 1

    if add_record_count > 0:
        table.flush()
        #print(market, stock_record)

        if ktype == 'DAY':
            # 更新基础信息数据库中股票对应的起止日期及其有效标志
            if valid == 0:
                cur = connect.cursor()
                cur.execute("update stock set valid=1, startdate=%i, enddate=%i where stockid=%i" %
                            (table[0]['datetime'], 99999999, stockid))
                connect.commit()
                cur.close()

            # 记录最新更新日期
            if (code == '000001' and marketid == MARKETID.SH) \
                    or (code == '399001' and marketid == MARKETID.SZ):
                update_last_date(connect, marketid, table[-1]['datetime'] / 10000)

    return add_record_count


def import_data(connect, market, ktype, quotations, api, dest_dir, progress=ProgressBar):
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

    total = len(stock_list)
    for i, stock in enumerate(stock_list):
        if stock[3] == 0:
            continue

        this_count = import_one_stock_data(connect, api, h5file, market, ktype, stock)
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


if __name__ == '__main__':
    import time
    starttime = time.time()

    dest_dir = "c:\\stock"
    tdx_server = '119.147.212.81'
    tdx_port = 7709
    quotations = ['stock', 'fund', 'bond']

    connect = sqlite3.connect(dest_dir + "\\stock.db")
    create_database(connect)

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    print("导入股票代码表")
    import_stock_name(connect, api, 'SH', quotations)
    import_stock_name(connect, api, 'SZ', quotations)

    add_count = 0

    print("\n导入上证日线数据")
    add_count = import_data(connect, 'SH', 'DAY', ['bond'], api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)

    print("\n导入深证日线数据")
    #add_count = import_data(connect, 'SZ', 'DAY', ['stock'], api, dest_dir, progress=ProgressBar)
    print("\n导入数量：", add_count)


    api.disconnect()

    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))

