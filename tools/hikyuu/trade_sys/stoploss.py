#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

StoplossBase = csys.StoplossBase
StoplossBase.__unicode__ = unicodeFunc
StoplossBase.__repr__ = reprFunc

ST_FixedPercent = csys.ST_FixedPercent
ST_Indicator = csys.ST_Indicator
ST_Saftyloss = csys.ST_Saftyloss

ST_FixedPercent.__doc__ =  """
    固定百分比止损策略
    p: (0, 1]
    """
