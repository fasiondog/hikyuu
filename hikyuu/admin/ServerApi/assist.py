# -*- coding: utf-8 -*-

from .config import getServerApiUrl, defaultRequestHeader
from .restful import get


def getServerStatus(host, token):
    url = getServerApiUrl(host, "assist", "status")
    headers = defaultRequestHeader()
    headers["hku_token"] = token
    return get(url, headers=headers)
