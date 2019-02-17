#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
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


from ._trade_sys import (AllocateFundsBase,
                         SystemWeight,
                         SystemWeightList, 
                         AF_EqualWeight)
from hikyuu.util.unicode import (unicodeFunc, reprFunc)
from hikyuu.util.slice import list_getitem

AllocateFundsBase.__unicode__ = unicodeFunc
AllocateFundsBase.__repr__ = reprFunc

SystemWeight.__unicode__ = unicodeFunc
SystemWeight.__repr = reprFunc

SystemWeightList.__getitem__ = list_getitem

#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------



#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------
