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
import mysql.connector
from hikyuu.util import *
from .common_pytdx import to_pytdx_market


@hku_catch(trace=True)
def pytdx_import_weight_to_mysql(pytdx_api, connect, market):
    """从 pytdx 导入权息数据"""
    cur = connect.cursor()
    cur.execute("select marketid from `hku_base`.`market` where market='%s'" % market)
    marketid = [id[0] for id in cur.fetchall()]
    marketid = marketid[0]
    pytdx_market = to_pytdx_market(market)

    total_count = 0
    cur.execute("select stockid, code from `hku_base`.`stock` where marketid=%s and valid=1" % (marketid))
    stockid_list = [x for x in cur.fetchall()]
    cur.close()

    for stockrecord in stockid_list:
        stockid, code = stockrecord
        # print("{}{}".format(market, code))

        # 获取当前数据库中最后的一条权息记录的总股本和流通股本
        cur = connect.cursor()
        cur.execute(
            "select id, stockid, date, countAsGift, countForSell, priceForSell, \
                    bonus, countOfIncreasement, totalCount, \
                    freeCount, suogu from `hku_base`.`stkweight` where stockid=%s \
                    order by date desc limit 1" % stockid
        )
        a = [x for x in cur.fetchall()]
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
                        new_last_db_weight[3] = 10000 * xdxr['songzhuangu']
                        update_last_db_weight = True
                    if xdxr['peigu'] is not None:
                        new_last_db_weight[4] = 10000 * xdxr['peigu']
                        update_last_db_weight = True
                    if xdxr['peigujia'] is not None:
                        new_last_db_weight[5] = 1000 * xdxr['peigujia']
                        update_last_db_weight = True
                    if xdxr['fenhong'] is not None:
                        new_last_db_weight[6] = 1000 * xdxr['fenhong']
                        update_last_db_weight = True
                    if xdxr['houzongguben'] is not None:
                        new_last_db_weight[8] = xdxr['houzongguben']
                        update_last_db_weight = True
                        last_total_count = new_last_db_weight[8]
                    if xdxr['panhouliutong'] is not None:
                        new_last_db_weight[9] = xdxr['panhouliutong']
                        update_last_db_weight = True
                        last_free_count = new_last_db_weight[9]
                    if xdxr['suogu'] is not None:
                        # etf 扩缩股
                        new_last_db_weight[10] = xdxr['suogu']
                        update_last_db_weight = True
                    continue
                if date not in records:
                    records[date] = [
                        stockid,
                        date,
                        10000 * xdxr['songzhuangu'] if xdxr['songzhuangu'] is not None else 0,  # countAsGift
                        10000 * xdxr['peigu'] if xdxr['peigu'] is not None else 0,  # countForSell
                        1000 * xdxr['peigujia'] if xdxr['peigujia'] is not None else 0,  # priceForSell
                        1000 * xdxr['fenhong'] if xdxr['fenhong'] is not None else 0,  # bonus
                        0,  # countOfIncreasement, pytdx 不区分送股和转增股，统一记在送股
                        xdxr['houzongguben'] if xdxr['houzongguben'] is not None else last_total_count,  # totalCount
                        xdxr['panhouliutong'] if xdxr['panhouliutong'] is not None else last_free_count,  # freeCount
                        xdxr["suogu"] if xdxr["suogu"] is not None else 0
                    ]
                else:
                    if xdxr['songzhuangu'] is not None:
                        records[date][2] = 10000 * xdxr['songzhuangu']
                    if xdxr['peigu'] is not None:
                        records[date][3] = 10000 * xdxr['peigu']
                    if xdxr['peigujia'] is not None:
                        records[date][4] = 1000 * xdxr['peigujia']
                    if xdxr['fenhong'] is not None:
                        records[date][5] = 1000 * xdxr['fenhong']
                    if xdxr['houzongguben'] is not None:
                        records[date][7] = xdxr['houzongguben']
                    if xdxr['panhouliutong'] is not None:
                        records[date][8] = xdxr['panhouliutong']
                    if xdxr['suogu'] is not None:
                        records[date][9] = xdxr['suogu']
                if xdxr['houzongguben'] is not None:
                    last_total_count = xdxr['houzongguben']
                if xdxr['panhouliutong'] is not None:
                    last_free_count = xdxr['panhouliutong']
            except Exception as e:
                print(e)
                print("{} {}{} xdxr: {} last_db_weigth:{}".format(stockid, market, code, xdxr, new_last_db_weight))
                raise e

        if update_last_db_weight:
            cur = connect.cursor()
            x = new_last_db_weight
            cur.execute(
                "UPDATE `hku_base`.`stkweight` SET countAsGift=%s, countForSell=%s, priceForSell=%s, \
                    bonus=%s, totalCount=%s, freeCount=%s, suogu=%s \
                    where id=%s" % (x[3], x[4], x[5], x[6], x[8], x[9], x[10], x[0])
            )
            connect.commit()
            cur.close()

        if records:
            cur = connect.cursor()
            cur.executemany(
                "INSERT INTO `hku_base`.`stkweight` (stockid, date, countAsGift, \
                countForSell, priceForSell, bonus, countOfIncreasement, totalCount, freeCount, suogu) \
                VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)", [x for x in records.values()]
            )
            connect.commit()
            cur.close()
            total_count += len(records)

    return total_count


if __name__ == '__main__':
    import os
    import time
    import sqlite3
    from hikyuu.data.common_mysql import create_database
    starttime = time.time()

    host = '127.0.0.1'
    port = 3306
    usr = 'root'
    pwd = ''
    tdx_server = '119.147.212.81'
    tdx_port = 7709
    quotations = ['stock', 'fund']

    connect = mysql.connector.connect(user=usr, password=pwd, host=host, port=port)
    create_database(connect)

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    print("导入数量: {}".format(pytdx_import_weight_to_mysql(api, connect, 'SH')))

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
