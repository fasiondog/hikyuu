# -*- coding: utf-8 -*-

from PyQt5 import QtCore

server_api_config = {
    "protocol": "http",
    "prefix": "hku",
    "version": "v1",
}


def getserviceUrl(host_url, port, service, api):
    return "{}://{}:{}/{}/{}/{}/{}".format(
        server_api_config["protocol"], host_url, port, server_api_config["prefix"], service,
        server_api_config["version"], api
    )


def defaultRequestHeader():
    header = {
        "Content-Type": "application/json",
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:88.0) Gecko/20100101 Firefox/88.0",
        "Accept-Encoding": "gzip, deflate, br"
    }
    loc = QtCore.QLocale()
    if loc.language() == QtCore.QLocale.Chinese:
        header["Accept-Language"] = "zh_CN"
    return header
