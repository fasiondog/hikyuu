# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

import requests
import json
import datetime
import threading
from hikyuu.util import hku_logger, hku_catch, hku_check

# The Zhima http proxy API url used to obtain a proxy; the local ip must be inside the Zhima whitelist
zhimahttp_url = 'http://webapi.http.zhimacangku.com/getip?num=1&type=2&pro=0&city=0&yys=0&port=1&time=1&ts=1&ys=1&cs=1&lb=1&sb=0&pb=45&mr=2&regions=110000,130000,140000,310000,320000,330000,370000,410000'


@hku_catch(retry=10)
def request_proxy_from_zhima():
    """Apply for a proxy IP from Zhima

    The returned format is as follows:
    {'ip': '115.226.137.200',
    'port': 4223,
    'expire_time': '2020-12-01 00:12:33' (actually a datetime)
    'city': '浙江省丽水市',  # returned as Chinese text
    'isp': '电信',  # returned as Chinese text
    'request_time': a datetime, the application time}
    """
    ret = requests.get(zhimahttp_url).text
    x = json.loads(ret)
    hku_check(x['success'], "Failed to get a proxy!")
    x = x['data'][0]
    x['expire_time'] = datetime.datetime.fromisoformat(x['expire_time'])
    x['request_time'] = datetime.datetime.now()
    return x


# Lock the application when multithreading
g_zhima_lock = threading.Lock()

# The last used proxy
g_zhima_last_proxy = None


def get_proxy(new=False):
    """Get a proxy from Zhima

    :param boolean new: True to force a new application | False to reuse the last obtained proxy ip as much as possible
    :return: None | 'host ip:port', e.g. '183.164.239.57:4264'
    """
    global g_zhima_last_proxy
    g_zhima_lock.acquire()
    if new or g_zhima_last_proxy is None:
        g_zhima_last_proxy = request_proxy_from_zhima()
    else:
        current_time = datetime.datetime.now()
        if current_time >= g_zhima_last_proxy['expire_time']:  # A simple, non-strict time check; the two clocks may not be synchronized
            g_zhima_last_proxy = request_proxy_from_zhima()
    g_zhima_lock.release()
    return '{}:{}'.format(
        g_zhima_last_proxy['ip'], g_zhima_last_proxy['port']
    ) if g_zhima_last_proxy else None
