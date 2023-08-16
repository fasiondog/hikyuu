# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

import requests
import json
import datetime
import time
from hikyuu.util import hku_warn, hku_info, hku_check

from .zhima import get_proxy


def request_with_proxy(url):
    """通过代理进行请求，访问失败将抛出异常"""
    # 获取到的 ip 可能无法访问相应的 url，重试10次，以便找到能用的 proxy
    new = False
    proxies = {'http': '127.0.0.1'}
    for i in range(10):  # pylint: disable=unused-variable
        try:
            proxy = get_proxy(new)
            hku_check(proxy, "Failed get proxy!")
            proxies = {'http': 'http://{}'.format(proxy)}
            result = requests.get(url, proxies=proxies, timeout=5).text
            #hku_info("use proxy: {}".format(proxies['http']))
            return result
        except Exception as e:
            if i == 6:
                new = True
            time.sleep(2)
    raise Exception("无法通过代理访问！")


def request_with_local(url):
    """通过本机ip直接获取请求，访问失败将抛出异常"""
    return requests.get(url).text