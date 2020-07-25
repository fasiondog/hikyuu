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
from bokeh.io import output_notebook, output_file, show

g_use_in_notbook = False
g_figure = None
g_axes = None
g_use_in_notbook = False

def use_bokeh_in_notebook(in_notebook=False):
    global g_use_in_notbook
    g_use_in_notbook = in_notebook
    if g_use_in_notbook:
        output_notebook()


def gcf():
    """获取当前Axis"""
    global g_figure
    return g_figure


def gca():
    """获取当前figure"""
    global g_axes
    return g_axes


def show_gcf():
    global g_use_in_notbook
    if not g_use_in_notbook:
        output_file("{}/bokeh.html".format(StockManager.instance().tmpdir()))
    show(gcf())


def create_one_axes_figure(figsize=(800,450)):
    """生成一个仅含有1个坐标轴的figure，并返回其坐标轴对象
    
    :param figsize: (宽, 高)
    :return: ax
    """
    global g_figure
    global g_axes
    g_axes = figure(width=figsize[0], height=figsize[1])
    g_figure = column(g_axes)
    return g_axes


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


def get_date_format(kdata):
    return '@datetime{%F}' if kdata.get_query().ktype \
        in (Query.DAY, Query.WEEK, Query.MONTH, Query.QUARTER, Query.HALFYEAR, Query.YEAR) \
        else '@datetime{%F %H:%M:%S}'


def kplot(kdata, new=True, axes=None, colorup='red', colordown='green'):
    """绘制K线图
    
    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
    :param colorup:     the color of the rectangle where close >= open
    :param colordown:   the color of the rectangle where close < open
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
    axes.segment(x0='datetime', y0='high', x1='datetime', y1='low', color=colorup, source=inc_source)
    axes.segment(x0='datetime', y0='high', x1='datetime', y1='low', color=colordown, source=dec_source)
    axes.vbar(x='datetime', width=w, top='close', bottom='open', fill_color="white", 
              line_color=colorup, source=inc_source)
    axes.vbar(x='datetime', width=w, top='open', bottom='close', fill_color="green", 
              line_color=colordown, source=dec_source)
    axes.add_tools(HoverTool(tooltips=[("index", "$index"), ('日期', get_date_format(k)), 
                                       ("开盘价", "@open{0.0000}"), ("最高价", "@high{0.0000}"), 
                                       ("最低价", "@low{0.0000}"),("收盘价", "@close{0.0000}"),
                                      ("成交金额", "@amount{0.0000}"), ("成交量", "@volume{0.0000}")],
                     formatters = { "datetime": "datetime"}))

    axes.xaxis[0].formatter = DatetimeTickFormatter()
    axes.title.text = k.get_stock().name
    axes.title.align = "center"
    axes.title.text_font_size = "16px"

    last_record = kdata[-1]
    color = colorup if last_record.close > kdata[-2].close else colordown
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

    show_gcf()


def mkplot(kdata, new=True, axes=None, colorup='r', colordown='g', ticksize=3):
    """绘制美式K线图
    
    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
    :param colorup:     the color of the lines where close >= open
    :param colordown:   the color of the lines where close < open
    :param ticksize:    open/close tick marker in points
    """
    print("Bokeh 暂不支持绘制美式K线图, 请使用 matplotlib")


def iplot(
    indicator,
    new=True,
    axes=None,
    legend_on=False,
    text_on=False,
    text_color='black',
    zero_on=False,
    label=None,
    *args,
    **kwargs
):
    """绘制indicator曲线
    
    :param Indicator indicator: indicator实例
    :param axes:            指定的坐标轴
    :param new:             是否在新窗口中显示，只在没有指定axes时生效
    :param legend_on:       是否打开图例
    :param text_on:         是否在左上角显示指标名称及其参数
    :param text_color:      指标名称解释文字的颜色，默认为黑色
    :param zero_on:         是否需要在y=0轴上绘制一条直线
    :param str label:       label显示文字信息，text_on 及 legend_on 为 True 时生效
    :param args:            pylab plot参数
    :param kwargs:          pylab plot参数，如：marker（标记类型）、
                             markerfacecolor（标记颜色）、
                             markeredgecolor（标记的边缘颜色）
    """
    if not indicator:
        print("indicator is None")
        return

    if not axes:
        axes = create_figure() if new else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    py_indicator = [None if x == constant.null_price else x for x in indicator]
    k = indicator.get_context()
    if len(k) > 0:
        source = ColumnDataSource(dict(
            datetime=[r.datetime() for r in k.get_datetime_list()], 
            value=py_indicator))
        if legend_on:
            axes.line(x='datetime', y='value', legend_label=label, line_width=2, source=source)
        else:
            axes.line(x='datetime', y='value', line_width=2, source=source)
        axes.add_tools(HoverTool(tooltips=[("index", "$index"), ('指标', indicator.name),
                                 ('日期', get_date_format(k)), ("值", "@value{0.0000}")],
                     formatters = { "datetime": "datetime"}))
        axes.xaxis[0].formatter = DatetimeTickFormatter()
    else:
        source = ColumnDataSource(dict(datetime=list(range(len(k))), value=py_indicator))
        if legend_on:
            axes.line(x='datetime', y='value', legend_label=label, line_width=2, source=source)
        else:
            axes.line(x='datetime', y='value', line_width=2, source=source)
        axes.add_tools(HoverTool(tooltips=[("index", "$index"), ('指标', indicator.name),
                                 ("值", "@value{0.0000}")]))

    show_gcf()