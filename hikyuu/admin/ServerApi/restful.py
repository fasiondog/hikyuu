# -*- coding: utf-8 -*-

import requests
import functools


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


def wrap_restful(func):
    @functools.wraps(func)
    def wrap_func(*args, **kwargs):
        try:
            r = func(*args, **kwargs)
            if r.status_code in (200, 400):
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
