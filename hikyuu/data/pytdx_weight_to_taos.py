#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-06-14
#    Author: fasiondog

import taos
from hikyuu import Datetime, UTCOffset
from pytdx.hq import TDXParams
from hikyuu.data.common_pytdx import to_pytdx_market


def pytdx_import_weight_to_taos(pytdx_api, connect, market):
    """导入钱龙格式的权息数据"""
    cur = connect.cursor()
    cur.execute(f"select marketid from hku_base.n_market where market='{market}'")
    marketid = [id[0] for id in cur]
    marketid = marketid[0]
    pytdx_market = to_pytdx_market(market)

    total_count = 0
    cur.execute(
        f"select cast(stockid as bigint), code from hku_base.n_stock where marketid={marketid} and valid=1")
    stockid_list = [x for x in cur]

    for stockrecord in stockid_list:
        stockid, code = stockrecord
        # print("{}{}".format(market, code))
        tbname = f"z_wgt_{market}{code}".lower()

        # 获取当前数据库中最后的一条权息记录的总股本和流通股本
        cur.execute(
            f"select cast(date as bigint), countAsGift, countForSell, priceForSell, \
                    bonus, countOfIncreasement, totalCount, \
                    freeCount, suogu from hku_base.s_stkweight where stockid={stockid} \
                    order by date desc limit 1"
        )
        a = [x for x in cur]
        last_db_weight = None
        if a:
            a = list(a[0])
            db_last_date = Datetime.from_timestamp(a[0]).ymd
            last_db_weight = [0, stockid].append(a)
            last_total_count, last_free_count = a[6:8]
        else:
            last_db_weight = None
            db_last_date, last_total_count, last_free_count = (0, 0, 0)

        xdxr_list = pytdx_api.get_xdxr_info(pytdx_market, code)
        if xdxr_list is None:
            continue

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
            x = new_last_db_weight
            # print(tbname, x)
            sql = f"INSERT INTO hku_base.{tbname} using hku_base.s_stkweight TAGS ({stockid}) VALUES ({x[0]}, {x[3]}, {x[4]}, {x[5]}, {x[6]}, {x[8]}, {x[9]}, {x[10]})"
            cur.execute(sql)

        if records:
            rawsql = f"INSERT INTO hku_base.{tbname} using hku_base.s_stkweight TAGS ({stockid}) VALUES "
            sql = rawsql
            for i, r in enumerate(records.values()):
                sql += f"({(Datetime(r[1])-UTCOffset()).timestamp()}, {r[2]}, {r[3]}, {r[4]}, {r[5]}, {r[6]}, {r[7]}, {r[8]}, {r[9]})"
                if i > 0 and i % 5000 == 0:
                    cur.execute(sql)
                    sql = rawsql
            if sql != rawsql:
                cur.execute(sql)
            total_count += len(records)

    cur.close()
    return total_count


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

    connect = taos.connect(
        user=user, password=password, host=host, port=port)

    import time
    starttime = time.time()

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    print("导入数量: {}".format(pytdx_import_weight_to_taos(api, connect, 'SH')))

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
