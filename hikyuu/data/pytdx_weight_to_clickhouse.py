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
from hikyuu.util import *
from hikyuu.data.common_pytdx import to_pytdx_market


@hku_catch(trace=True)
def pytdx_import_weight_to_clickhouse(pytdx_api, connect, market):
    """从 pytdx 导入权息数据"""
    pytdx_market = to_pytdx_market(market)

    total_count = 0
    update_count = 0
    stock_list = connect.query(f"select code from `hku_base`.`stock` where market='{market}' and valid=1")
    stockid_list = stock_list.result_rows

    for stockrecord in stockid_list:
        code = stockrecord[0]

        # 获取当前数据库中最后的一条权息记录的总股本和流通股本
        sql = f"select date, countAsGift, countForSell, priceForSell, bonus, countOfIncreasement, totalCount, freeCount, suogu from `hku_base`.`stkweight` where market='{market}' and code='{code}' order by date desc limit 1"
        tmp = connect.query(sql)
        a = tmp.result_rows
        last_db_weight = None
        if a:
            a = list(a[0])
            last_db_weight = a
            db_last_date = a[0]
            last_total_count, last_free_count = a[6:8]
        else:
            last_db_weight = None
            db_last_date, last_total_count, last_free_count = (0, 0, 0)

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
                        new_last_db_weight[1] = 10000 * xdxr['songzhuangu']
                        update_last_db_weight = True
                    if xdxr['peigu'] is not None:
                        new_last_db_weight[2] = 10000 * xdxr['peigu']
                        update_last_db_weight = True
                    if xdxr['peigujia'] is not None:
                        new_last_db_weight[3] = 1000 * xdxr['peigujia']
                        update_last_db_weight = True
                    if xdxr['fenhong'] is not None:
                        new_last_db_weight[4] = 1000 * xdxr['fenhong']
                        update_last_db_weight = True
                    if xdxr['houzongguben'] is not None:
                        new_last_db_weight[6] = xdxr['houzongguben']
                        update_last_db_weight = True
                        last_total_count = new_last_db_weight[6]
                    if xdxr['panhouliutong'] is not None:
                        new_last_db_weight[7] = xdxr['panhouliutong']
                        update_last_db_weight = True
                        last_free_count = new_last_db_weight[7]
                    if xdxr['suogu'] is not None:
                        # etf 扩缩股
                        new_last_db_weight[8] = xdxr['suogu']
                        update_last_db_weight = True
                    continue
                if date not in records:
                    records[date] = [
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
                        records[date][1] = 10000 * xdxr['songzhuangu']
                    if xdxr['peigu'] is not None:
                        records[date][2] = 10000 * xdxr['peigu']
                    if xdxr['peigujia'] is not None:
                        records[date][3] = 1000 * xdxr['peigujia']
                    if xdxr['fenhong'] is not None:
                        records[date][4] = 1000 * xdxr['fenhong']
                    if xdxr['houzongguben'] is not None:
                        records[date][6] = xdxr['houzongguben']
                    if xdxr['panhouliutong'] is not None:
                        records[date][7] = xdxr['panhouliutong']
                    if xdxr['suogu'] is not None:
                        records[date][8] = xdxr['suogu']
                if xdxr['houzongguben'] is not None:
                    last_total_count = xdxr['houzongguben']
                if xdxr['panhouliutong'] is not None:
                    last_free_count = xdxr['panhouliutong']
            except Exception as e:
                print(e)
                print("{}{} xdxr: {} last_db_weigth:{}".format(market, code, xdxr, new_last_db_weight))
                raise e

        if update_last_db_weight and new_last_db_weight != last_db_weight:
            v = new_last_db_weight
            sql = f"delete from hku_base.stkweight where market='{market}' and code='{code}' and date={v[0]}"
            connect.command(sql)
            records[v[0]] = v
            update_count += 1

        if records:
            insert_records = []
            for v in records.values():
                tmp = [market, code]
                tmp.extend(v)
                insert_records.append(tmp)
            ic = connect.create_insert_context(table='stkweight', database='hku_base',
                                               column_names=['market', 'code', 'date', 'countAsGift', 'countForSell',
                                                             'priceForSell', 'bonus', 'countOfIncreasement', 'totalCount', 'freeCount', 'suogu'],
                                               data=insert_records)
            connect.insert(context=ic)
            total_count += len(records)

    total_count = total_count - update_count
    return total_count if total_count > 0 else 0


if __name__ == '__main__':
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

    # sql = "select date, countAsGift, countForSell, priceForSell, bonus, countOfIncreasement, totalCount, freeCount, suogu from `hku_base`.`stkweight` where market='SH' and code='000001' order by date desc limit 1"
    # result = client.query(sql)
    # print(result.result_rows)
    # client.close()

    tdx_server = '180.101.48.170'
    tdx_port = 7709
    quotations = ['stock', 'fund']

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    import time
    starttime = time.time()
    print("开始导入")
    print("导入数量: {}".format(pytdx_import_weight_to_clickhouse(api, client, 'SH')))

    api.disconnect()
    client.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
