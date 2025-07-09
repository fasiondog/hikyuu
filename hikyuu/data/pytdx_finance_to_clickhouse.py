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

from hikyuu.data.common import MARKET, historyfinancialreader, get_a_stktype_list
from hikyuu.data.common_pytdx import to_pytdx_market
from hikyuu.util import *


@hku_catch(trace=True)
def pytdx_import_finance_to_clickhouse(db_connect, pytdx_connect, market):
    """导入公司财务信息"""
    sql = "select code from hku_base.stock where market='{}' and type in {} and valid=1"\
        .format(market, get_a_stktype_list())
    a = db_connect.query(sql)
    all_list = a.result_rows
    # print(all_list)

    records = []
    for stk in all_list:
        x = pytdx_connect.get_finance_info(to_pytdx_market(market), stk[0])
        if x is not None and x['code'] == stk[0]:
            sql = f"select updated_date from hku_base.stkfinance where market='{market}' and code='{stk[0]}' and updated_date={x['updated_date']}"
            a = db_connect.query(sql)
            a = a.result_rows
            if a:
                continue
            records.append(
                (
                    market, stk[0], x['updated_date'], x['ipo_date'], x['province'], x['industry'], x['zongguben'],
                    x['liutongguben'], x['guojiagu'], x['faqirenfarengu'], x['farengu'], x['bgu'], x['hgu'],
                    x['zhigonggu'], x['zongzichan'], x['liudongzichan'], x['gudingzichan'], x['wuxingzichan'],
                    x['gudongrenshu'], x['liudongfuzhai'], x['changqifuzhai'], x['zibengongjijin'], x['jingzichan'],
                    x['zhuyingshouru'], x['zhuyinglirun'], x['yingshouzhangkuan'], x['yingyelirun'], x['touzishouyu'],
                    x['jingyingxianjinliu'], x['zongxianjinliu'], x['cunhuo'], x['lirunzonghe'], x['shuihoulirun'],
                    x['jinglirun'], x['weifenpeilirun'], x['meigujingzichan'], x['baoliu2']
                )
            )

    if records:
        ic = db_connect.create_insert_context(table='stkfinance', database='hku_base',
                                              data=records)
        db_connect.insert(context=ic)
    return len(records)


def history_finance_import_clickhouse(connect, filename):
    file_date = filename[-12:-4]
    ret = historyfinancialreader(filename)
    buf = []
    for v in ret:
        buf.append((v[1][:2], v[1][2:], v[2], v[3], file_date))
    if len(ret) > 0:
        connect.command(f"delete from hku_base.historyfinance where file_date={file_date}")
        ic = connect.create_insert_context(table='historyfinance', database='hku_base',
                                           column_names=['market', 'code', 'report_date', 'values', 'file_date'],
                                           data=buf)
        connect.insert(context=ic)


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
    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    # pytdx_import_finance_to_clickhouse(client, api, "BJ")
    history_finance_import_clickhouse(client, "/Users/fasiondog/stock/downloads/finance/gpcw20121231.dat")

    client.close()
