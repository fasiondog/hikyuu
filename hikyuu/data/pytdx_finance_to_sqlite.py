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

from hikyuu.data.common import MARKETID, STOCKTYPE, historyfinancialreader, get_a_stktype_list
from hikyuu.data.common_sqlite3 import get_marketid, create_database


def pytdx_import_finance_to_sqlite(db_connect, pytdx_connect, market):
    """导入公司财务信息"""
    marketid = get_marketid(db_connect, market)
    sql = "select stockid, marketid, code, valid, type from stock where marketid={} and type in {} and valid=1"\
        .format(marketid, get_a_stktype_list())

    cur = db_connect.cursor()
    all_list = cur.execute(sql).fetchall()
    db_connect.commit()

    records = []
    for stk in all_list:
        x = pytdx_connect.get_finance_info(1 if stk[1] == MARKETID.SH else 0, stk[2])
        # print(stk[2])
        if x is not None and x['code'] == stk[2]:
            cur.execute(
                "select updated_date from stkfinance where stockid={} and updated_date={}".format(
                    stk[0], x['updated_date']
                )
            )
            a = cur.fetchall()
            a = [x[0] for x in a]
            if a:
                # print(a)
                continue
            # else:
            #    print(market, stk[2])
            records.append(
                (
                    stk[0], x['updated_date'], x['ipo_date'], x['province'], x['industry'], x['zongguben'],
                    x['liutongguben'], x['guojiagu'], x['faqirenfarengu'], x['farengu'], x['bgu'], x['hgu'],
                    x['zhigonggu'], x['zongzichan'], x['liudongzichan'], x['gudingzichan'], x['wuxingzichan'],
                    x['gudongrenshu'], x['liudongfuzhai'], x['changqifuzhai'], x['zibengongjijin'], x['jingzichan'],
                    x['zhuyingshouru'], x['zhuyinglirun'], x['yingshouzhangkuan'], x['yingyelirun'], x['touzishouyu'],
                    x['jingyingxianjinliu'], x['zongxianjinliu'], x['cunhuo'], x['lirunzonghe'], x['shuihoulirun'],
                    x['jinglirun'], x['weifenpeilirun'], x['meigujingzichan'], x['baoliu2']
                )
            )
            pass

    if records:
        cur.executemany(
            "INSERT INTO stkfinance(stockid, \
                                                updated_date, \
                                                ipo_date, \
                                                province, \
                                                industry, \
                                                zongguben, \
                                                liutongguben, \
                                                guojiagu, \
                                                faqirenfarengu, \
                                                farengu, \
                                                bgu, \
                                                hgu, \
                                                zhigonggu, \
                                                zongzichan, \
                                                liudongzichan, \
                                                gudingzichan, \
                                                wuxingzichan, \
                                                gudongrenshu, \
                                                liudongfuzhai, \
                                                changqifuzhai, \
                                                zibengongjijin, \
                                                jingzichan, \
                                                zhuyingshouru, \
                                                zhuyinglirun, \
                                                yingshouzhangkuan, \
                                                yingyelirun, \
                                                touzishouyu, \
                                                jingyingxianjinliu, \
                                                zongxianjinliu, \
                                                cunhuo, \
                                                lirunzonghe, \
                                                shuihoulirun, \
                                                jinglirun, \
                                                weifenpeilirun, \
                                                meigujingzichan, \
                                                baoliu2) \
                        VALUES (?,?,?,?,?,?,?,?,?,?, \
                                ?,?,?,?,?,?,?,?,?,?, \
                                ?,?,?,?,?,?,?,?,?,?, \
                                ?,?,?,?,?,?)", records
        )
        db_connect.commit()

    cur.close()
    return len(records)


def history_finance_import_sqlite(connect, filename):
    file_date = filename[-12:-4]
    ret = historyfinancialreader(filename)
    cur = connect.cursor()
    cur.execute(f"delete from `HistoryFinance` where file_date={file_date}")
    cur.executemany(
        "insert into `HistoryFinance` (`file_date`, `market_code`, `report_date`, `values`) values (?,?,?,?)", ret)
    connect.commit()
    cur.close()


if __name__ == '__main__':
    import os
    import time
    import sqlite3

    starttime = time.time()

    dest_dir = "d:\\stock"
    tdx_server = '120.76.152.87'  # '119.147.212.81'
    tdx_port = 7709

    connect = sqlite3.connect(dest_dir + "\\stock.db")
    create_database(connect)

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    x = pytdx_import_finance_to_sqlite(connect, api, "SZ")
    print(x)

    x = pytdx_import_finance_to_sqlite(connect, api, "SH")
    print(x)

    x = pytdx_import_finance_to_sqlite(connect, api, "BJ")
    print(x)

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
