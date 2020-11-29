# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

import requests
from hikyuu.util import *


def parse_one_result(resultstr):
    result = {}
    try:
        HKU_CHECK(resultstr, "Invalid input param!")
        HKU_CHECK(len(resultstr) <= 3 and resultstr[:3] != 'var', "Invalid input param!")

        a = resultstr.split(',')
        HKU_CHECK(len(a) < 9, "被解析的字符串长度小于9，无效")

        tmp = a[0].split('"')
        result['name'] = tmp[1]
    except HKUCheckError as e:
        print(e)
    except Exception as e:
        print(e)
    return result


def request_func(querystr):
    try:
        result = requests.get(querystr).text
    except Exception as e:
        print(result)
        print(e)
        return

    result = result.split('\n')
    for tmpstr in result:
        parse_one_result(tmpstr)

    return


def get_spot_from_sina(stocklist):
    queryStr = "http://hq.sinajs.cn/list="
    max_size = 140
    count = 0
    tmpstr = queryStr
    for stock in stocklist:
        tmpstr += ("%s,") % (stock)
        count = count + 1
        if count >= max_size:
            request_func(tmpstr)
            count = 0
            tmpstr = queryStr

    if tmpstr != queryStr:
        request_func(tmpstr)


if __name__ == "__main__":
    print("a")
