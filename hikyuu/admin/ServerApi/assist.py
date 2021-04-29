# -*- coding: utf-8 -*-

import requests
from .config import getServerApiUrl, defaultRequestHeader


def getServerStatus(host, token):
    url = getServerApiUrl(host, "assist", "status")
    headers = defaultRequestHeader()
    headers["hku_token"] = token
    return requests.get(url, headers=headers)
