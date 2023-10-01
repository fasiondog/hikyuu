#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from .indicator import *


def KDJ(kdata=None, n=9, m1=3, m2=3):
    """ 经典 KDJ 随机指标

        :param KData kdata: 关联的K线数据
        :param int n:
        :param int m1:
        :param int m2:
        :return: k, d, j
    """
    rsv = (CLOSE() - LLV(LOW(), n)) / (HHV(HIGH(), n) - LLV(LOW(), n)) * 100
    k = SMA(rsv, m1, 1)
    d = SMA(k, m2, 1)
    j = 3 * k - 2 * d
    if kdata is not None:
        k.set_context(kdata)
        j.set_context(kdata)
        d.set_context(kdata)
    return k, d, j


def RSI_PY(kdata=None, N1=6, N2=12, N3=24):
    """相对强弱指标

        :param KData kdata: 关联的K线数据
        :param int N1: 参数N1
        :param int N2: 参数N1
        :param int N3: 参数N1
        :return: rsi1, rsi2, rsi3
    """
    LC = REF(CLOSE(), 1)
    rsi1 = SMA(MAX(CLOSE() - LC, 0), N1, 1) / SMA(ABS(CLOSE() - LC), N1, 1) * 100
    rsi2 = SMA(MAX(CLOSE() - LC, 0), N2, 1) / SMA(ABS(CLOSE() - LC), N2, 1) * 100
    rsi3 = SMA(MAX(CLOSE() - LC, 0), N3, 1) / SMA(ABS(CLOSE() - LC), N3, 1) * 100

    if kdata is not None:
        rsi1.set_context(kdata)
        rsi2.set_context(kdata)
        rsi3.set_context(kdata)
    return rsi1, rsi2, rsi3
