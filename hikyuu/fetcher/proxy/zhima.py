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

# 芝麻 http 代理获取代理 API url，本机ip地址需在芝麻的白名单内
zhimahttp_url = 'http://webapi.http.zhimacangku.com/getip?num=1&type=2&pro=0&city=0&yys=0&port=1&time=1&ts=1&ys=1&cs=1&lb=1&sb=0&pb=45&mr=2&regions=110000,130000,140000,310000,320000,330000,370000,410000'


@hku_catch(retry=10)
def request_proxy_from_zhima():
    """从芝麻申请代理IP

    返回格式如下：
    {'ip': '115.226.137.200',
    'port': 4223,
    'expire_time': '2020-12-01 00:12:33'（实际为datetime类型）
    'city': '浙江省丽水市',
    'isp': '电信',
    'request_time': datetime类型，申请时间}
    """
    ret = requests.get(zhimahttp_url).text
    x = json.loads(ret)
    hku_check(x['success'], "获取代理失败！")
    x = x['data'][0]
    x['expire_time'] = datetime.datetime.fromisoformat(x['expire_time'])
    x['request_time'] = datetime.datetime.now()
    return x


# 多线程时锁住申请
g_zhima_lock = threading.Lock()

# 最后使用的代理
g_zhima_last_proxy = None


def get_proxy(new=False):
    """从芝麻获取代理

    :param boolean new: True 强制重新申请 | False 尽可能使用上次获取的代理ip
    :return: None | 'host ip:port'， 如: '183.164.239.57:4264'
    """
    global g_zhima_last_proxy
    g_zhima_lock.acquire()
    if new or g_zhima_last_proxy is None:
        g_zhima_last_proxy = request_proxy_from_zhima()
    else:
        current_time = datetime.datetime.now()
        if current_time >= g_zhima_last_proxy['expire_time']:  # 简单不严格的时间判断，两者时间可能不同步
            g_zhima_last_proxy = request_proxy_from_zhima()
    g_zhima_lock.release()
    return '{}:{}'.format(
        g_zhima_last_proxy['ip'], g_zhima_last_proxy['port']
    ) if g_zhima_last_proxy else None
