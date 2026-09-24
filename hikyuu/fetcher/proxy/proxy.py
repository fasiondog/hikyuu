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
    """Make a request through the proxy; an exception is raised when the access fails"""
    # The obtained ip may not be able to access the url; retry 10 times to find a usable proxy
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
    raise Exception("Unable to access through the proxy!")


def request_with_local(url):
    """Make the request directly with the local ip; an exception is raised when the access fails"""
    return requests.get(url).text