#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#==========================================================
# History
# 1. 20200821, Added by fasiondog
#==========================================================

import sys
import traceback
import functools
from .mylog import hku_logger


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


def hku_check(exp, msg):
    if not exp:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        errmsg = "{}) {} [{}] [{}:{}]".format(check_exp, msg, st.name, st.filename, st.lineno)
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


def hku_check_ignore(exp, msg=None):
    """可忽略的检查"""
    if not exp:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        errmsg = "{}) {} [{}] [{}:{}]".format(check_exp, msg, st.name, st.filename, st.lineno)
        raise HKUIngoreError(exp, errmsg)


def get_exception_info():
    info = sys.exc_info()
    return "{}: {}".format(info[0].__name__, info[1])


def hku_catch(ret=None, trace=False, callback=None, retry=1):
    """捕获发生的异常, 包装方式: @hku_catch()
    :param ret: 异常发生时返回值
    :param boolean trace: 打印异常堆栈信息
    :param func callback: 发生异常后的回调函数，入参同func
    :param int retry: 发生异常后，重复尝试执行的次数
    """
    def hku_catch_wrap(func):
        @functools.wraps(func)
        def wrappedFunc(*args, **kargs):
            for i in range(retry):
                try:
                    return func(*args, **kargs)
                except HKUIngoreError:
                    hku_logger.debug(
                        "{} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    )
                except Exception:
                    hku_logger.error(
                        "{} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    )
                    if trace:
                        traceback.print_exc()
                    if callback and i == (retry - 1):
                        callback(*args, **kargs)
                except:
                    hku_logger.error(
                        "Unknown error! {} [{}.{}]".format(
                            get_exception_info(), func.__module__, func.__name__
                        )
                    )
                    if trace:
                        traceback.print_exc()
                    if callback and i == (retry - 1):
                        callback(*args, **kargs)
                return ret

        return wrappedFunc

    return hku_catch_wrap