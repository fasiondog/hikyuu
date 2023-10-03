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

from hikyuu.cpp.core import KData, Indicator, SignalBase, ConditionBase, System

import matplotlib
from matplotlib.pylab import gca as mpl_gca
from matplotlib.pylab import gcf as mpl_gcf
from .matplotlib_draw import set_mpl_params
from .matplotlib_draw import create_figure as mpl_create_figure
from .matplotlib_draw import kplot as mpl_kplot
from .matplotlib_draw import mkplot as mpl_mkplot
from .matplotlib_draw import iplot as mpl_iplot
from .matplotlib_draw import ibar as mpl_ibar
from .matplotlib_draw import sgplot as mpl_sgplot
from .matplotlib_draw import cnplot as mpl_cnplot
from .matplotlib_draw import sysplot as mpl_sysplot
from .matplotlib_draw import ax_draw_macd as mpl_ax_draw_macd
from .matplotlib_draw import ax_draw_macd2 as mpl_ax_draw_macd2
from .matplotlib_draw import ax_set_locator_formatter as mpl_ax_set_locator_formatter
from .matplotlib_draw import adjust_axes_show as mpl_adjust_axes_show

from .bokeh_draw import gcf as bk_gcf
from .bokeh_draw import gca as bk_gca
from .bokeh_draw import create_figure as bk_create_figure
from .bokeh_draw import show_gcf as bk_show_gcf
from .bokeh_draw import kplot as bk_kplot
from .bokeh_draw import iplot as bk_iplot
from .bokeh_draw import ibar as bk_ibar
from .bokeh_draw import ax_draw_macd as bk_ax_draw_macd
from .bokeh_draw import ax_draw_macd2 as bk_ax_draw_macd2
from .bokeh_draw import sgplot as bk_sgplot
from .bokeh_draw import use_bokeh_in_notebook

from .echarts_draw import sysplot as ec_sysplot
from .echarts_draw import iplot as ec_iplot
from .echarts_draw import ibar as ec_ibar
from .echarts_draw import kplot as ec_kplot

from .common import in_ipython_frontend

g_draw_engine = 'matplotlib'


def get_current_draw_engine():
    global g_draw_engine
    return g_draw_engine


def set_current_draw_engine(engine):
    global g_draw_engine
    g_draw_engine = engine


def use_draw_engine(engine='matplotlib'):
    if engine == 'matplotlib':
        use_draw_with_matplotlib()
    elif engine == 'bokeh':
        use_draw_with_bokeh()
    elif engine == 'echarts':
        use_draw_with_echarts()
    else:
        print("未知的引擎: {}".format(engine))


def use_draw_with_bokeh():
    if in_ipython_frontend():
        use_bokeh_in_notebook(True)
    set_current_draw_engine('bokeh')

    KData.plot = bk_kplot
    KData.kplot = bk_kplot

    Indicator.plot = bk_iplot
    Indicator.bar = bk_ibar

    SignalBase.plot = bk_sgplot


def use_draw_with_matplotlib():
    set_current_draw_engine('matplotlib')
    set_mpl_params()

    KData.plot = mpl_kplot
    KData.kplot = mpl_kplot
    KData.mkplot = mpl_mkplot

    Indicator.plot = mpl_iplot
    Indicator.bar = mpl_ibar

    SignalBase.plot = mpl_sgplot
    ConditionBase.plot = mpl_cnplot

    System.plot = mpl_sysplot

def use_draw_with_echarts():
    set_current_draw_engine('echarts')

    KData.plot = ec_kplot
    KData.kplot = ec_kplot
    # KData.mkplot = ec_mkplot

    Indicator.plot = ec_iplot
    Indicator.bar = ec_ibar

    System.plot = ec_sysplot

def create_figure(n=1, figsize=None):
    """生成含有指定坐标轴数量的窗口，最大只支持4个坐标轴。

    :param int n: 坐标轴数量
    :param figsize: (宽, 高)
    :return: (ax1, ax2, ...) 根据指定的坐标轴数量而定，超出[1,4]个坐标轴时，返回None
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_create_figure(n, figsize if figsize else (10, 8))
    elif engine == 'bokeh':
        return bk_create_figure(n, figsize if figsize else (800, 800))
    else:
        print("未知的引擎: {}".format(engine))


def gcf():
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_gcf()
    elif engine == 'bokeh':
        return bk_gcf()
    else:
        print("未知的引擎: {}".format(engine))


def gca():
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_gca()
    elif engine == 'bokeh':
        return bk_gca()
    else:
        print("未知的引擎: {}".format(engine))


def show_gcf():
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        pass
    elif engine == 'bokeh':
        bk_show_gcf()
    else:
        print("未知的引擎: {}".format(engine))


def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """绘制MACD
    
    :param axes: 指定的坐标轴
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_ax_draw_macd(axes, kdata, n1, n2, n3)
    elif engine == 'bokeh':
        pass
    else:
        print("未知的引擎: {}".format(engine))


def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """绘制MACD
    
    :param axes: 指定的坐标轴
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_ax_draw_macd(axes, kdata, n1, n2, n3)
    elif engine == 'bokeh':
        return bk_ax_draw_macd(axes, kdata, n1, n2, n3)
    else:
        print("未知的引擎: {}".format(engine))


def ax_draw_macd2(axes, ref, kdata, n1=12, n2=26, n3=9):
    """绘制MACD。
    当BAR值变化与参考序列ref变化不一致时，显示为灰色，
    当BAR和参考序列ref同时上涨，显示红色
    当BAR和参考序列ref同时下跌，显示绿色

    :param axes: 指定的坐标轴
    :param ref: 参考序列，EMA
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_ax_draw_macd2(axes, ref, kdata, n1, n2, n3)
    elif engine == 'bokeh':
        bk_ax_draw_macd2(axes, ref, kdata, n1, n2, n3)
    else:
        print("未知的引擎: {}".format(engine))


def adjust_axes_show(axeslist):
    """用于调整上下紧密相连的坐标轴显示时，其上一坐标轴最小值刻度和下一坐标轴最大值刻度
    显示重叠的问题。
    
    :param axeslist: 上下相连的坐标轴列表 (ax1,ax2,...)
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_adjust_axes_show(axeslist)
    elif engine == 'bokeh':
        pass
    else:
        print("未知的引擎: {}".format(engine))


def ax_set_locator_formatter(axes, dates, typ):
    """ 设置指定坐标轴的日期显示，根据指定的K线类型优化X轴坐标显示
    
    :param axes: 指定的坐标轴
    :param dates: Datetime构成可迭代序列
    :param Query.KType typ: K线类型
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_ax_set_locator_formatter(axes, dates, typ)
    elif engine == 'bokeh':
        pass
    else:
        print("未知的引擎: {}".format(engine))


# 设置默认引擎
use_draw_engine('matplotlib')

__all__ = [
    'use_draw_engine',
    'get_current_draw_engine',
    'create_figure',
    'gcf',
    'show_gcf',
    'gca',
    'ax_draw_macd',
    'ax_draw_macd2',
    'use_bokeh_in_notebook',
    'use_draw_with_echarts'
]
