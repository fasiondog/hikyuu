# -*- coding: utf-8 -*-

import requests
from .config import getServerApiUrl, defaultRequestHeader
from .restful import post

from data import SessionModel


def login(session: SessionModel):
    url = getServerApiUrl(session.host, session.port, "user", "login")
    headers = defaultRequestHeader()
    return post(url, headers=headers, json={"user": session.user, "password": session.password})


def update_token(res, session: SessionModel):
    """ 如果响应中包含 "update_token, 则更新相应的session中的token

    :param SessionModel session: 会话
    :param res: 已经加码json的请求响应数据
    """
    if "update_token" in res:
        session.token = res["update_token"]


if __name__ == "__main__":
    r = login("http://127.0.0.1:9001", "1", "1")
    print(r.status_code)
    print(r.json())
