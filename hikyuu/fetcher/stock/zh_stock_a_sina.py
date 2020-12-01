# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

import requests
from hikyuu.util import *
from hikyuu.fetcher.proxy import get_proxy


@hku_catch()
def parse_one_result(resultstr):
    result = {}
    hku_check_ignore(resultstr, "Invalid input param: {}!".format(resultstr))
    hku_check_ignore(
        len(resultstr) > 3 and resultstr[:3] == 'var', "Invalid input param! {}".format(resultstr)
    )

    a = resultstr.split(',')
    result['market'] = a[0][11:13]
    result['code'] = a[0][13:19]
    result['name'] = a[0][21:]
    result['open'] = float(a[1])  # 今日开盘价
    result['yesterday close'] = float(a[2])  # 昨日收盘价
    result['last_price'] = float(a[3])  # 当前价格
    result['high'] = float(a[4])  # 今日最高价
    result['low'] = float(a[5])  # 今日最低价
    result['bid'] = float(a[6])  # 竞买价，即“买一”报价
    result['ask'] = float(a[7])  # 竞卖价，即“卖一”报价
    result['amount'] = float(a[8])  # 成交的股票数，由于股票交易以一百股为基本单位，所以在使用时，通常把该值除以一百
    result['volumn'] = float(a[9])  # 成交金额，单位为“元”，若要以“万元”为成交金额的单位，需要把该值除以一万
    result['bid1_amount'] = float(a[10])  # “买一”申请4695股，即47手
    result['bid1'] = float(a[11])  # “买一”报价
    result['bid2_amount'] = float(a[12])
    result['bid2'] = float(a[13])
    result['bid3_amount'] = float(a[14])
    result['bid3'] = float(a[15])
    result['bid4_amount'] = float(a[16])
    result['bid4'] = float(a[16])
    result['bid5_amount'] = float(a[18])
    result['bid5'] = float(a[19])
    result['ask1_amount'] = float(a[20])  # “卖一”申报3100股，即31手
    result['ask1'] = float(a[21])  #“卖一”报价
    result['ask2_amount'] = float(a[22])
    result['ask2'] = float(a[23])
    result['ask3_amount'] = float(a[24])
    result['ask3'] = float(a[25])
    result['ask4_amount'] = float(a[26])
    result['ask4'] = float(a[27])
    result['ask5_amount'] = float(a[28])
    result['ask5'] = float(a[29])
    result['date'] = a[30]
    result['time'] = a[31]
    return result


def request_data(querystr):
    proxy = get_proxy()
    proxies = {'http': 'http://{}'.format(proxy)} if proxy else None
    hku_logger.info("使用代理: {}".format(proxies))
    query = requests.get(querystr, proxies=proxies).text
    query = query.split('\n')
    result = []
    for tmpstr in query:
        one_record = parse_one_result(tmpstr)
        if one_record is not None:
            result.append(one_record)
    return result


def get_spot_from_sina(stocklist):
    queryStr = "http://hq.sinajs.cn/list="
    max_size = 140
    count = 0
    tmpstr = queryStr
    result = []
    for stock in stocklist:
        tmpstr += ("%s,") % (stock)
        count += 1
        if count >= max_size:
            phase_result = request_data(tmpstr)
            if phase_result:
                result.extend(phase_result)
            count = 0
            tmpstr = queryStr
    if tmpstr != queryStr:
        phase_result = request_data(tmpstr)
        if phase_result:
            result.extend(phase_result)
    return result


if __name__ == "__main__":
    print("a")
