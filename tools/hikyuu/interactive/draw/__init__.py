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

#===============================================================================
# history:
# 1. 20171122, Added by fasiondog
#===============================================================================

from hikyuu import KData
from hikyuu.indicator import Indicator

from .echart_draw import create_figure as ect_create_figure
from .echart_draw import kplot as ect_kplot
from .echart_draw import mkplot as ect_mkplot
from .echart_draw import iplot as ect_iplot
from .echart_draw import ibar as ect_ibar

from .matplotlib_draw import create_figure as mpl_create_figure
from .matplotlib_draw import kplot as mpl_kplot
from .matplotlib_draw import mkplot as mpl_mkplot
from .matplotlib_draw import iplot as mpl_iplot
from .matplotlib_draw import ibar as mpl_ibar


g_engine = 'matplotlib'

def use_draw_engine(engine='matplotlib'):
    if engine == 'echarts':
        use_draw_with_echarts()
    else:
        use_draw_with_matplotlib()
        
     
def use_draw_with_matplotlib():
    KData.plot = mpl_kplot
    KData.kplot = mpl_kplot
    KData.mkplot = mpl_mkplot
    
    Indicator.plot = mpl_iplot
    Indicator.bar = mpl_ibar
    
    global g_engine
    g_engine = 'matplotlib'
    

def use_draw_with_echarts():
    KData.plot = ect_kplot
    KData.kplot = ect_kplot
    KData.mkplot = ect_mkplot
    
    Indicator.plot = ect_iplot
    Indicator.bar = ect_ibar
    
    global g_engine
    g_engine = 'echarts'

def create_figure(n=1, figsize=(8,6)):
    if g_engine == 'matplotlib':
        return mpl_create_figure(n, figsize)
    else:
        return ect_create_figure(n, figsize)
    

__all__ = ['use_draw_engine',
           'create_figure']

