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

from pytdx.hq import TDXParams


def to_pytdx_market(market):
    """转换为pytdx的market"""
    pytdx_market = {'SH': TDXParams.MARKET_SH, 'SZ': TDXParams.MARKET_SZ}
    return pytdx_market[market.upper()]


def pytdx_import_weight_to_sqlite(pytdx_api, connect, market):
    """导入钱龙格式的权息数据"""
    cur = connect.cursor()
    marketid = cur.execute("select marketid from Market where market='%s'" % market)
    marketid = [id[0] for id in marketid]
    marketid = marketid[0]
    pytdx_market = to_pytdx_market(market)

    total_count = 0
    stockid_list = cur.execute("select stockid, code from Stock where marketid=%s" % (marketid))
    stockid_list = [x for x in stockid_list]
    cur.close()

    for stockrecord in stockid_list:
        stockid, code = stockrecord
        #print("{}{}".format(market, code))

        # 获取当前数据库中最后的一条权息记录的总股本和流通股本
        cur = connect.cursor()
        a = cur.execute(
            "select date, totalCount, \
                         freeCount from stkweight where stockid=%s \
                         order by date desc limit 1" % stockid
        )
        a = [x for x in a]
        if a:
            last_date, last_total_count, last_free_count = a[0]
        else:
            last_date, last_total_count, last_free_count = (0, 0, 0)
        cur.close()

        xdxr_list = pytdx_api.get_xdxr_info(pytdx_market, code)
        records = {}
        for xdxr in xdxr_list:
            date = xdxr['year'] * 1000 + xdxr['month'] * 100 + xdxr['day']
            if date < last_date:
                continue
            if date not in records:
                records[date] = [
                    stockid,
                    date,
                    int(10000 * xdxr['songzhuangu'])
                    if xdxr['songzhuangu'] is not None else 0,  #countAsGift
                    int(10000 * xdxr['peigu']) if xdxr['peigu'] is not None else 0,  #countForSell
                    int(1000 *
                        xdxr['peigujia']) if xdxr['peigujia'] is not None else 0,  #priceForSell
                    int(1000 * xdxr['fenhong']) if xdxr['fenhong'] is not None else 0,  #bonus
                    0,  #countOfIncreasement, pytdx 不区分送股和转增股，统一记在送股
                    round(xdxr['houzongguben'])
                    if xdxr['houzongguben'] is not None else last_total_count,  #totalCount
                    round(xdxr['panhouliutong'])
                    if xdxr['panhouliutong'] is not None else last_free_count  #freeCount
                ]
            else:
                if xdxr['songzhuangu'] is not None:
                    records[date][2] = int(10000 * xdxr['songzhuangu'])
                if xdxr['peigu'] is not None:
                    records[date][3] = int(10000 * xdxr['peigu'])
                if xdxr['peigujia'] is not None:
                    records[date][4] = int(1000 * xdxr['peigujia'])
                if xdxr['fenhong'] is not None:
                    records[date][5] = int(1000 * xdxr['fenhong'])
                if xdxr['houzongguben'] is not None:
                    records[date][7] = round(xdxr['houzongguben'])
                if xdxr['panhouliutong'] is not None:
                    records[date][8] = round(xdxr['panhouliutong'])
            last_date = date
            if xdxr['houzongguben'] is not None:
                last_total_count = round(xdxr['houzongguben'])
            if xdxr['panhouliutong'] is not None:
                last_free_count = round(xdxr['panhouliutong'])

            if records:
                cur = connect.cursor()
                cur.executemany(
                    "INSERT INTO StkWeight(stockid, date, countAsGift, \
                                 countForSell, priceForSell, bonus, countOfIncreasement, totalCount, freeCount) \
                                 VALUES (?,?,?,?,?,?,?,?,?)", [x for x in records.values()]
                )
                connect.commit()
                cur.close()
                total_count += len(records)

    return total_count


if __name__ == '__main__':
    import os
    import time
    import sqlite3
    from hikyuu.data.common_sqlite3 import create_database
    starttime = time.time()

    dest_dir = "c:\\stock"
    tdx_server = '119.147.212.81'
    tdx_port = 7709
    quotations = ['stock', 'fund']

    connect = sqlite3.connect(dest_dir + "\\hikyuu.db")
    create_database(connect)

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    print("导入数量: {}".format(pytdx_import_weight_to_sqlite(api, connect, 'SH')))

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))