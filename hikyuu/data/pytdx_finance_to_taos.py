#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-06-14
#    Author: fasiondog

from hikyuu import Datetime, UTCOffset
from hikyuu.data.common import MARKETID, STOCKTYPE, historyfinancialreader, get_a_stktype_list
from hikyuu.data.common_taos import get_marketid, get_taos, reload_taos
from hikyuu.data.common_pytdx import to_pytdx_market
from hikyuu.util import hku_info, hku_error


def pytdx_import_finance_to_taos(db_connect, pytdx_connect, market):
    """导入公司财务信息"""
    sql = "select cast(stockid as bigint), market, code, valid, type from hku_base.n_stock where market='{}' and type in {} and valid=1"\
        .format(market, get_a_stktype_list())

    cur = db_connect.cursor()
    cur.execute(sql)
    all_list = [v for v in cur]

    records = []
    count = 0
    for stk in all_list:
        records.clear()
        code = stk[2]
        x = pytdx_connect.get_finance_info(to_pytdx_market(market), code)
        # print(f'{market}{stk[2]}', len(x) if x is not None else 0)
        if x is not None and x['code'] == stk[2]:
            if x['updated_date'] == 0:
                continue
            update_date = (Datetime(x['updated_date'])-UTCOffset()).timestamp()
            cur.execute(
                f"select updated_date from hku_base.s_stkfinance where market='{market}' and code='{code}' and updated_date={update_date}"
            )
            a = [v for v in cur]
            if a:
                continue

            records.append(
                (
                    stk[0], update_date, x['ipo_date'], x['province'], x['industry'], x['zongguben'],
                    x['liutongguben'], x['guojiagu'], x['faqirenfarengu'], x['farengu'], x['bgu'], x['hgu'],
                    x['zhigonggu'], x['zongzichan'], x['liudongzichan'], x['gudingzichan'], x['wuxingzichan'],
                    x['gudongrenshu'], x['liudongfuzhai'], x['changqifuzhai'], x['zibengongjijin'], x['jingzichan'],
                    x['zhuyingshouru'], x['zhuyinglirun'], x['yingshouzhangkuan'], x['yingyelirun'], x['touzishouyu'],
                    x['jingyingxianjinliu'], x['zongxianjinliu'], x['cunhuo'], x['lirunzonghe'], x['shuihoulirun'],
                    x['jinglirun'], x['weifenpeilirun'], x['meigujingzichan'], x['baoliu2']
                )
            )

        if records:
            tbname = f"z_fin_{market}{code}".lower()
            # print(tbname)
            rawsql = f"INSERT INTO hku_base.{tbname} using hku_base.s_stkfinance TAGS ('{market}', '{code}') VALUES "
            sql = rawsql
            for i, r in enumerate(records):
                sql += f"({r[1]},{r[2]},{r[3]},{r[4]},{r[5]},{r[6]},{r[7]},{r[8]},{r[9]},{r[10]},{r[11]},{r[12]},{r[13]},{r[14]},{r[15]},{r[16]},{r[17]},{r[18]},{r[19]},{r[20]},{r[21]},{r[22]},{r[23]}, {r[24]}, {r[25]}, {r[26]}, {r[27]}, {r[28]}, {r[29]}, {r[30]}, {r[31]}, {r[32]}, {r[33]}, {r[34]}, {r[35]})"
                if i > 0 and i % 3000 == 0:
                    cur.execute(sql)
                    sql = rawsql
            if sql != rawsql:
                cur.execute(sql)
            count += len(records)

    cur.close()
    return count


def history_finance_import_taos(connect, filename):
    ret = historyfinancialreader(filename)
    hku_info("total: {}", len(ret))
    taos = get_taos()
    if taos is None:
        hku_error("Can not import taos.")
        return 0
    for i, r in enumerate(ret):
        # if filename == '/Users/fasiondog/stock/downloads/finance/gpcw20241231.dat':
        #     hku_info(f'{i}: marketcode: {r[1]}, {r[2]}, {len(r[3])}')
        marketcode = r[1].lower()
        stmt = connect.statement("INSERT INTO ? using hku_base.s_historyfinance tags(?) VALUES (?,?,?)")
        tags = taos.new_bind_params(1)
        tags[0].binary(marketcode.upper())
        stmt.set_tbname_tags(f'hku_base.z_his_fin_{marketcode}', tags)
        params = taos.new_bind_params(3)
        params[0].timestamp((Datetime(r[0]) - UTCOffset()).timestamp(), taos.PrecisionEnum.Microseconds)
        params[1].int(r[2])
        # params[2].varbinary(bytearray(r[3]))
        params[2].varbinary(r[3])
        stmt.bind_param(params)
        stmt.execute()
        stmt.close()
    connect.commit()
    return len(ret)


def test_func(param):
    filename, user, password, host, port = param
    taos = get_taos()
    connect = taos.connect(
        user=user, password=password, host=host, port=port)
    hku_info('正在导入文件：{}', filename)
    ret = history_finance_import_taos(connect, '/Users/fasiondog/stock/downloads/finance/' + filename)
    connect.close()
    return ret


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

    taos = get_taos()
    connect = taos.connect(
        user=user, password=password, host=host, port=port)

    import time
    starttime = time.time()

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    # file_list = os.listdir('/Users/fasiondog/stock/downloads/finance')
    # params = [(filename, user, password, host, port) for filename in file_list if filename.endswith('.dat')]

    # from concurrent import futures
    # with futures.ProcessPoolExecutor(4) as executor:
    #     res = executor.map(test_func, params, timeout=60)
    # x = [i for i in res]
    # print(sum(x))

    # cnt = 1
    # file_list = os.listdir('/Users/fasiondog/stock/downloads/finance')
    # for filename in file_list:
    #     if filename.endswith('.dat'):
    #         if cnt == 137:
    #             reload_taos()
    #         hku_info('[{}] 正在导入文件：{}', cnt, filename)
    #         history_finance_import_taos(connect, '/Users/fasiondog/stock/downloads/finance/' + filename)
    #         cnt += 1

    # history_finance_import_taos(connect, '/Users/fasiondog/stock/downloads/finance/gpcw20021231.dat')
    # history_finance_import_taos(connect, '/Users/fasiondog/stock/downloads/finance/gpcw20030930.dat')
    print("导入数量: {}".format(pytdx_import_finance_to_taos(connect, api, 'SH')))

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
