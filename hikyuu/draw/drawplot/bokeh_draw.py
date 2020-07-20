# -*- coding: utf8 -*-
# cp936
"""
交互模式下绘制相关图形，如K线图，美式K线图
"""

from hikyuu import *
from .common import get_draw_title

from bokeh.plotting import figure,ColumnDataSource
from bokeh.models import DatetimeTickFormatter
from bokeh.io import show

global g_figure

def gcf():
    global g_figure
    return g_figure


gca = gcf


def create_one_axes_figure(figsize=(800,450)):
    """生成一个仅含有1个坐标轴的figure，并返回其坐标轴对象
    
    :param figsize: (宽, 高)
    :return: ax
    """
    global g_figure
    g_figure = figure(width=figsize[0], height=figsize[1])
    return g_figure


def create_figure(n=1, figsize=(10, 8)):
    """生成含有指定坐标轴数量的窗口，最大只支持4个坐标轴。

    :param int n: 坐标轴数量
    :param figsize: (宽, 高)
    :return: (ax1, ax2, ...) 根据指定的坐标轴数量而定，超出[1,4]个坐标轴时，返回None
    """
    if n == 1:
        return create_one_axes_figure(figsize)
    else:
        pass


def kplot(kdata, new=True, axes=None, colorup='r', colordown='g', width=0.6, alpha=1.0):
    """绘制K线图
    
    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
    :param colorup:     the color of the rectangle where close >= open
    :param colordown:   the color of the rectangle where close < open
    :param width:       fraction of a day for the rectangle width
    :param alpha:       the rectangle alpha level, 透明度(0.0~1.0) 1.0为不透明
    """
    if not kdata:
        print("kdata is None")
        return

    if not axes:
        axes = create_figure() if new else gca()

    k = kdata
    up_dates = [r.datetime.datetime() for r in k if r.close > r.open]
    up_high = [r.high for r in k if r.close > r.open]
    up_low = [r.low for r in k if r.close > r.open]
    up_open = [r.open for r in k if r.close > r.open]
    up_close = [r.close for r in k if r.close > r.open]
    down_dates = [r.datetime.datetime() for r in k if r.close <= r.open]
    down_high = [r.high for r in k if r.close <= r.open]
    down_low = [r.low for r in k if r.close <= r.open]
    down_open = [r.open for r in k if r.close <= r.open]
    down_low = [r.close for r in k if r.close <= r.open]

    w = 12*60*60*1000
    axes.segment(up_dates, up_high, up_dates, up_low, color='red')
    axes.segment(down_dates, down_high, down_dates, down_low, color='green')
    axes.vbar(up_dates, w, up_open, up_close, fill_color="white", line_color="red")
    axes.vbar(down_dates, w, down_open, down_low, fill_color="green", line_color="green")
    axes.xaxis[0].formatter = DatetimeTickFormatter()
    show(axes)
