# -*- coding: utf8 -*-
# cp936
"""
Draw the related charts in the interactive mode, such as the K-line chart and the American K-line chart
"""

from hikyuu import *
from hikyuu import htr

from bokeh.plotting import figure, ColumnDataSource
from bokeh.models import DatetimeTickFormatter, HoverTool, Title, Label
from bokeh.layouts import column
from bokeh.io import output_notebook, output_file, show


def trans_color(color):
    """Convert the common matplotlib color to the corresponding bokeh color; return the original value if it fails"""
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
    """Get the current Axis"""
    global g_figure
    return g_figure


def gca():
    """Get the current figure"""
    global g_axes
    return g_axes


def show_gcf():
    global g_use_in_notbook
    if not g_use_in_notbook:
        output_file("{}/bokeh.html".format(StockManager.instance().tmpdir()))
    show(gcf())


def create_one_axes_figure(figsize=(800, 450)):
    """Generate a figure containing only 1 axes, and return its axes object

    :param figsize: (width, height)
    :return: ax
    """
    global g_figure
    global g_axes
    g_axes = figure(width=figsize[0], height=figsize[1])
    g_figure = column(g_axes)
    return g_axes


def create_two_axes_figure(figsize=(800, 450)):
    """Generate a figure containing 2 axes, and return the list of the axes

    :param figsize: (width, height)
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
    """Generate a figure containing 3 axes, and return the list of the axes

    :param figsize: (width, height)
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
    """Generate a window containing the specified number of axes; at most 4 axes are supported.

    :param int n: the number of the axes
    :param figsize: (width, height)
    :return: (ax1, ax2, ...) depending on the specified number of the axes; None is returned
        when the number is out of [1, 4]
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
    """Draw the K-line chart

    :param KData kdata: the K-line data
    :param bool new:    whether to display in a new window; it takes effect only when axes is not specified
    :param axes:        the specified axes
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
                ("index", "$index"), (htr('Date'), get_date_format(k)), (htr('Open'), "@open{0.0000}"),
                (htr('High'), "@high{0.0000}"), (htr('Low'), "@low{0.0000}"), (htr('Close'), "@close{0.0000}"),
                (htr('Amount'), "@amount{0.0000}"), (htr('Volume'), "@volume{0.0000}")
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
    text = htr('{} Open:{:.2f} High:{:.2f} Low:{:.2f} Close:{:.2f} Change:{:.2f}%').format(
        last_record.datetime, last_record.open, last_record.high, last_record.low, last_record.close,
        100 * (last_record.close - kdata[-2].close) / kdata[-2].close)

    label = Label(
        x=axes.width * 0.01,
        y=axes.height * 0.82,
        x_units='screen',
        y_units='screen',
        text=text,
        text_font_size='14px',
        text_color=color,
        background_fill_color='white',
        background_fill_alpha=0.5
    )
    axes.add_layout(label)

    return gcf()


def mkplot(kdata, new=True, axes=None, colorup='r', colordown='g', ticksize=3):
    """Draw the American K-line chart

    :param KData kdata: the K-line data
    :param bool new:    whether to display in a new window; it takes effect only when axes is not specified
    :param axes:        the specified axes
    :param colorup:     the color of the lines where close >= open
    :param colordown:   the color of the lines where close < open
    :param ticksize:    open/close tick marker in points
    """
    print(htr("Bokeh does not support the American K-line chart yet, please use matplotlib"))
    return None


def get_color(index):
    """Get the color specified by index; return the color of index=0 if it does not exist"""
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
    """Draw the indicator curve

    :param Indicator indicator: the Indicator instance
    :param axes:            the specified axes
    :param new:             whether to display in a new window; it takes effect only when axes is not specified
    :param kref:            the referenced K-line data, used to draw the date X coordinate
    :param legend_on:       whether to turn on the legend
    :param text_on:         whether to display the indicator name and its parameters in the upper left corner
    :param text_color:      the color of the indicator name text, black by default
    :param zero_on:         whether to draw a straight line on the y=0 axis
    :param str label:       the text displayed by the label; it takes effect when text_on and legend_on are True
    :param args:            the pylab plot arguments
    :param kwargs:          the pylab plot arguments, such as marker (the marker type),
                             markerfacecolor (the marker color) and
                             markeredgecolor (the marker edge color)
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
                    ("index", "$index"), (htr('Indicator'), indicator.name), (htr('Date'), get_date_format(kref)),
                    (htr('Value'), "@value{0.0000}")
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
        axes.add_tools(
            HoverTool(tooltips=[("index", "$index"),
                                (htr('Indicator'), indicator.name),
                                (htr('Value'), "@value{0.0000}")]))

    if zero_on:
        axes.line(x=x_value, y=[0 for i in range(len(indicator))], line_color='black')

    if text_on:
        label = Label(
            x=int(axes.width * 0.01),
            y=int(axes.height * 0.88),
            x_units='screen',
            y_units='screen',
            text=label,
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
    """Draw the indicator bar chart

    :param Indicator indicator: the Indicator instance
    :param axes:       the specified axes
    :param new:        whether to display in a new window; it takes effect only when axes is not specified
    :param kref:       the referenced K-line data, used to draw the date X coordinate
    :param legend_on:  whether to turn on the legend
    :param text_on:    whether to display the indicator name and its parameters in the upper left corner
    :param text_color: the color of the indicator name text, black by default
    :param str label:  the text displayed by the label; it takes effect when text_on and legend_on are True
    :param zero_on:    whether to draw a straight line on the y=0 axis
    :param width:      the width of the bar
    :param color:      the color of the bar
    :param edgecolor:  the edge color of the bar
    :param args:       the pylab plot arguments
    :param kwargs:     the pylab plot arguments
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
                    ("index", "$index"), (htr('Indicator'), indicator.name), (htr('Date'), get_date_format(kref)),
                    (htr('Value'), "@value{0.0000}")
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
        axes.add_tools(
            HoverTool(tooltips=[("index", "$index"),
                                (htr('Indicator'), indicator.name),
                                (htr('Value'), "@value{0.0000}")]))

    if text_on:
        label = Label(
            x=int(axes.width * 0.01),
            y=int(axes.height * 0.88),
            x_units='screen',
            y_units='screen',
            text=label,
            text_font_size='14px',
            text_color=trans_color(text_color),
            background_fill_color='white',
            background_fill_alpha=0.5
        )
        axes.add_layout(label)

    axes.tags.append(line_color)
    return gcf()


def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """Draw MACD

    :param axes: the specified axes
    :param KData kdata: KData
    :param int n1: the parameter 1 of the MACD indicator
    :param int n2: the parameter 2 of the MACD indicator
    :param int n3: the parameter 3 of the MACD indicator
    """
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.get_result(0), macd.get_result(1), macd.get_result(2)

    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f' % (n1, n2, n3, fmacd[-1], smacd[-1], bmacd[-1])
    label = Label(
        x=int(axes.width * 0.01),
        y=int(axes.height * 0.88),
        x_units='screen',
        y_units='screen',
        text=text,
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
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.get_result(0), macd.get_result(1), macd.get_result(2)

    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f' % (n1, n2, n3, fmacd[-1], smacd[-1], bmacd[-1])
    label = Label(
        x=int(axes.width * 0.01),
        y=int(axes.height * 0.88),
        x_units='screen',
        y_units='screen',
        text=text,
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
    """Draw the buy/sell signals

    :param SignalBase sg: the signal generator
    :param new: it takes effect only when axes is not specified; when True, a new window object is
        created and the drawing is done in it
    :param axes: the axes object in which to draw
    :param style: 1 | 2, the drawing style of the signal arrows
    :param KData kdata: the specified KData (i.e. the trading object of the signal generator);
        when it is None, the signal generator is assumed to have its trading object specified,
        otherwise this argument is used as the trading object
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
