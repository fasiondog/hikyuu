# -*- coding: utf8 -*-
# cp936
"""
交互模式下绘制相关图形，如K线图，美式K线图
"""

from hikyuu import *
from .common import get_draw_title

from bokeh.plotting import figure,ColumnDataSource
from bokeh.models import DatetimeTickFormatter,HoverTool, Title, Label
from bokeh.layouts import column
from bokeh.io import output_notebook, show

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


def create_figure(n=1, figsize=(800, 450)):
    """生成含有指定坐标轴数量的窗口，最大只支持4个坐标轴。

    :param int n: 坐标轴数量
    :param figsize: (宽, 高)
    :return: (ax1, ax2, ...) 根据指定的坐标轴数量而定，超出[1,4]个坐标轴时，返回None
    """
    if n == 1:
        return create_one_axes_figure(figsize)
    else:
        pass


def kplot(kdata, new=True, axes=None, colorup='red', colordown='green', width=0.6, alpha=1.0):
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
    inc_k = [r for r in k if r.close >  r.open]
    dec_k = [r for r in k if r.close <= r.open]
    
    inc_source = ColumnDataSource(dict(datetime=[r.datetime.datetime() for r in inc_k],
                                      open=[r.open for r in inc_k],
                                      high=[r.high for r in inc_k],
                                      low=[r.low for r in inc_k],
                                      close=[r.close for r in inc_k],
                                      amount=[r.amount for r in inc_k],
                                      volume=[r.volume for r in inc_k]))
    dec_source = ColumnDataSource(dict(datetime=[r.datetime.datetime() for r in dec_k],
                                      open=[r.open for r in dec_k],
                                      high=[r.high for r in dec_k],
                                      low=[r.low for r in dec_k],
                                      close=[r.close for r in dec_k],
                                      amount=[r.amount for r in dec_k],
                                      volume=[r.volume for r in dec_k]))
    
    w = 12*60*60*1000
    axes.segment(x0='datetime', y0='high', x1='datetime', y1='low', color='red', source=inc_source)
    axes.segment(x0='datetime', y0='high', x1='datetime', y1='low', color='green', source=dec_source)
    axes.vbar(x='datetime', width=w, top='close', bottom='open', fill_color="white", 
              line_color="red", source=inc_source)
    axes.vbar(x='datetime', width=w, top='open', bottom='close', fill_color="green", 
              line_color="green", source=dec_source)
    date_format = '@datetime{%F}' if k.get_query().ktype \
        in (Query.DAY, Query.WEEK, Query.MONTH, Query.QUARTER, Query.HALFYEAR, Query.YEAR) \
        else '@datetime{%F %H:%M:%S}'
    axes.add_tools(HoverTool(tooltips=[("index", "$index"), ('日期', date_format), 
                                       ("开盘价", "@open{0.0000}"), ("最高价", "@high{0.0000}"), 
                                       ("最低价", "@low{0.0000}"),("收盘价", "@close{0.0000}"),
                                      ("成交金额", "@amount{0.0000}"), ("成交量", "@volume{0.0000}")],
                     formatters = { "datetime": "datetime"}))

    axes.xaxis[0].formatter = DatetimeTickFormatter()
    axes.title.text = k.get_stock().name
    axes.title.align = "center"
    axes.title.text_font_size = "16px"

    last_record = kdata[-1]
    color = 'red' if last_record.close > kdata[-2].close else 'green'
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f 涨幅:%.2f%%' % (
        last_record.datetime, last_record.open, last_record.high, last_record.low,
        last_record.close, 100 * (last_record.close - kdata[-2].close) / kdata[-2].close
    )
    
    label = Label(
        x=20,
        y=axes.plot_height - 90,
        x_units='screen', y_units='screen',
        text=text,
        render_mode='css',
        text_font_size='14px',
        text_color=color,
        background_fill_color='white', 
        background_fill_alpha=0.5
    )
    axes.add_layout(label)

    show(axes)
