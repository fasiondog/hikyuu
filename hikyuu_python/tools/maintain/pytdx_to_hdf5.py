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

from common import get_stktype_list
from sqlite3_common import (get_codepre_list, create_database,
                            get_marketid, get_last_date, get_stock_list)
from hdf5_common import (H5Record, H5Index,
                         open_h5file, get_h5table)


def ProgressBar(cur, total):
    percent = '{:.0%}'.format(cur / total)
    sys.stdout.write('\r')
    sys.stdout.write("[%-50s] %s" % ('=' * int(math.floor(cur * 50 / total)),percent))
    sys.stdout.flush()


def to_pytdx_market(market):
    """转换为pytdx的market"""
    pytdx_market = {'SH': TDXParams.MARKET_SH, 'SZ': TDXParams.MARKET_SZ}
    return pytdx_market[market.upper()]

pytdx_market = {'SH': TDXParams.MARKET_SH, 'SZ': TDXParams.MARKET_SZ}


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


def import_one_stock_day_data(connect, api, h5file, market, code):
    market = market.upper()
    table = get_h5table(h5file, market, code)
    if table.nrows > 0:
        start_date = table[0]['datetime'] // 10000
        last_date = table[-1]['datetime'] // 10000
    else:
        start_date = None
        last_date = 19901219

    today = datetime.date.today()

    last_y = last_date // 10000
    ny = today.year - last_y
    days = (ny + 1) * 250
    step = 800

    if days < step:
        last_m = last_date // 100 - last_y * 100
        last_d = last_date - (last_y * 10000 + last_m * 100)
        days = (today - datetime.date(int(last_y), int(last_m), int(last_d))).days
        step = days

    print(code, days)
    add_record_count = 0
    while days > 0:
        x = api.get_security_bars(9, to_pytdx_market(market), '000001', days, step)
        days -= step
        if x is not None:
            add_record_count += len(x)

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
            print(stock[2], 'invalid!!!!')
            continue
        this_count = import_one_stock_day_data(connect, api, h5file, market, stock[2])
        add_record_count += this_count
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
    #import_stock_name(connect, api, 'SH', quotations)
    #import_stock_name(connect, api, 'SZ', quotations)

    print("\n导入上证日线数据")
    add_count = 0
    add_count = import_data(connect, 'SH', 'DAY', ['stock'], api, dest_dir, progress=None)
    print(add_count)

    """x = get_last_date(connect, 1)
    print(x)
    today = datetime.date.today()
    y = x // 10000
    m = x // 100 - y * 100
    d = x - (y*10000+m*100)
    ny = today.year - y
    #days = (today - datetime.date(y, m, d)).days
    days = (ny+1)*250
    i = 0
    x = []
    step = 800
    if days <= step:
        days = (today - datetime.date(2018, 10, 29)).days
        step =days
        #print(days)
        #x = api.get_security_bars(9, 0, '000001', 0, days)
        #print(len(x))
        #print(x[0])
        #print(x[-1])
    while days > 0:
        x += api.get_security_bars(9, 0, '000001', days, step)
        days -= 800
        i+=1
        print(i ,len(x))

    #for i in x:
    #    print(i)
    
    """
    #x = api.get_security_bars(9, 1, '000001', 6500, 800)
    #print(len(x))
    #print(x[0])

    #print(api.get_security_list(1, 0))

    api.disconnect()

    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))

