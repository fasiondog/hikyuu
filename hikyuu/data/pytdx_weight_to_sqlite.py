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
from hikyuu.data.common_pytdx import to_pytdx_market


def pytdx_import_weight_to_sqlite(pytdx_api, connect, market):
    """导入钱龙格式的权息数据"""
    cur = connect.cursor()
    marketid = cur.execute("select marketid from Market where market='%s'" % market)
    marketid = [id[0] for id in marketid]
    marketid = marketid[0]
    pytdx_market = to_pytdx_market(market)

    total_count = 0
    stockid_list = cur.execute("select stockid, code from Stock where marketid=%s and valid=1" % (marketid))
    stockid_list = [x for x in stockid_list]
    cur.close()

    for stockrecord in stockid_list:
        stockid, code = stockrecord
        # print("{}{}".format(market, code))

        # 获取当前数据库中最后的一条权息记录的总股本和流通股本
        cur = connect.cursor()
        a = cur.execute(
            "select id, stockid, date, countAsGift, countForSell, priceForSell, \
                    bonus, countOfIncreasement, totalCount, \
                    freeCount, suogu from stkweight where stockid=%s \
                    order by date desc limit 1" % stockid
        )
        a = [x for x in a]
        last_db_weight = None
        if a:
            a = list(a[0])
            last_db_weight = a
            db_last_date = a[2]
            last_total_count, last_free_count = a[8:10]
        else:
            last_db_weight = None
            db_last_date, last_total_count, last_free_count = (0, 0, 0)
        cur.close()

        xdxr_list = pytdx_api.get_xdxr_info(pytdx_market, code)
        update_last_db_weight = False
        new_last_db_weight = last_db_weight
        records = {}
        for xdxr in xdxr_list:
            try:
                date = xdxr['year'] * 10000 + xdxr['month'] * 100 + xdxr['day']
                if date < db_last_date:
                    continue
                if date == db_last_date and new_last_db_weight is not None:
                    if xdxr['songzhuangu'] is not None:
                        new_last_db_weight[3] = int(10000 * xdxr['songzhuangu'])
                        update_last_db_weight = True
                    if xdxr['peigu'] is not None:
                        new_last_db_weight[4] = int(10000 * xdxr['peigu'])
                        update_last_db_weight = True
                    if xdxr['peigujia'] is not None:
                        new_last_db_weight[5] = int(1000 * xdxr['peigujia'])
                        update_last_db_weight = True
                    if xdxr['fenhong'] is not None:
                        new_last_db_weight[6] = int(1000 * xdxr['fenhong'])
                        update_last_db_weight = True
                    if xdxr['houzongguben'] is not None:
                        new_last_db_weight[8] = round(xdxr['houzongguben'])
                        update_last_db_weight = True
                        last_total_count = new_last_db_weight[8]
                    if xdxr['panhouliutong'] is not None:
                        new_last_db_weight[9] = round(xdxr['panhouliutong'])
                        update_last_db_weight = True
                        last_free_count = new_last_db_weight[9]
                    if xdxr['suogu'] is not None:
                        # etf 扩股
                        new_last_db_weight[10] = xdxr['suogu']
                        update_last_db_weight = True
                    continue
                if date not in records:
                    records[date] = [
                        stockid,
                        date,
                        int(10000 * xdxr['songzhuangu']) if xdxr['songzhuangu'] is not None else 0,  # countForGift
                        int(10000 * xdxr['peigu']) if xdxr['peigu'] is not None else 0,  # countForSell
                        int(1000 * xdxr['peigujia']) if xdxr['peigujia'] is not None else 0,  # priceForSell
                        int(1000 * xdxr['fenhong']) if xdxr['fenhong'] is not None else 0,  # bonus
                        0,  # countOfIncreasement, pytdx 不区分送股和转增股，统一记在送股
                        round(xdxr['houzongguben'])
                        if xdxr['houzongguben'] is not None else last_total_count,  # totalCount
                        round(xdxr['panhouliutong'])
                        if xdxr['panhouliutong'] is not None else last_free_count,  # freeCount
                        xdxr["suogu"] if xdxr["suogu"] is not None else 0
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
                    if xdxr['suogu'] is not None:
                        records[date][9] = xdxr['suogu']
                if xdxr['houzongguben'] is not None:
                    last_total_count = round(xdxr['houzongguben'])
                if xdxr['panhouliutong'] is not None:
                    last_free_count = round(xdxr['panhouliutong'])
            except Exception as e:
                print(e)
                print("{} {}{} xdxr: {} last_db_weigth:{}".format(stockid, market, code, xdxr, new_last_db_weight))
                raise e

        if update_last_db_weight:
            cur = connect.cursor()
            x = new_last_db_weight
            cur.execute(
                "UPDATE StkWeight SET countAsGift=%s, countForSell=%s, priceForSell=%s, \
                    bonus=%s, totalCount=%s, freeCount=%s, suogu=%s \
                    where id=%s" % (x[3], x[4], x[5], x[6], x[8], x[9], x[10], x[0])
            )
            connect.commit()
            cur.close()

        if records:
            cur = connect.cursor()
            cur.executemany(
                "INSERT INTO StkWeight(stockid, date, countAsGift, \
                             countForSell, priceForSell, bonus, countOfIncreasement, totalCount, freeCount, suogu) \
                             VALUES (?,?,?,?,?,?,?,?,?,?)", [x for x in records.values()]
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

    dest_dir = "d:\\stock"
    tdx_server = '119.147.212.81'
    tdx_port = 7709
    quotations = ['stock', 'fund']

    connect = sqlite3.connect(dest_dir + "\\stock.db")
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
