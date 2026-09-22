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

# ===============================================================================
# history:
# 1. 20171122, Added by fasiondog
# ===============================================================================

from hikyuu.core import KData, Indicator, SignalBase, ConditionBase, EnvironmentBase, System, Portfolio, TradeManager, htr

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
from .matplotlib_draw import evplot as mpl_evplot
from .matplotlib_draw import sysplot as mpl_sysplot
from .matplotlib_draw import ax_draw_macd as mpl_ax_draw_macd
from .matplotlib_draw import ax_draw_macd2 as mpl_ax_draw_macd2
from .matplotlib_draw import ax_set_locator_formatter as mpl_ax_set_locator_formatter
from .matplotlib_draw import adjust_axes_show as mpl_adjust_axes_show
from .matplotlib_draw import sys_performance as mpl_sys_performance
from .matplotlib_draw import tm_performance as mpl_tm_performance
from .matplotlib_draw import tm_heatmap as mpl_tm_heatmap
from .matplotlib_draw import tm_year_profit as mpl_tm_year_profit
from .matplotlib_draw import sys_heatmap as mpl_sys_heatmap
from .matplotlib_draw import iheatmap as mpl_iheatmap
from .matplotlib_draw import (DRAWNULL, STICKLINE, DRAWBAND, RGB, PLOYLINE,
                              DRAWLINE, DRAWTEXT, DRAWNUMBER, DRAWTEXT_FIX, DRAWNUMBER_FIX, DRAWSL,
                              DRAWIMG, DRAWICON, DRAWBMP, SHOWICONS, DRAWRECTREL)

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
from .echarts_draw import sys_performance as ec_sys_performance

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
        print(htr("Unknown engine: {}").format(engine))


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
    Indicator.heatmap = mpl_iheatmap

    SignalBase.plot = mpl_sgplot

    EnvironmentBase.plot = mpl_evplot
    ConditionBase.plot = mpl_cnplot

    TradeManager.performance = mpl_tm_performance
    System.plot = mpl_sysplot
    System.performance = mpl_sys_performance
    Portfolio.performance = mpl_sys_performance
    TradeManager.heatmap = mpl_tm_heatmap
    TradeManager.year_profit_bar = mpl_tm_year_profit
    System.heatmap = mpl_sys_heatmap
    Portfolio.heatmap = mpl_sys_heatmap


def use_draw_with_echarts():
    set_current_draw_engine('echarts')

    KData.plot = ec_kplot
    KData.kplot = ec_kplot
    # KData.mkplot = ec_mkplot

    Indicator.plot = ec_iplot
    Indicator.bar = ec_ibar

    System.plot = ec_sysplot
    System.performance = ec_sys_performance
    Portfolio.performance = ec_sys_performance


def create_figure(n=1, figsize=None):
    """Generate a window containing the specified number of axes; at most 4 axes are supported.

    :param int n: the number of the axes
    :param figsize: (width, height)
    :return: (ax1, ax2, ...) depending on the specified number of the axes; None is returned
        when the number is out of [1, 4]
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_create_figure(n, figsize if figsize else (10, 8))
    elif engine == 'bokeh':
        return bk_create_figure(n, figsize if figsize else (800, 800))
    else:
        print(htr("Unknown engine: {}").format(engine))


def gcf():
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_gcf()
    elif engine == 'bokeh':
        return bk_gcf()
    else:
        print(htr("Unknown engine: {}").format(engine))


def gca():
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_gca()
    elif engine == 'bokeh':
        return bk_gca()
    else:
        print(htr("Unknown engine: {}").format(engine))


def show_gcf():
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        pass
    elif engine == 'bokeh':
        bk_show_gcf()
    else:
        print(htr("Unknown engine: {}").format(engine))


def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """Draw MACD

    :param axes: the specified axes
    :param KData kdata: KData
    :param int n1: the parameter 1 of the MACD indicator
    :param int n2: the parameter 2 of the MACD indicator
    :param int n3: the parameter 3 of the MACD indicator
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_ax_draw_macd(axes, kdata, n1, n2, n3)
    elif engine == 'bokeh':
        pass
    else:
        print(htr("Unknown engine: {}").format(engine))


def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """Draw MACD

    :param axes: the specified axes
    :param KData kdata: KData
    :param int n1: the parameter 1 of the MACD indicator
    :param int n2: the parameter 2 of the MACD indicator
    :param int n3: the parameter 3 of the MACD indicator
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        return mpl_ax_draw_macd(axes, kdata, n1, n2, n3)
    elif engine == 'bokeh':
        return bk_ax_draw_macd(axes, kdata, n1, n2, n3)
    else:
        print(htr("Unknown engine: {}").format(engine))


def ax_draw_macd2(axes, ref, kdata, n1=12, n2=26, n3=9):
    """Draw MACD.
    It is drawn in gray when the change of the BAR value is inconsistent with that of the
    reference sequence ref, in red when both BAR and the reference sequence ref rise, and in
    green when both of them fall.

    :param axes: the specified axes
    :param ref: the reference sequence, EMA
    :param KData kdata: KData
    :param int n1: the parameter 1 of the MACD indicator
    :param int n2: the parameter 2 of the MACD indicator
    :param int n3: the parameter 3 of the MACD indicator
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_ax_draw_macd2(axes, ref, kdata, n1, n2, n3)
    elif engine == 'bokeh':
        bk_ax_draw_macd2(axes, ref, kdata, n1, n2, n3)
    else:
        print(htr("Unknown engine: {}").format(engine))


def adjust_axes_show(axeslist):
    """Adjust the display of the axes that are closely connected up and down, so that the
    minimum tick of the upper axis does not overlap the maximum tick of the lower one.

    :param axeslist: the list of the axes connected up and down (ax1, ax2, ...)
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_adjust_axes_show(axeslist)
    elif engine == 'bokeh':
        pass
    else:
        print(htr("Unknown engine: {}").format(engine))


def ax_set_locator_formatter(axes, dates, typ):
    """Set the date display of the specified axes, and optimize the X-axis display
    according to the specified K-line type

    :param axes: the specified axes
    :param dates: an iterable sequence of Datetime
    :param Query.KType typ: the K-line type
    """
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        mpl_ax_set_locator_formatter(axes, dates, typ)
    elif engine == 'bokeh':
        pass
    else:
        print(htr("Unknown engine: {}").format(engine))


# Set the default engine
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
    'use_draw_with_echarts',
    'DRAWNULL', 'STICKLINE', 'DRAWBAND', 'RGB', 'PLOYLINE', 'DRAWLINE', 'DRAWTEXT', 'DRAWNUMBER',
    'DRAWTEXT_FIX', 'DRAWNUMBER_FIX', 'DRAWSL', 'DRAWIMG', 'DRAWICON', 'DRAWBMP', 'SHOWICONS',
    'DRAWRECTREL'
]
