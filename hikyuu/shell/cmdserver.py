#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2021 hikyuu.org
#
# Create on: 2021-03-23
#    Author: fasiondog

import requests, json
from hkucmd import HKUShell

g_server_url = "http://127.0.0.1:520/hku"
g_api_version = "v1"
g_token = None


def get_url(service, api):
    return "{}/{}/{}/{}".format(g_server_url, service, g_api_version, api)


def login():
    data = json.dumps({'user': 'hku'})
    url = get_url('login', 'login')
    r = requests.post(url, data)
    global g_token
    g_token = r.json()['hku_token']


def logout():
    url = get_url('login', 'logout')
    r = requests.post(url, data)


def get_token():
    global g_token
    if g_token is None:
        login()
    return g_token


def get_headers():
    headers = {"hku_token": get_token(), "User-Agent": "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.103 Safari/537.36"}
    return headers


def status():
    url = get_url('assist', 'status')
    headers = get_headers()
    r = requests.get(url, headers=headers)
    print(r.json())


def set_logger_level(logger, level):
    url = get_url('assist', 'log_level')
    headers = get_headers()
    data = {'logger': logger, 'level': int(level)} if logger is not None else {'level': int(level)}
    r = requests.post(url, data, headers=headers)
    print(r.json())


def server(self, args):
    """
    status: 查看当前服务器运行状态
    """
    x = args.split()
    if not x or x[0] == 'status':
        status()
    elif x[0] == 'set_logger_level':
        if len(x) == 2:
            set_logger_level(None, x[1])
        else:
            set_logger_level(x[1], x[2])
