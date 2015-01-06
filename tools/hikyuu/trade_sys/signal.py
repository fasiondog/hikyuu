#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

SignalBase = csys.SignalBase
SignalBase.__unicode__ = unicodeFunc
SignalBase.__repr__ = reprFunc

def AMA_SG(n = 10, fast_n = 2, slow_n = 30, filter_n = 20, filter_p = 0.1,
           kpart = "CLOSE"):
    """
    佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
    参数：
        n 周期窗口
        fast_n 快速损耗周期
        slown_ 慢速损耗周期
        filter_n 拐点判定样本标准差窗口
        filter_p 拐点判定系数
        kpart 取KDATA的哪个部分
    """
    return csys.AMA_SG(n, fast_n, slow_n, filter_n, filter_p, kpart)