#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#==========================================================
# History
# 1. 20200821, Added by fasiondog
#==========================================================

from .mylog import mylogger


class HKUCheckError(Exception):
    def __init__(self, expression, message):
        self.expression = expression
        self.message = message

    def __str__(self):
        return self.message


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


def HKU_CHECK(exp, msg):
    if not exp:
        raise HKUCheckError(exp, msg)


def HKU_CHECK_THROW(expression, message, excepion=None, **kwargs):
    """如果 expression 为 False，则抛出异常。

    :param boolean expression: 判断条件
    :param str message: 异常注解信息
    :param Exception exception: 指定的异常类，为None时，为默认 HKUCheckError 异常
    """
    if not expression:
        if excepion is None:
            raise HKUCheckError(expression, message)
        else:
            raise excepion(message, **kwargs)
