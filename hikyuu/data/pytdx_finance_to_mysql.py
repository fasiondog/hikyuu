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

from hikyuu.data.common import MARKETID, STOCKTYPE
from hikyuu.data.common_mysql import get_marketid
from hikyuu.util import *

@hku_catch(trace=True)
def pytdx_import_finance_to_mysql(db_connect, pytdx_connect, market):
    """导入公司财务信息"""
    marketid = get_marketid(db_connect, market)
    sql = "select `stockid`, `marketid`, `code`, `valid`, `type` from `hku_base`.`stock` where marketid={} and type = {} and valid=1"\
        .format(marketid, STOCKTYPE.A)

    cur = db_connect.cursor()
    a = cur.execute(sql)
    all_list = cur.fetchall()
    db_connect.commit()

    records = []
    for stk in all_list:
        x = pytdx_connect.get_finance_info(1 if stk[1] == MARKETID.SH else 0, stk[2])
        #print(stk[2])
        if x is not None and x['code'] == stk[2]:
            cur.execute(
                "select updated_date from `hku_base`.`stkfinance` where stockid={} and updated_date={}".format(
                    stk[0], x['updated_date']
                )
            )
            a = cur.fetchall()
            a = [x[0] for x in a]
            if a:
                #print(a)
                continue
            #else:
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
            "INSERT INTO `hku_base`.`stkfinance`(stockid, \
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
                        VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s, \
                                %s,%s,%s,%s,%s,%s,%s,%s,%s,%s, \
                                %s,%s,%s,%s,%s,%s,%s,%s,%s,%s, \
                                %s,%s,%s,%s,%s,%s)", records
        )
        db_connect.commit()

    cur.close()
    return len(records)