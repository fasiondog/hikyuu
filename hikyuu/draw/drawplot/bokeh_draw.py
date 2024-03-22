# -*- coding: utf8 -*-
# cp936
"""
交互模式下绘制相关图形，如K线图，美式K线图
"""

from hikyuu import *

from bokeh.plotting import figure, ColumnDataSource
from bokeh.models import DatetimeTickFormatter, HoverTool, Title, Label
from bokeh.layouts import column
from bokeh.io import output_notebook, output_file, show


def trans_color(color):
    """将 matplotlib 常用的 color 转换为对应的 bokeh color，如果无法转换则返回原值"""
    color_dict = {'r': 'red', 'g': 'green', 'k': 'black', 'b': 'blue', 'y': 'yellow', 'm': 'mediumorchid'}
    return color_dict[color] if color in color_dict else color


def ax_set_xlim(self, *args, **kwargs):
    pass


def ax_set_ylim(self, *args, **kwargs):
    pass


def ax_fill_between(self, *args, **kwargs):
    pass


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


def create_one_axes_figure(figsize=(800, 450)):
    """生成一个仅含有1个坐标轴的figure，并返回其坐标轴对象

    :param figsize: (宽, 高)
    :return: ax
    """
    global g_figure
    global g_axes
    g_axes = figure(width=figsize[0], height=figsize[1])
    g_figure = column(g_axes)
    return g_axes


def create_two_axes_figure(figsize=(800, 450)):
    """生成一个含有2个坐标轴的figure，并返回坐标轴列表

    :param figsize: (宽, 高)
    :return: (ax1, ax2)    
    """
    global g_figure
    global g_axes
    ax1 = figure(width=figsize[0], height=int(figsize[1] * 0.667))
    ax2 = figure(width=figsize[0], height=int(figsize[1] * 0.333))
    g_figure = column(ax1, ax2)
    g_axes = ax2
    return ax1, ax2


def create_three_axes_figure(figsize=(800, 450)):
    """生成一个含有2个坐标轴的figure，并返回坐标轴列表

    :param figsize: (宽, 高)
    :return: (ax1, ax2)    
    """
    global g_figure
    global g_axes
    ax1 = figure(width=figsize[0], height=int(figsize[1] * 0.5))
    ax2 = figure(width=figsize[0], height=int(figsize[1] * 0.25))
    ax3 = figure(width=figsize[0], height=int(figsize[1] * 0.25))
    g_figure = column(ax1, ax2, ax3)
    g_axes = ax3
    return ax1, ax2, ax3


def create_figure(n=1, figsize=(800, 450)):
    """生成含有指定坐标轴数量的窗口，最大只支持4个坐标轴。

    :param int n: 坐标轴数量
    :param figsize: (宽, 高)
    :return: (ax1, ax2, ...) 根据指定的坐标轴数量而定，超出[1,4]个坐标轴时，返回None
    """
    if n == 1:
        return create_one_axes_figure(figsize)
    elif n == 2:
        return create_two_axes_figure(figsize)
    elif n == 3:
        return create_three_axes_figure(figsize)
    else:
        pass


def get_date_format(kdata):
    return '@datetime{%F}' if kdata.get_query().ktype \
        in (Query.DAY, Query.WEEK, Query.MONTH, Query.QUARTER, Query.HALFYEAR, Query.YEAR) \
        else '@datetime{%F %H:%M:%S}'


def kplot(kdata, new=True, axes=None, colorup='r', colordown='g'):
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
        axes = create_figure() if new or gca() is None else gca()

    k = kdata
    inc_k = [r for r in k if r.close > r.open]
    dec_k = [r for r in k if r.close <= r.open]

    inc_source = ColumnDataSource(
        dict(
            datetime=[r.datetime.datetime() for r in inc_k],
            open=[r.open for r in inc_k],
            high=[r.high for r in inc_k],
            low=[r.low for r in inc_k],
            close=[r.close for r in inc_k],
            amount=[r.amount for r in inc_k],
            volume=[r.volume for r in inc_k]
        )
    )
    dec_source = ColumnDataSource(
        dict(
            datetime=[r.datetime.datetime() for r in dec_k],
            open=[r.open for r in dec_k],
            high=[r.high for r in dec_k],
            low=[r.low for r in dec_k],
            close=[r.close for r in dec_k],
            amount=[r.amount for r in dec_k],
            volume=[r.volume for r in dec_k]
        )
    )

    w = 12 * 60 * 60 * 1000
    colorup = trans_color(colorup)
    colordown = trans_color(colordown)
    axes.segment(x0='datetime', y0='high', x1='datetime', y1='low', color=colorup, source=inc_source)
    axes.segment(x0='datetime', y0='high', x1='datetime', y1='low', color=colordown, source=dec_source)
    axes.vbar(
        x='datetime', width=w, top='close', bottom='open', fill_color="white", line_color=colorup, source=inc_source
    )
    axes.vbar(
        x='datetime', width=w, top='open', bottom='close', fill_color="green", line_color=colordown, source=dec_source
    )
    axes.add_tools(
        HoverTool(
            tooltips=[
                ("index", "$index"), ('日期', get_date_format(k)), ("开盘价", "@open{0.0000}"), ("最高价", "@high{0.0000}"),
                ("最低价", "@low{0.0000}"), ("收盘价", "@close{0.0000}"), ("成交金额", "@amount{0.0000}"),
                ("成交量", "@volume{0.0000}")
            ],
            formatters={"@datetime": "datetime"}
        )
    )

    axes.xaxis[0].formatter = DatetimeTickFormatter()
    axes.title.text = k.get_stock().name
    axes.title.align = "center"
    axes.title.text_font_size = "16px"

    last_record = kdata[-1]
    color = colorup if last_record.close > kdata[-2].close else colordown
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f 涨幅:%.2f%%' % (
        last_record.datetime, last_record.open, last_record.high, last_record.low, last_record.close, 100 *
        (last_record.close - kdata[-2].close) / kdata[-2].close
    )

    label = Label(
        x=axes.width * 0.01,
        y=axes.height * 0.82,
        x_units='screen',
        y_units='screen',
        text=text,
        render_mode='css',
        text_font_size='14px',
        text_color=color,
        background_fill_color='white',
        background_fill_alpha=0.5
    )
    axes.add_layout(label)

    return gcf()


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
    return None


def get_color(index):
    """获取 index 指定的颜色，如果没有固定返回 index=0 的颜色"""
    color_list = ['blue', 'orange', 'green', 'red', 'purple', 'darkgoldenrod', 'pink', 'darkcyan']
    return color_list[index] if index in range(len(color_list)) else color_list[0]


def iplot(
    indicator,
    new=True,
    axes=None,
    kref=None,
    legend_on=False,
    text_on=False,
    text_color='k',
    zero_on=False,
    label=None,
    *args,
    **kwargs
):
    """绘制indicator曲线

    :param Indicator indicator: indicator实例
    :param axes:            指定的坐标轴
    :param new:             是否在新窗口中显示，只在没有指定axes时生效
    :param kref:            参考的K线数据，以便绘制日期X坐标
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
        axes = create_figure() if new or gca() is None else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    line_color = get_color(len(axes.tags)) if 'color' not in kwargs else trans_color(kwargs['color'])

    width = 1
    py_indicator = [None if x == constant.null_price else x for x in indicator]
    if kref:
        x_value = [r.datetime() for r in kref.get_datetime_list()]
        source = ColumnDataSource(dict(datetime=x_value, value=py_indicator))
        if legend_on:
            axes.line(
                x='datetime', y='value', legend_label=label, line_width=width, line_color=line_color, source=source
            )
            axes.legend.location = "top_left"
        else:
            axes.line(x='datetime', y='value', line_width=width, line_color=line_color, source=source)
        axes.add_tools(
            HoverTool(
                tooltips=[
                    ("index", "$index"), ('指标', indicator.name), ('日期', get_date_format(kref)), ("值", "@value{0.0000}")
                ],
                formatters={"@datetime": "datetime"}
            )
        )
        axes.xaxis[0].formatter = DatetimeTickFormatter()
    else:
        x_value = list(range(len(indicator)))
        source = ColumnDataSource(dict(datetime=x_value, value=py_indicator))
        if legend_on:
            axes.line(
                x='datetime', y='value', legend_label=label, line_width=width, line_color=line_color, source=source
            )
            axes.legend.location = "top_left"
        else:
            axes.line(x='datetime', y='value', line_width=width, line_color=line_color, source=source)
        axes.add_tools(HoverTool(tooltips=[("index", "$index"), ('指标', indicator.name), ("值", "@value{0.0000}")]))

    if zero_on:
        axes.line(x=x_value, y=[0 for i in range(len(indicator))], line_color='black')

    if text_on:
        label = Label(
            x=int(axes.plot_width * 0.01),
            y=int(axes.plot_height * 0.88),
            x_units='screen',
            y_units='screen',
            text=label,
            render_mode='css',
            text_font_size='14px',
            text_color=trans_color(text_color),
            background_fill_color='white',
            background_fill_alpha=0.5
        )
        axes.add_layout(label)

    axes.tags.append(line_color)
    return gcf()


def ibar(
    indicator,
    new=True,
    axes=None,
    kref=None,
    legend_on=False,
    text_on=False,
    text_color='k',
    label=None,
    width=0.4,
    color='r',
    edgecolor='r',
    zero_on=False,
    *args,
    **kwargs
):
    """绘制indicator柱状图

    :param Indicator indicator: Indicator实例
    :param axes:       指定的坐标轴
    :param new:        是否在新窗口中显示，只在没有指定axes时生效
    :param kref:       参考的K线数据，以便绘制日期X坐标
    :param legend_on:  是否打开图例
    :param text_on:    是否在左上角显示指标名称及其参数
    :param text_color: 指标名称解释文字的颜色，默认为黑色
    :param str label:  label显示文字信息，text_on 及 legend_on 为 True 时生效
    :param zero_on:    是否需要在y=0轴上绘制一条直线
    :param width:      Bar的宽度
    :param color:      Bar的颜色
    :param edgecolor:  Bar边缘颜色
    :param args:       pylab plot参数
    :param kwargs:     pylab plot参数
    """
    if not indicator:
        print("indicator is None")
        return

    if not axes:
        axes = create_figure() if new or gca() is None else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    line_color = trans_color(color)

    py_indicator = [None if x == constant.null_price else x for x in indicator]
    if kref:
        width = 12 * 60 * 60 * 1000
        x_value = [r.datetime() for r in kref.get_datetime_list()]
        source = ColumnDataSource(dict(datetime=x_value, value=py_indicator))
        if legend_on:
            axes.vbar(x='datetime', top='value', legend_label=label, width=width, color=line_color, source=source)
            axes.legend.location = "top_left"
        else:
            axes.vbar(x='datetime', top='value', width=width, color=line_color, source=source)
        axes.add_tools(
            HoverTool(
                tooltips=[
                    ("index", "$index"), ('指标', indicator.name), ('日期', get_date_format(kref)), ("值", "@value{0.0000}")
                ],
                formatters={"@datetime": "datetime"}
            )
        )
        axes.xaxis[0].formatter = DatetimeTickFormatter()
    else:
        width = 0.5
        x_value = list(range(len(indicator)))
        source = ColumnDataSource(dict(datetime=x_value, value=py_indicator))
        if legend_on:
            axes.vbar(x='datetime', top='value', legend_label=label, width=width, color=line_color, source=source)
            axes.legend.location = "top_left"
        else:
            axes.vbar(x='datetime', top='value', width=width, color=line_color, source=source)
        axes.add_tools(HoverTool(tooltips=[("index", "$index"), ('指标', indicator.name), ("值", "@value{0.0000}")]))

    if text_on:
        label = Label(
            x=int(axes.plot_width * 0.01),
            y=int(axes.plot_height * 0.88),
            x_units='screen',
            y_units='screen',
            text=label,
            render_mode='css',
            text_font_size='14px',
            text_color=trans_color(text_color),
            background_fill_color='white',
            background_fill_alpha=0.5
        )
        axes.add_layout(label)

    axes.tags.append(line_color)
    return gcf()


def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """绘制MACD

    :param axes: 指定的坐标轴
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
    """
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.get_result(0), macd.get_result(1), macd.get_result(2)

    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f' % (n1, n2, n3, fmacd[-1], smacd[-1], bmacd[-1])
    label = Label(
        x=int(axes.plot_width * 0.01),
        y=int(axes.plot_height * 0.88),
        x_units='screen',
        y_units='screen',
        text=text,
        render_mode='css',
        text_font_size='14px',
        background_fill_color='white',
        background_fill_alpha=0.5
    )
    axes.add_layout(label)

    bmacd.bar(axes=axes, kref=kdata, color='red')

    fmacd.plot(axes=axes, legend_on=False, text_on=False, kref=kdata)
    smacd.plot(axes=axes, legend_on=False, text_on=False, kref=kdata)

    return gcf()


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
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.get_result(0), macd.get_result(1), macd.get_result(2)

    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f' % (n1, n2, n3, fmacd[-1], smacd[-1], bmacd[-1])
    label = Label(
        x=int(axes.plot_width * 0.01),
        y=int(axes.plot_height * 0.88),
        x_units='screen',
        y_units='screen',
        text=text,
        render_mode='css',
        text_font_size='14px',
        background_fill_color='white',
        background_fill_alpha=0.5
    )
    axes.add_layout(label)

    pre_ref = REF(ref, 1)
    pre_bmacd = REF(bmacd, 1)
    x1 = IF((ref < pre_ref) & (bmacd < pre_bmacd), bmacd, 0)
    x2 = IF((ref > pre_ref) & (bmacd > pre_bmacd), bmacd, 0)
    x3 = IF(NOT((ref < pre_ref) & (bmacd < pre_bmacd)) & NOT((ref > pre_ref) & (bmacd > pre_bmacd)), bmacd, 0)

    x1.bar(axes=axes, kref=kdata, color='#BFBFBF')
    x2.bar(axes=axes, kref=kdata, color='red')
    x3.bar(axes=axes, kref=kdata, color='green')

    fmacd.plot(axes=axes, legend_on=False, text_on=False, kref=kdata)
    smacd.plot(axes=axes, legend_on=False, text_on=False, kref=kdata)

    return gcf()


def sgplot(sg, new=True, axes=None, style=1, kdata=None):
    """绘制买入/卖出信号

    :param SignalBase sg: 信号指示器
    :param new: 仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式
    :param KData kdata: 指定的KData（即信号发生器的交易对象），
                       如该值为None，则认为该信号发生器已经指定了交易对象，
                       否则，使用该参数作为交易对象
    """
    kdata = sg.to if kdata is None else kdata
    date_index = dict([(d, i) for i, d in enumerate(kdata.get_datetime_list())])

    if axes is None:
        if new:
            axes = create_figure()
            kplot(kdata, axes=axes)
        else:
            axes = gca()

    height = max(kdata.high) - min(kdata.low)

    buy_dates = sg.get_buy_signal()
    buy_y = []
    for d in buy_dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        x = krecord.datetime.datetime().timestamp() * 1000
        if pos > 0:
            x = x - (krecord.datetime - kdata[pos - 1].datetime).ticks * 0.001 / 2
        buy_y.append(krecord.low - height * 0.05)
        label = Label(
            x=x,
            y=krecord.low - height * 0.1,
            text='B',
            text_font_size='14px',
            text_color='red',
        )
        axes.add_layout(label)
    axes.triangle(x=[d.datetime() for d in buy_dates], y=buy_y, fill_color='red', line_color='red', size=10)

    sell_dates = sg.get_sell_signal()
    sell_y = []
    for d in sell_dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        x = krecord.datetime.datetime().timestamp() * 1000
        if pos > 0:
            x = x - (krecord.datetime - kdata[pos - 1].datetime).ticks * 0.001 / 2
        sell_y.append(krecord.high + height * 0.05)
        label = Label(
            x=x,
            y=krecord.high + height * 0.08,
            text='S',
            text_font_size='14px',
            text_color='blue',
        )
        axes.add_layout(label)
    axes.inverted_triangle(
        x=[d.datetime() for d in sell_dates], y=sell_y, fill_color='blue', line_color='blue', size=10
    )

    return gcf()
