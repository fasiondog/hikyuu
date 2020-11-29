# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

import requests


def parse_one_result(resultstr):
    result = {}
    if not resultstr:
        return result

    if len(resultstr) <= 3 and resultstr[:3] != 'var':
        return result

    a = resultstr.split(',')
    if len(a) < 9:
        return

    try:
        tmp = a[0].split('"')
        result['name'] = tmp[1]

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
