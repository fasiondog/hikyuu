# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

__version__ = "0.0.1"

from .proxy import request_with_proxy, request_with_local

__all__ = ['request_with_proxy', 'request_with_local']