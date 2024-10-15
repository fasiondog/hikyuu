#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ==========================================================
# History
# 1. 20200821, Added by fasiondog
# ==========================================================

import sys
import traceback
import functools
import asyncio
from .mylog import hku_logger, g_hku_logger_lock


class HKUCheckError(Exception):
    def __init__(self, expression, message):
        self.expression = expression
        self.message = message

    def __str__(self):
        return str(self.message)


class HKUIngoreError(Exception):
    def __init__(self, expression, message=None):
        self.expression = expression
        self.message = message if message is not None else 'ignore'

    def __str__(self):
        return str(self.message)


def checkif(expression, message, excepion=None, **kwargs):
    """如果 expression 为 True，则抛出异常。注意：该函数的判定和 assert 是相反的。

    :param boolean expression: 判断条件
    :param str message: 异常注解信息
    :param Exception exception: 指定的异常类，为None时，为默认 HKUCheckError 异常
    """
    if expression:
        if excepion is None:
            raise HKUCheckError(expression, message)
        else:
            raise excepion(message, **kwargs)


def hku_check(exp, msg, *args, **kwargs):
    if not exp:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        errmsg = "{}) {} [{}] [{}:{}]".format(check_exp, msg.format(*args, **kwargs), st.name, st.filename, st.lineno)
        raise HKUCheckError(exp, errmsg)


def hku_check_throw(expression, message, excepion=None, **kwargs):
    """如果 expression 为 False，则抛出异常。

    :param boolean expression: 判断条件
    :param str message: 异常注解信息
    :param Exception exception: 指定的异常类，为None时，为默认 HKUCheckError 异常
    """
    if not expression:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        errmsg = "{}) {} [{}] [{}:{}]".format(check_exp, message, st.name, st.filename, st.lineno)
        if excepion is None:
            raise HKUCheckError(expression, errmsg)
        else:
            raise excepion(errmsg, **kwargs)


def hku_check_ignore(exp, *args, **kwargs):
    """可忽略的检查"""
    if not exp:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        msg = kwargs.pop("msg") if "msg" in kwargs else ''
        if msg:
            errmsg = "{}) {} [{}] [{}:{}]".format(
                check_exp, msg.format(*args, **kwargs), st.name, st.filename, st.lineno
            )
        elif args:
            msg = args[0]
            errmsg = "{}) {} [{}] [{}:{}]".format(
                check_exp, msg.format(*args[1:], **kwargs), st.name, st.filename, st.lineno
            )
        raise HKUIngoreError(exp, errmsg)


def get_exception_info():
    info = sys.exc_info()
    return "{}: {}".format(info[0].__name__, info[1])


def hku_catch(ret=None, trace=False, callback=None, retry=1, with_msg=False, re_raise=False):
    """捕获发生的异常, 包装方式: @hku_catch()
    :param ret: 异常发生时返回值, with_msg为True时, 返回为 (ret, errmsg)
    :param boolean trace: 打印异常堆栈信息
    :param func callback: 发生异常后的回调函数, 入参同func
    :param int retry: 尝试执行的次数
    :param boolean with_msg: 是否返回异常错误信息, 为True时, 函数返回为 (ret, errmsg)
    :param boolean re_raise: 是否将错误信息以异常的方式重新抛出
    """
    def hku_catch_wrap(func):
        @functools.wraps(func)
        def wrappedFunc(*args, **kargs):
            for i in range(retry):
                errmsg = ""
                try:
                    val = func(*args, **kargs)
                    return (val, errmsg) if with_msg else val
                except HKUIngoreError:
                    errmsg = "{} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    hku_logger.debug(errmsg)
                except Exception:
                    errmsg = "{} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    with g_hku_logger_lock:
                        hku_logger.error(errmsg)
                        if trace:
                            hku_logger.error(traceback.format_exc())
                    if i == (retry - 1):
                        if callback is not None:
                            callback(*args, **kargs)
                        if re_raise:
                            raise Exception(errmsg)
                except KeyboardInterrupt:
                    raise KeyboardInterrupt()
                except:
                    errmsg = "Unknown error! {} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    with g_hku_logger_lock:
                        hku_logger.error(errmsg)
                        if trace:
                            hku_logger.error(traceback.format_exc())
                    if i == (retry - 1):
                        if callback is not None:
                            callback(*args, **kargs)
                        if re_raise:
                            raise Exception(errmsg)
                return (ret, errmsg) if with_msg else ret

        return wrappedFunc

    return hku_catch_wrap


def hku_to_async(func):
    @functools.wraps(func)
    async def async_func(*args, **kwargs):
        loop = asyncio.get_event_loop()
        return await loop.run_in_executor(None, func, *args, **kwargs)
    return async_func
