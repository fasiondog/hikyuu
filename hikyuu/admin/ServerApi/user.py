# -*- coding: utf-8 -*-

import requests
from .config import getServerApiUrl, defaultRequestHeader
from .restful import post

def login(host: str, user: str, password: str):
    url = getServerApiUrl(host, "user", "login")
    headers = defaultRequestHeader()
    return post(url, headers=headers, json={"user": user, "password": password})


if __name__ == "__main__":
    r = login("http://127.0.0.1:9001", "1", "1")
    print(r.status_code)
    print(r.json())
