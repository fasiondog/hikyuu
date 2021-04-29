# -*- coding: utf-8 -*-

server_api_config = {"protocol": "http", "prefix": "hku", "version": "v1"}


def getServerApiUrl(host_url, service, api):
    return "{}://{}/{}/{}/{}/{}".format(
        server_api_config["protocol"], host_url, server_api_config["prefix"], service, server_api_config["version"], api
    )


def defaultRequestHeader():
    return {
        "Content-Type": "application/json",
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:88.0) Gecko/20100101 Firefox/88.0",
        "Accept-Encoding": "gzip, deflate, br"
    }
