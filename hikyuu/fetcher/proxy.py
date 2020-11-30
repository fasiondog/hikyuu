# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

import requests
import json
import datetime
from hikyuu.util import hku_logger, hku_catch, hku_check

zhimahttp_url = 'http://webapi.http.zhimacangku.com/getip?num=1&type=2&pro=330000&city=0&yys=0&port=1&time=1&ts=1&ys=1&cs=1&lb=1&sb=0&pb=45&mr=2&regions='

# 上次获取时间
g_pre_fetch_time = datetime.datetime.now()

# 过期时间
g_expire_time = None

# 当前代理ip
g_host_ip = None

# 当前代理端口号
g_host_port = None

# 最大重试次数
g_max_retry_count = 10


def request_proxy_from_zhima():
    """返回格式如下：
    {'ip': '115.226.137.200',
    'port': 4223,
    'expire_time': '2020-12-01 00:12:33',
    'city': '浙江省丽水市',
    'isp': '电信'}
    """
    ret = requests.get(zhimahttp_url).text
    x = json.loads(ret)
    hku_check(x['success'], "fetch failed!")
    return x['data'][0]


def get_proxy_from_zhima():
    global g_max_retry_count
    for i in range(g_max_retry_count):
        try:
            data = request_proxy_from_zhima()
            return "{}:{}".format(data['ip'], data['port'])
        except:
            pass


def get_proxy():
    """ 
    返回代理地址，格式 :代理ip地址:代理端口号，如：183.164.239.57:4264
    如无法获取时，返回 None
    """
    #return None
    #ProxyError
    return "123.156.181.103:4281"