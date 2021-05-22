# -*- coding: utf-8 -*-

import requests
import functools
from .config import getserviceUrl, defaultRequestHeader
from data import SessionModel


class HttpStatusError(Exception):
    """ others http status """
    def __init__(self, status):
        super(HttpStatusError, self).__init__()
        self.status = status

    def __str__(self):
        return "Http status {}".format(self.status)


class HttpInternalServerError(HttpStatusError):
    """ http 500 """
    def __init__(self):
        super(HttpInternalServerError, self).__init__(500)

    def __str__(self):
        return "Http status 500: Internal Server Error"


class RestfulError(Exception):
    def __init__(self, res):
        super(RestfulError, self).__init__()
        self.errcode = res["errcode"]
        self.errmsg = res["errmsg"]

    def __str__(self):
        return self.errmsg


def wrap_restful(func):
    @functools.wraps(func)
    def wrap_func(*args, **kwargs):
        try:
            r = func(*args, **kwargs)
            if r.status_code in (200, 400, 401):
                return r.json()
            elif r.status_code == 500:
                raise HttpInternalServerError()
            else:
                raise HttpStatusError(r.status_code)
        except Exception as e:
            raise e

    return wrap_func


@wrap_restful
def get(url, params=None, **kwargs):
    return requests.get(url, params, **kwargs)


@wrap_restful
def post(url, data=None, json=None, **kwargs):
    return requests.post(url, data, json, **kwargs)


@wrap_restful
def delete(url, **kwargs):
    return requests.delete(url, **kwargs)


@wrap_restful
def patch(url, data=None, **kwargs):
    return requests.patch(url, data, **kwargs)


@wrap_restful
def put(url, data=None, **kwargs):
    return requests.put(url, data, **kwargs)


class RestErrorCode:
    MISS_TOKEN = 10000
    FAILED_AUTHORIZED = 10001
    AUTHORIZE_EXPIRED = 10002


def login(session: SessionModel):
    url = getserviceUrl(session.host, session.port, "user", "login")
    headers = defaultRequestHeader()
    res = post(url, headers=headers, json={"user": session.user, "password": session.password})
    if res["result"]:
        session.token = res["token"]
        session.userid = res["userid"]
    else:
        raise RestfulError(res)
    return session


def check_res(res):
    if not res["result"]:
        raise RestfulError(res)


def session_get(session: SessionModel, service, api, params=None, **kwargs):
    def inner_get(session: SessionModel, service, api, params, **kwargs):
        url = getserviceUrl(session.host, session.port, service, api)
        headers = defaultRequestHeader()
        headers["hku_token"] = session.token
        r = get(url, headers=headers, params=params, **kwargs)
        if "update_token" in r:
            session.token = r["update_token"]
        return r

    session.running = False
    if not session.token:
        session = login(session)
    res = inner_get(session, service, api, params, **kwargs)
    if not res["result"] and (
        res['errcode'] == RestErrorCode.AUTHORIZE_EXPIRED or res['errcode'] == RestErrorCode.FAILED_AUTHORIZED
    ):
        session = login(session)
        res = inner_get(session, service, api, params, **kwargs)
    session.running = True if res["result"] else False
    return res


def session_post(session: SessionModel, service, api, data=None, json=None, **kwargs):
    def inner_func(session: SessionModel, service, api, data=None, json=None, **kwargs):
        url = getserviceUrl(session.host, session.port, service, api)
        headers = defaultRequestHeader()
        headers["hku_token"] = session.token
        r = post(url, data, json, headers=headers, **kwargs)
        if "update_token" in r:
            session.token = r["update_token"]
        return r

    session.running = False
    if not session.token:
        session = login(session)
    res = inner_func(session, service, api, data, json, **kwargs)
    if not res["result"] and (
        res['errcode'] == RestErrorCode.AUTHORIZE_EXPIRED or res['errcode'] == RestErrorCode.FAILED_AUTHORIZED
    ):
        session = login(session)
        res = inner_func(session, service, api, data, json, **kwargs)
    session.running = True if res["result"] else False
    return res


def session_delete(session: SessionModel, service, api, json, **kwargs):
    def inner_func(session: SessionModel, service, api, json, **kwargs):
        url = getserviceUrl(session.host, session.port, service, api)
        headers = defaultRequestHeader()
        headers["hku_token"] = session.token
        r = delete(url, json=json, headers=headers, **kwargs)
        if "update_token" in r:
            session.token = r["update_token"]
        return r

    session.running = False
    if not session.token:
        session = login(session)
    res = inner_func(session, service, api, json, **kwargs)
    if not res["result"] and (
        res['errcode'] == RestErrorCode.AUTHORIZE_EXPIRED or res['errcode'] == RestErrorCode.FAILED_AUTHORIZED
    ):
        session = login(session)
        res = inner_func(session, service, api, json, **kwargs)
    session.running = True if res["result"] else False
    return res


def session_put(session: SessionModel, service, api, json, **kwargs):
    def inner_func(session: SessionModel, service, api, json, **kwargs):
        url = getserviceUrl(session.host, session.port, service, api)
        headers = defaultRequestHeader()
        headers["hku_token"] = session.token
        r = put(url, json=json, headers=headers, **kwargs)
        if "update_token" in r:
            session.token = r["update_token"]
        return r

    session.running = False
    if not session.token:
        session = login(session)
    res = inner_func(session, service, api, json, **kwargs)
    if not res["result"] and (
        res['errcode'] == RestErrorCode.AUTHORIZE_EXPIRED or res['errcode'] == RestErrorCode.FAILED_AUTHORIZED
    ):
        session = login(session)
        res = inner_func(session, service, api, json, **kwargs)
    session.running = True if res["result"] else False
    return res
