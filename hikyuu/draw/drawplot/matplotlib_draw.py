# -*- coding: utf8 -*-
# cp936
"""
Draw the related charts in the interactive mode, such as the K-line chart and the American K-line chart
"""
import sys
import os
import logging
import numpy as np
import matplotlib
import seaborn as sns
import math
from typing import Union
from matplotlib.pylab import Rectangle, gca, gcf, figure, ylabel, axes, draw
from matplotlib import rcParams
from matplotlib.font_manager import FontManager, _log as fm_logger
from matplotlib.lines import Line2D, TICKLEFT, TICKRIGHT
from matplotlib.ticker import FuncFormatter, FixedLocator
from matplotlib.image import imread

from hikyuu import *
from hikyuu import constant, isnan, Indicator, KData, IF, ALIGN, htr

from .common import get_draw_title


ICON_PATH = os.path.dirname(os.path.abspath(__file__))


def set_mpl_params():
    '''Set the interactive and the language environment parameters'''
    if in_interactive_session():
        rcParams['interactive'] = True
    else:
        rcParams['interactive'] = False

    if sys.platform == 'darwin':
        matplotlib.rcParams['font.sans-serif'] = 'Arial Unicode MS'
        return

    rcParams['font.family'] = 'sans-serif'
    rcParams['axes.unicode_minus'] = False

    expected_fonts = ['Microsoft YaHei', 'SimSun', 'SimHei',
                      'Source Han Sans CN', 'Noto Sans CJK JP', 'Arial Unicode MS']
    current_fonts = matplotlib.rcParams['font.sans-serif']
    for font in expected_fonts:
        if font in current_fonts:
            return

    with LoggingContext(fm_logger, level=logging.WARNING):
        all_fonts = [f.name for f in FontManager().ttflist]
    for font in expected_fonts:
        if font in all_fonts:
            current_fonts.insert(0, font)
            break
    matplotlib.rcParams['font.sans-serif'] = current_fonts


def create_one_axes_figure(figsize=(10, 6)):
    """Generate a figure containing only 1 axes, and return its axes object

    :param figsize: (width, height)
    :return: ax
    """
    rect1 = [0.05, 0.05, 0.9, 0.90]
    fg = figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    return ax1


def create_two_axes_figure(figsize=(10, 8)):
    """Generate a figure containing 2 axes, and return the list of the axes

    :param figsize: (width, height)
    :return: (ax1, ax2)
    """
    rect1 = [0.05, 0.35, 0.9, 0.60]
    rect2 = [0.05, 0.05, 0.9, 0.30]

    fg = figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    ax2 = fg.add_axes(rect2, sharex=ax1)

    return ax1, ax2


def create_three_axes_figure(figsize=(10, 8)):
    """Generate a figure containing 3 axes, and return the list of the axes

    :param figsize: (width, height)
    :return: (ax1, ax2, ax3)
    """
    rect1 = [0.05, 0.45, 0.9, 0.50]
    rect2 = [0.05, 0.25, 0.9, 0.20]
    rect3 = [0.05, 0.05, 0.9, 0.20]

    fg = figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    ax2 = fg.add_axes(rect2, sharex=ax1)
    ax3 = fg.add_axes(rect3, sharex=ax1)

    return ax1, ax2, ax3


def create_four_axes_figure(figsize=(10, 8)):
    """Generate a figure containing 4 axes, and return the list of the axes

    :param figsize: (width, height)
    :return: (ax1, ax2, ax3, ax4)
    """
    rect1 = [0.05, 0.50, 0.9, 0.45]
    rect2 = [0.05, 0.35, 0.9, 0.15]
    rect3 = [0.05, 0.20, 0.9, 0.15]
    rect4 = [0.05, 0.05, 0.9, 0.15]

    fg = figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    ax2 = fg.add_axes(rect2, sharex=ax1)
    ax3 = fg.add_axes(rect3, sharex=ax1)
    ax4 = fg.add_axes(rect4, sharex=ax1)

    return ax1, ax2, ax3, ax4


def create_figure(n=1, figsize=(10, 8)):
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
    elif n == 4:
        return create_four_axes_figure(figsize)
    else:
        print("Max support axes number is 4!")
        return None


class StockFuncFormatter(object):
    """Display the date on the axes
    For the usage of FuncFormatter in matplotlib, see:
    http://matplotlib.sourceforge.net/examples/api/date_index_formatter.html
    """

    def __init__(self, ix2date):
        self.__ix2date = ix2date

    def __call__(self, x, pos=None):  # IGNORE:W0613
        result = ''
        ix = int(x)
        if ix in self.__ix2date:
            result = self.__ix2date[ix]
        return result


def getDayLocatorAndFormatter(dates):
    """Get the major locator and the major formatter used when displaying the daily line"""
    sep = int(len(dates) / 10)
    loc = [
        (i, str(d) if (i != (len(dates) - 1)) and (i % sep != 0) else "{}-{}-{}".format(d.year, d.month, d.day))
        for i, d in enumerate(dates)
    ]
    fixed_loc = [i for i in range(len(dates)) if (i == (len(dates) - 1)) or (i != 0 and i % sep == 0)]

    month_loc = FixedLocator(fixed_loc)
    month_fm = FuncFormatter(StockFuncFormatter(dict(loc)))
    return month_loc, month_fm


def getMinLocatorAndFormatter(dates):
    """Get the major locator and the major formatter used when displaying the minute line"""
    sep = len(dates) / 5
    loc = [
        (i, str(d) if i % sep != 0 else "{}-{}-{} {}:{}".format(d.year, d.month, d.day, d.hour, d.minute))
        for i, d in enumerate(dates)
    ]
    fixed_loc = [i for i in range(len(dates)) if i != 0 and i % sep == 0]

    month_loc = FixedLocator(fixed_loc)
    month_fm = FuncFormatter(StockFuncFormatter(dict(loc)))
    return month_loc, month_fm


def ax_set_locator_formatter(axes, dates, typ):
    """Set the date display of the specified axes, and optimize the X-axis display according to
    the specified K-line type

    :param axes: the specified axes
    :param dates: an iterable sequence of Datetime
    :param Query.KType typ: the K-line type
    """
    major_loc, major_fm = None, None
    if typ == Query.DAY:
        major_loc, major_fm = getDayLocatorAndFormatter(dates)
    elif typ == Query.WEEK:
        major_loc, major_fm = getDayLocatorAndFormatter(dates)
    elif typ == Query.MONTH:
        major_loc, major_fm = getDayLocatorAndFormatter(dates)
    elif typ == Query.QUARTER:
        major_loc, major_fm = getDayLocatorAndFormatter(dates)
    elif typ == Query.HALFYEAR:
        major_loc, major_fm = getDayLocatorAndFormatter(dates)
    elif typ == Query.YEAR:
        major_loc, major_fm = getDayLocatorAndFormatter(dates)
    else:
        major_loc, major_fm = getMinLocatorAndFormatter(dates)

    axes.xaxis.set_major_locator(major_loc)
    axes.xaxis.set_major_formatter(major_fm)


def adjust_axes_show(axeslist):
    """Adjust the display of the axes that are closely connected up and down, so that the
    minimum tick of the upper axis does not overlap the maximum tick of the lower one.

    :param axeslist: the list of the axes connected up and down (ax1, ax2, ...)
    """
    for ax in axeslist[:-1]:
        for label in ax.get_xticklabels():
            label.set_visible(False)
        ylabels = ax.get_yticklabels()
        ylabels[0].set_visible(False)


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
        axes = create_figure() if new else gca()

    alpha = 1.0
    width = 0.6
    OFFSET = width / 2.0
    rfcolor = matplotlib.rcParams['axes.facecolor']
    for i in range(len(kdata)):
        record = kdata[i]
        open, high, low, close = record.open, record.high, record.low, record.close
        if close >= open:
            color = colorup
            lower = open
            height = close - open
            rect = Rectangle(xy=(i - OFFSET, lower), width=width, height=height, facecolor=rfcolor, edgecolor=color)
        else:
            color = colordown
            lower = close
            height = open - close
            rect = Rectangle(xy=(i - OFFSET, lower), width=width, height=height, facecolor=color, edgecolor=color)

        vline1 = Line2D(xdata=(i, i), ydata=(low, lower), color=color, linewidth=0.5, antialiased=True)
        vline2 = Line2D(xdata=(i, i), ydata=(lower + height, high), color=color, linewidth=0.5, antialiased=True)
        rect.set_alpha(alpha)

        axes.add_line(vline1)
        axes.add_line(vline2)
        axes.add_patch(rect)

    title = get_draw_title(kdata)
    axes.set_title(title)
    last_record = kdata[-1]
    color = 'r' if last_record.close > kdata[-2].close else 'g'
    text = htr('{} Open:{:.2f} High:{:.2f} Low:{:.2f} Close:{:.2f} Change:{:.2f}%').format(
        last_record.datetime.number / 10000, last_record.open, last_record.high, last_record.low, last_record.close,
        100 * (last_record.close - kdata[-2].close) / kdata[-2].close)
    axes.text(
        0.99, 0.97, text, horizontalalignment='right', verticalalignment='top', transform=axes.transAxes, color=color
    )

    axes.autoscale_view()
    axes.set_xlim(-1, len(kdata) + 1)
    ax_set_locator_formatter(axes, kdata.get_datetime_list(), kdata.get_query().ktype)
    # draw()


def mkplot(kdata, new=True, axes=None, colorup='r', colordown='g', ticksize=3):
    """Draw the American K-line chart

    :param KData kdata: the K-line data
    :param bool new:    whether to display in a new window; it takes effect only when axes is not specified
    :param axes:        the specified axes
    :param colorup:     the color of the lines where close >= open
    :param colordown:   the color of the lines where close < open
    :param ticksize:    open/close tick marker in points
    """
    if not kdata:
        print("kdata is None")
        return

    if not axes:
        axes = create_figure() if new else gca()

    for t in range(len(kdata)):
        record = kdata[t]
        open, high, low, close = record.open, record.high, record.low, record.close
        color = colorup if close >= open else colordown

        vline = Line2D(xdata=(t, t), ydata=(low, high), color=color, antialiased=False)
        oline = Line2D(
            xdata=(t, t), ydata=(open, open), color=color, antialiased=False, marker=TICKLEFT, markersize=ticksize
        )
        cline = Line2D(
            xdata=(t, t), ydata=(close, close), color=color, antialiased=False, markersize=ticksize, marker=TICKRIGHT
        )

        axes.add_line(vline)
        axes.add_line(oline)
        axes.add_line(cline)

    title = get_draw_title(kdata)
    axes.set_title(title)
    last_record = kdata[-1]
    color = 'r' if last_record.close > kdata[-2].close else 'g'
    text = htr('{} Open:{:.2f} High:{:.2f} Low:{:.2f} Close:{:.2f}').format(
        last_record.datetime.number / 10000, last_record.open, last_record.high, last_record.low, last_record.close)
    axes.text(
        0.99, 0.97, text, horizontalalignment='right', verticalalignment='top', transform=axes.transAxes, color=color
    )

    axes.autoscale_view()
    axes.set_xlim(-1, len(kdata) + 1)
    ax_set_locator_formatter(axes, kdata.get_datetime_list(), kdata.get_query().ktype)
    # draw()


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
    linestyle='-',
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
        axes = create_figure() if new else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    py_ind = ALIGN(indicator, kref) if kref is not None else indicator

    # Fix the problem that the chart cannot be drawn when there are null values, by stone 20251217
    # py_indicatr = [None if x == constant.null_price else x for x in indicator]
    # axes.plot(py_indicatr, linestyle=linestyle, label=label, *args, **kwargs)

    py_indicatr = np.array([None if x == constant.null_price else x for x in py_ind])
    py_x = np.arange(len(py_indicatr))
    imask = np.isfinite(py_indicatr)
    axes.plot(py_x[imask], py_indicatr[imask], linestyle=linestyle, label=label, *args, **kwargs)

    if legend_on:
        leg = axes.legend(loc='upper left')
        leg.get_frame().set_alpha(0.5)

    if text_on:
        if not axes.texts:
            axes.text(
                0.01,
                0.97,
                label,
                horizontalalignment='left',
                verticalalignment='top',
                transform=axes.transAxes,
                color=text_color
            )
        else:
            temp_str = axes.texts[0].get_text() + '  ' + label
            axes.texts[0].set_text(temp_str)

    if zero_on:
        ylim = axes.get_ylim()
        if ylim[0] < 0 < ylim[1]:
            axes.hlines(0, 0, len(indicator))

    axes.autoscale_view()
    if kref is not None:
        ax_set_locator_formatter(axes, kref.get_datetime_list(), kref.get_query().ktype)
        axes.set_xlim(-1, len(kref) + 1)
    else:
        k = indicator.get_context()
        if len(k) > 0:
            ax_set_locator_formatter(axes, k.get_datetime_list(), k.get_query().ktype)
            axes.set_xlim(-1, len(k) + 1)
        else:
            axes.set_xlim(-1, len(indicator) + 1)


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
        axes = create_figure() if new else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    py_ind = ALIGN(indicator, kref) if kref is not None else indicator

    py_indicatr = [None if x == constant.null_price else x for x in py_ind]
    x = [i - 0.2 for i in range(len(py_ind))]
    y = py_indicatr

    axes.bar(x, py_indicatr, width=width, color=color, edgecolor=edgecolor, *args, **kwargs)

    if legend_on:
        leg = axes.legend(loc='upper left')
        leg.get_frame().set_alpha(0.5)

    if text_on:
        if not axes.texts:
            axes.text(
                0.01,
                0.97,
                label,
                horizontalalignment='left',
                verticalalignment='top',
                transform=axes.transAxes,
                color=text_color
            )
        else:
            temp_str = axes.texts[0].get_text() + '  ' + label
            axes.texts[0].set_text(temp_str)

    if zero_on:
        ylim = axes.get_ylim()
        if ylim[0] < 0 < ylim[1]:
            axes.hlines(0, 0, len(indicator))

    axes.autoscale_view()
    axes.set_xlim(-1, len(indicator) + 1)
    if kref is not None:
        ax_set_locator_formatter(axes, kref.get_datetime_list(), kref.get_query().ktype)
    else:
        k = indicator.get_context()
        if len(k) > 0:
            ax_set_locator_formatter(axes, k.get_datetime_list(), k.get_query().ktype)
    # draw()


def iheatmap(ind, axes=None):
    """
    Draw the year-month return heatmap of the indicator

    The indicator return = (the value at the end of the current month - the value at the end of
    the last month) / the value at the end of the last month * 100

    The indicator should have been calculated (i.e. it has values) and be a time series

    :param ind: the specified indicator
    :param axes: the axes object to draw in; None by default, which means a new axes object is created
    :return: None
    """
    if axes is None:
        axes = create_figure()

    if len(ind) == 0:
        hku_error(htr("The indicator length is 0; the indicator should have been calculated (i.e. it has values)"))
        return

    dates = ind.get_datetime_list()
    if len(dates) == 0:
        hku_error(htr("Failed to get the date list! The indicator should be a time series"))
        return

    values = ind.value_to_np()
    if values.dtype.names is not None:
        # value_to_np returns a structured array when the indicator holds several result sets,
        # and only the first result set is drawn here
        values = values[values.dtype.names[0]]
    data = pd.DataFrame({'date': dates, 'value': values})
    data = data[(data[['value']] != 0).all(axis=1)]

    # Extract the year and the month information
    data['year'] = data['date'].apply(lambda v: v.year)
    data['month'] = data['date'].apply(lambda v: v.month)

    # Get the return of each month
    monthly = data.groupby(['year', 'month']).last()['value'].reset_index()
    if len(monthly) < 2:
        hku_warn(htr("Insufficient monthly data!"))
        return

    monthly['return'] = ((monthly['value'] - monthly['value'].shift(1)) / monthly['value'].shift(1)) * 100.

    pivot_data = monthly.pivot_table(index='year', columns='month', values='return')

    sns.heatmap(pivot_data, cmap='RdYlGn_r', center=0, annot=True, fmt="<.2f", ax=axes)
    # Set the title and the axis labels
    axes.set_title(htr('Year-Month Return (%) Heatmap'))
    axes.set_xlabel(htr('Month'))
    axes.set_ylabel(htr('Year'))


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
    axes.text(0.01, 0.97, text, horizontalalignment='left', verticalalignment='top', transform=axes.transAxes)
    total = len(kdata)
    x = [i - 0.2 for i in range(total)]
    x1 = [x[i] for i, d in enumerate(bmacd) if d > 0]
    y1 = [i for i in bmacd if i > 0]
    x2 = [x[i] for i, d in enumerate(bmacd) if d <= 0]
    y2 = [i for i in bmacd if i <= 0]
    axes.bar(x1, y1, width=0.4, color='r', edgecolor='r')
    axes.bar(x2, y2, width=0.4, color='g', edgecolor='g')

    axt = axes.twinx()
    axt.grid(False)
    axt.set_yticks([])

    # Calculate and align the y-axis extremes of the MACD bar and the DIF/DEA
    y_all = np.concatenate([np.asarray(bmacd), np.asarray(fmacd), np.asarray(smacd)])
    y_min = np.nanmin(y_all)
    y_max = np.nanmax(y_all)
    y_pad = (y_max - y_min) * 0.1  # 10% blank space at the top and the bottom
    y_min -= y_pad
    y_max += y_pad
    axes.set_ylim(y_min, y_max)  # set the y range of the MACD bar
    axt.set_ylim(y_min, y_max)  # set the y range of the DIF/DEA

    fmacd.plot(axes=axt, linestyle='--', legend_on=False, text_on=False)
    smacd.plot(axes=axt, legend_on=False, text_on=False)

    for label in axt.get_xticklabels():
        label.set_visible(False)


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
    axes.text(0.01, 0.97, text, horizontalalignment='left', verticalalignment='top', transform=axes.transAxes)
    total = len(kdata)
    x = [i - 0.2 for i in range(0, total)]
    y = bmacd
    x1, x2, x3 = [x[0]], [], []
    y1, y2, y3 = [y[0]], [], []
    for i in range(1, total):
        if ref[i] - ref[i - 1] > 0 and y[i] - y[i - 1] > 0:
            x2.append(x[i])
            y2.append(y[i])
        elif ref[i] - ref[i - 1] < 0 and y[i] - y[i - 1] < 0:
            x3.append(x[i])
            y3.append(y[i])
        else:
            x1.append(x[i])
            y1.append(y[i])

    axes.bar(x1, y1, width=0.4, color='#BFBFBF', edgecolor='#BFBFBF')
    axes.bar(x2, y2, width=0.4, color='r', edgecolor='r')
    axes.bar(x3, y3, width=0.4, color='g', edgecolor='g')

    axt = axes.twinx()
    axt.grid(False)
    axt.set_yticks([])

    # Calculate and align the y-axis extremes of the MACD bar and the DIF/DEA
    y_all = np.concatenate([np.asarray(bmacd), np.asarray(fmacd), np.asarray(smacd)])
    y_min = np.nanmin(y_all)
    y_max = np.nanmax(y_all)
    y_pad = (y_max - y_min) * 0.1  # 10% blank space at the top and the bottom
    y_min -= y_pad
    y_max += y_pad
    axes.set_ylim(y_min, y_max)  # set the y range of the MACD bar
    axt.set_ylim(y_min, y_max)  # set the y range of the DIF/DEA

    fmacd.plot(axes=axt, linestyle='--', legend_on=False, text_on=False)
    smacd.plot(axes=axt, legend_on=False, text_on=False)

    for label in axt.get_xticklabels():
        label.set_visible(False)


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
    refdates = kdata.get_datetime_list()
    date_index = dict([(d, i) for i, d in enumerate(refdates)])

    if axes is None:
        if new:
            axes = create_figure()
            kplot(kdata, axes=axes)
        else:
            axes = gca()

    ylim = axes.get_ylim()
    height = ylim[1] - ylim[0]

    if style == 1:
        arrow_buy = dict(arrowstyle="->")
        arrow_sell = arrow_buy
    else:
        arrow_buy = dict(facecolor='red', frac=0.5)
        arrow_sell = dict(facecolor='blue', frac=0.5)

    dates = sg.get_buy_signal()
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        axes.annotate(
            'B', (pos, krecord.low - height * 0.01), (pos, krecord.low - height * 0.1),
            arrowprops=arrow_buy,
            horizontalalignment='center',
            verticalalignment='bottom',
            color='red'
        )

    dates = sg.get_sell_signal()
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        axes.annotate(
            'S', (pos, krecord.high + height * 0.01), (pos, krecord.high + height * 0.1),
            arrowprops=arrow_sell,
            horizontalalignment='center',
            verticalalignment='top',
            color='blue'
        )


def evplot(ev, ref_kdata, new=True, axes=None, upcolor='red', downcolor='blue', alpha=0.2):
    """Draw the market environment (the validity judgment)

    :param EnvironmentBase cn: the market environment
    :param KData ref_kdata: used as the date reference
    :param new: it takes effect only when axes is not specified; when True, a new window object is
        created and the drawing is done in it
    :param axes: the axes object in which to draw
    :param upcolor: the color when the environment is valid
    :param downcolor: the color when the environment is invalid
    :param alpha: the transparency
    """
    refdates = ref_kdata.get_datetime_list()
    if axes is None:
        if new:
            axes = create_figure(2)
            kplot(ref_kdata, axes=axes[0])
            axes = axes[1]
        else:
            axes = gca()

    x = np.array([i for i in range(len(refdates))])
    y1 = np.array([1 if ev.is_valid(d) else -1 for d in refdates])
    y2 = np.array([-1 if ev.is_valid(d) else 1 for d in refdates])
    axes.fill_between(x, y1, y2, where=y2 > y1, facecolor=downcolor, alpha=alpha)
    axes.fill_between(x, y1, y2, where=y2 < y1, facecolor=upcolor, alpha=alpha)


def cnplot(cn, new=True, axes=None, kdata=None, upcolor='red', downcolor='blue', alpha=0.2):
    """Draw the condition (the system valid condition)

    :param ConditionBase cn: the condition base
    :param new: it takes effect only when axes is not specified; when True, a new window object is
        created and the drawing is done in it
    :param axes: the axes object in which to draw
    :param KData kdata: the specified KData; when it is None, the condition is assumed to have its
        trading object specified, otherwise this argument is used as the trading object
    :param upcolor: the color when the condition is valid
    :param downcolor: the color when the condition is invalid
    :param alpha: the transparency
    """
    if kdata is None:
        kdata = cn.to
    else:
        cn.to = kdata

    refdates = kdata.get_datetime_list()
    if axes is None:
        if new:
            axes = create_figure(2)
            kplot(kdata, axes=axes[0])
            axes = axes[1]
        else:
            axes = gca()

    x = np.array([i for i in range(len(refdates))])
    y1 = np.array([1 if cn.is_valid(d) else -1 for d in refdates])
    y2 = np.array([-1 if cn.is_valid(d) else 1 for d in refdates])
    axes.fill_between(x, y1, y2, where=y2 > y1, facecolor=downcolor, alpha=alpha)
    axes.fill_between(x, y1, y2, where=y2 < y1, facecolor=upcolor, alpha=alpha)


def sysplot(sys, new=True, axes=None, style=1, only_draw_close=False):
    """Draw the actual buy/sell signals of the system

    :param SystemBase sys: the system instance
    :param new:   it takes effect only when axes is not specified; when True,
                   a new window object is created and the drawing is done in it
    :param axes:  the axes object in which to draw
    :param style: 1 | 2, the drawing style of the signal arrows
    :param bool only_draw_close: do not draw the K-line, and draw the close only
    """
    kdata = sys.to

    refdates = kdata.get_datetime_list()
    date_index = dict([(d, i) for i, d in enumerate(refdates)])

    if axes is None:
        if new:
            axes = create_figure()
        else:
            axes = gca()

    if only_draw_close:
        iplot(kdata.close, axes=axes)
    else:
        kplot(kdata, axes=axes)

    ylim = axes.get_ylim()
    height = ylim[1] - ylim[0]

    if style == 1:
        arrow_buy = dict(arrowstyle="->")
        arrow_sell = arrow_buy
    else:
        arrow_buy = dict(facecolor='red', frac=0.5)
        arrow_sell = dict(facecolor='blue', frac=0.5)

    tds = sys.tm.get_trade_list()
    buy_dates = []
    sell_dates = []
    for t in tds:
        if t.business == BUSINESS.BUY:
            buy_dates.append(t.datetime)
        elif t.business == BUSINESS.SELL:
            sell_dates.append(t.datetime)
        else:
            pass

    for d in buy_dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        axes.annotate(
            'B', (pos, krecord.low - height * 0.01), (pos, krecord.low - height * 0.1),
            arrowprops=arrow_buy,
            horizontalalignment='center',
            verticalalignment='bottom',
            color='red'
        )

    for d in sell_dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        axes.annotate(
            'S', (pos, krecord.high + height * 0.01), (pos, krecord.high + height * 0.1),
            arrowprops=arrow_sell,
            horizontalalignment='center',
            verticalalignment='top',
            color='blue'
        )


def tm_performance(tm: TradeManager, query: Query, ref_stk: Stock = None, ext: bool = True, log: bool = False):
    """
    Draw the system performance, i.e. the account cumulative return curve

    :param TradeManager tm: the account (the TradeManager) instance
    :param Stock ref_stk: the reference stock, sh000300 (the CSI 300) by default; the return curve
        of the reference object is drawn
    :param bool ext: whether to calculate the extended information (for the donators; otherwise the
        default statistics are still used)
    :param bool log: whether the Y axis uses the logarithmic coordinates
    :return: None
    """
    if ref_stk is None:
        ref_stk = get_stock('sh000300')

    sh000001_k = get_kdata('sh000001', query)
    ref_dates = sh000001_k.get_datetime_list()

    ref_k = ref_stk.get_kdata(query)

    funds_list = tm.get_funds_list(ref_dates)
    funds = [f.total_assets for f in funds_list]
    funds = VALUE(funds)
    funds_return = [f.total_assets / f.total_base if f.total_base != 0.0 else constant.null_price for f in funds_list]
    funds_return = VALUE(funds_return, align_dates=ref_dates)
    ref_return = ALIGN(ROCR(ref_k.close, 0), ref_dates)
    ref_return.name = f"{ref_stk.name}({ref_stk.market_code})"

    per = tm.get_performance(sh000001_k[-1].datetime, ext=ext)
    text = per.report()

    # Calculate the max drawdown percentage
    max_pullback = MDD(funds)[-1]

    # Calculate the percentage of the drawdown from the current point to the historical highest point
    mdd_current = MDD_CURRENT(funds)[-1]

    # Calculate the Sharpe ratio
    bond = ZHBOND10(ref_dates)
    sigma = STDEV(ROCP(funds), 0)  # n=0: the sample standard deviation of the whole period (expand-all)
    sigma = 15.874507866387544 * sigma[-1]  # 15.874 = sqrt(252)
    sharp = (per['Account Avg Annual Return %'] - bond[-1]) * 0.01 / sigma if sigma != 0.0 else 0.0

    invest_total = per['Total Invested Principal'] + per['Total Invested Assets']
    cur_fund = per['Current Total Assets']
    t1 = htr('Total Invested Assets: {:<.2f}    Current Total Assets: {:<.2f}    Current Profit: {:<.2f}').format(
        invest_total, cur_fund, cur_fund - invest_total)
    t2 = htr(
        'Current Strategy Return: {:<.2f}%    Annualized Return: {:<.2f}%    Max Drawdown: {:<.2f}%'
        '    Current Drawdown from Peak: {:<.2f}%').format(
            funds_return[-1] * 100 - 100, per["Account Avg Annual Return %"], max_pullback, mdd_current)
    t3 = htr('Win Rate: {:<.2f}%    Avg Win/Avg Loss: 1 : {:<.2f}    Sharpe Ratio: {:<.2f}').format(
        per['Win Rate %'], per['Avg Win / Avg Loss Ratio'], sharp)

    import matplotlib.pyplot as plt
    fg = plt.figure(figsize=(15, 10))
    gs = fg.add_gridspec(5, 4)
    ax1 = fg.add_subplot(gs[:4, :3])
    ax2 = fg.add_subplot(gs[:, 3:])
    ax3 = fg.add_subplot(gs[4:, :3])
    if log:
        ax1.set_yscale('log')

    ref_return.plot(axes=ax1,
                    legend_on=True,
                    label=htr('{} Return Curve').format(f'{ref_stk.name}({ref_stk.market_code})'))
    funds_return.plot(axes=ax1,
                      legend_on=True,
                      label=htr('{} Cumulative Return {:<.2f}%').format(tm.name, funds_return[-1] * 100.))
    ax1.set_title(htr('Account({}) Cumulative Return').format(tm.name))
    label = t1 + '\n\n' + t2 + '\n\n' + t3
    ax2.text(0,
             1,
             text,
             horizontalalignment='left',
             verticalalignment='top',
             transform=ax2.transAxes,
             # color='r'
             )
    ax3.text(0.02,
             0.9,
             label,
             horizontalalignment='left',
             verticalalignment='top',
             transform=ax3.transAxes,
             # color='r'
             )
    ax2.xaxis.set_visible(False)
    ax2.yaxis.set_visible(False)
    ax2.set_frame_on(False)
    ax3.xaxis.set_visible(False)
    ax3.yaxis.set_visible(False)
    ax3.set_frame_on(False)
    if log:
        from matplotlib.ticker import FuncFormatter, NullFormatter
        ax1.yaxis.set_major_formatter(NullFormatter())
        ax1.yaxis.set_minor_formatter(NullFormatter())
        ax1.yaxis.set_major_formatter(FuncFormatter(lambda x, pos: f'{x:.1f}'))
        ax1.yaxis.set_minor_formatter(FuncFormatter(lambda x, pos: f'{x:.1f}'))
    return ax1  # return the main axes


def sys_performance(sys, ref_stk=None, ext=True, log=False):
    """
    Draw the system performance, i.e. the account cumulative return curve

    :param SystemBase | PortfolioBase sys: the SYS or PF instance
    :param Stock ref_stk: the reference stock, sh000300 (the CSI 300) by default; the return curve
        of the reference object is drawn
    :param bool ext: whether to calculate the extended information (for the donators; otherwise the
        default statistics are still used)
    :param bool log: whether the Y axis uses the logarithmic coordinates
    :return: None
    """
    if ref_stk is None:
        ref_stk = get_stock('sh000300')

    query = sys.query
    return tm_performance(sys.tm, query, ref_stk, ext=ext, log=log)


def tm_heatmap(tm, start_date, end_date=None, axes=None, show_high_low=False):
    """
    Draw the year-month return heatmap of the account

    :param tm: the trade manager (the account)
    :param start_date: the start date
    :param end_date: the end date, today by default
    :param axes: the axes object to draw in; None by default, which means a new axes object is created
    :param show_high_low: whether to display the monthly highest and lowest returns, False by default
    :return: None
    """
    if axes is None:
        axes = create_figure()

    if end_date is None or end_date == Datetime():
        end_date = Datetime.today() + Days(1)

    dates = get_date_range(start_date, end_date)
    if len(dates) == 0:
        hku_error(htr("No data, please check the date range! start_date={}, end_date={}"), start_date, end_date)
        return

    funds = tm.get_funds_curve(dates)
    if len(funds) == 0:
        hku_error(
            htr("Failed to get the tm return curve, please check the tm initial date!"
                " tm.init_datetime={} start_date={}, end_date={}"), tm.init_datetime, start_date, end_date)
        return

    data = pd.DataFrame({'date': dates, 'value': funds})
    data = data[(data[['value']] != 0).all(axis=1)]

    # Extract the year and the month information
    data['year'] = data['date'].apply(lambda v: v.year)
    data['month'] = data['date'].apply(lambda v: v.month)

    # Get the last value, the highest value and the lowest value of each month
    monthly = data.groupby(['year', 'month']).agg(
        last_value=('value', 'last'),
        max_value=('value', 'max'),
        min_value=('value', 'min')
    ).reset_index()
    if len(monthly) < 2:
        hku_warn(htr("Insufficient monthly data!"))
        return

    monthly['return'] = ((monthly['last_value'] - monthly['last_value'].shift(1)) /
                         monthly['last_value'].shift(1)) * 100.
    monthly['prev_last'] = monthly['last_value'].shift(1)
    monthly['max_return'] = ((monthly['max_value'] - monthly['prev_last']) / monthly['prev_last']) * 100.
    monthly['min_return'] = ((monthly['min_value'] - monthly['prev_last']) / monthly['prev_last']) * 100.

    pivot_data = monthly.pivot_table(index='year', columns='month', values='return')

    yearly_value = monthly.groupby('year').last()['last_value'].reset_index()
    yearly_first = data.groupby('year').first()['value'].reset_index()

    yearly_value['year_return'] = ((yearly_value['last_value'] - yearly_value['last_value'].shift(1)
                                    ) / yearly_value['last_value'].shift(1)) * 100.
    yearly_value.loc[0, 'year_return'] = (
        (yearly_value.loc[0, 'last_value'] - yearly_first.loc[0, 'value']) / yearly_first.loc[0, 'value']) * 100.

    year_return_df = yearly_value.set_index('year')['year_return']
    annual_col = htr('Annual Return')
    pivot_data[''] = np.nan
    pivot_data[annual_col] = year_return_df

    annot_matrix = []
    for year in pivot_data.index:
        row = []
        for col in pivot_data.columns:
            if col in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]:
                ret = pivot_data.loc[year, col] if year in pivot_data.index and col in pivot_data.columns else np.nan
                if not np.isnan(ret):
                    if show_high_low:
                        max_ret = monthly[(monthly['year'] == year) & (monthly['month'] == col)]['max_return'].values[0]
                        min_ret = monthly[(monthly['year'] == year) & (monthly['month'] == col)]['min_return'].values[0]
                        row.append(f"{ret:.2f}\n↑{max_ret:.2f} ↓{min_ret:.2f}")
                    else:
                        row.append(f"{ret:.2f}")
                else:
                    row.append("")
            elif col == '':
                row.append("")
            elif col == annual_col:
                row.append(f"{year_return_df.loc[year]:.2f}" if year in year_return_df.index else "")
        annot_matrix.append(row)

    max_abs_return = monthly['return'].abs().max()
    v_limit = max(max_abs_return, 5)

    sns.heatmap(pivot_data, cmap='RdYlGn_r', center=0, vmin=-v_limit, vmax=v_limit, annot=annot_matrix, fmt='', ax=axes)
    # Set the title and the axis labels
    axes.set_title(htr('{} Year-Month Return (%) Heatmap').format(tm.name))
    axes.set_xlabel(htr('Month'))
    axes.set_ylabel(htr('Year'))


def tm_year_profit(tm, start_date, end_date=None, axes=None, show_high_low=True):
    """
    Draw the annual return bar chart of the account

    :param tm: the trade manager (the account)
    :param start_date: the start date
    :param end_date: the end date, today by default
    :param axes: the axes object to draw in; None by default, which means a new axes object is created
    :param show_high_low: whether to display the yearly highest and lowest returns, False by default
    :return: None
    """
    if axes is None:
        axes = create_figure()

    if end_date is None or end_date == Datetime():
        end_date = Datetime.today() + Days(1)

    dates = get_date_range(start_date, end_date)
    if len(dates) == 0:
        hku_error(htr("No data, please check the date range! start_date={}, end_date={}"), start_date, end_date)
        return

    funds = tm.get_funds_curve(dates)
    if len(funds) == 0:
        hku_error(
            htr("Failed to get the tm return curve, please check the tm initial date!"
                " tm.init_datetime={} start_date={}, end_date={}"), tm.init_datetime, start_date, end_date)
        return

    data = pd.DataFrame({'date': dates, 'value': funds})
    data = data[(data[['value']] != 0).all(axis=1)]

    data['year'] = data['date'].apply(lambda v: v.year)

    yearly = data.groupby('year').agg(
        last_value=('value', 'last'),
        max_value=('value', 'max'),
        min_value=('value', 'min')
    ).reset_index()
    yearly_first = data.groupby('year').first()['value'].reset_index()

    if len(yearly) < 1:
        hku_warn(htr("Insufficient yearly data!"))
        return

    yearly['return'] = ((yearly['last_value'] - yearly['last_value'].shift(1)) /
                        yearly['last_value'].shift(1)) * 100.
    yearly.loc[0, 'return'] = ((yearly.loc[0, 'last_value'] - yearly_first.loc[0, 'value']) /
                               yearly_first.loc[0, 'value']) * 100.

    yearly['prev_last'] = yearly['last_value'].shift(1)
    yearly['max_return'] = ((yearly['max_value'] - yearly['prev_last']) / yearly['prev_last']) * 100.
    yearly['min_return'] = ((yearly['min_value'] - yearly['prev_last']) / yearly['prev_last']) * 100.
    yearly.loc[0, 'max_return'] = ((yearly.loc[0, 'max_value'] - yearly_first.loc[0, 'value']) /
                                   yearly_first.loc[0, 'value']) * 100.
    yearly.loc[0, 'min_return'] = ((yearly.loc[0, 'min_value'] - yearly_first.loc[0, 'value']) /
                                   yearly_first.loc[0, 'value']) * 100.

    years = yearly['year'].astype(str)
    returns = yearly['return']
    max_returns = yearly['max_return']
    min_returns = yearly['min_return']

    bar_width = 0.8
    x_pos = range(len(years))

    for i, (ret, max_ret, min_ret) in enumerate(zip(returns, max_returns, min_returns)):
        base_color = '#FF4444' if ret >= 0 else '#44FF44'
        max_color = '#FFAAAA' if ret >= 0 else '#AAFFAA'
        min_color = '#CC0000' if ret >= 0 else '#00CC00'

        if show_high_low:
            if max_ret > ret:
                axes.bar(x_pos[i], max_ret - ret, bottom=ret, width=bar_width,
                         color=max_color, alpha=0.4, label=htr('Highest Return Range') if i == 0 else "")
            if min_ret < ret:
                axes.bar(x_pos[i], ret - min_ret, bottom=min_ret, width=bar_width,
                         color=min_color, alpha=0.7, label=htr('Lowest Return Range') if i == 0 else "")

        axes.bar(x_pos[i], ret, width=bar_width, color=base_color, alpha=0.9)

        if show_high_low:
            axes.text(x_pos[i], max(ret, max_ret) + (max(ret, max_ret) - min(ret, min_ret)) * 0.05,
                      f'{ret:.2f}%\n↑{max_ret:.2f} ↓{min_ret:.2f}',
                      ha='center', va='bottom', fontsize=9)
        else:
            axes.text(x_pos[i], ret + (ret if ret >= 0 else -ret) * 0.05,
                      f'{ret:.2f}%', ha='center', va='bottom', fontsize=9)

    axes.set_xticks(x_pos)
    axes.set_xticklabels(years)

    if show_high_low:
        axes.legend()

    axes.set_title(htr('{} Annual Return (%) Bar Chart').format(tm.name))
    axes.set_xlabel(htr('Year'))
    axes.set_ylabel(htr('Return (%)'))
    axes.grid(axis='y', linestyle='--', alpha=0.7)


def sys_heatmap(sys, axes=None):
    """
    Draw the year-month return heatmap of the system
    """
    hku_check(sys.tm is not None, htr("The system has no trade manager initialized"))
    query = sys.query
    k = get_kdata('sh000001', query)
    tm_heatmap(sys.tm, k[0].datetime, k[-1].datetime, axes)


# ============================================================================
# The TDX drawing functions
# ============================================================================
DRAWNULL = constant.null_price


def RGB(r: int, g: int, b: int):
    hku_check(0 <= r <= 255 and 0 <= g <= 255 and 0 <= b <= 255, "r,g,b must in [0,255]!")
    return f"#{r:02x}{g:02x}{b:02x}"


def STICKLINE(cond: Indicator, price1: Indicator, price2: Indicator, width: float = 2.0,
              empty: bool = False, color='m', alpha=1.0, kdata=None, new=False, axes=None):
    """Draw a bar with the width of width between price1 and price2 when cond is satisfied.

    Note: cond, price1 and price2 should contain data; otherwise, specify kdata as the context of
    the indicator calculation

    The parameters:
        cond (Indicator): the condition expression, used to determine whether to draw the bar
        price1 (Indicator): the first price
        price2 (Indicator): the second price
        width (float, optional): the width of the bar. Defaults to 2.0.
        empty (bool, optional): hollow. Defaults to False.
        kdata (_type_, optional): the specified context K-line. Defaults to None.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): draw in the specified axes. Defaults to None.
        color (str, optional): the color. Defaults to 'm'.
        alpha (float, optional): the transparency. Defaults to 1.0.
    """
    hku_check(cond is not None and price1 is not None and price2 is not None, "cond, price1, price2 cannot be None")

    if kdata is not None:
        cond = cond(kdata)
        price1 = price1(kdata)
        price2 = price2(kdata)
    hku_check(len(cond) == len(price1) == len(price2), "cond, price1, price2 length not match")
    hku_warn_if(len(cond) <= 0, "cond, price1, price2 length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    width = 0.3 * width
    OFFSET = width / 2.0
    for i in range(len(cond)):
        if cond[i] > 0.:
            height = abs(price1[i] - price2[i])
            lower = min(price1[i], price2[i])
            rect = Rectangle(xy=(i - OFFSET, lower), width=width, height=height,
                             facecolor=color, edgecolor=color, fill=(not empty))
            rect.set_alpha(alpha)
            axes.add_patch(rect)

    axes.autoscale_view()
    axes.set_xlim(-1, len(cond) + 1)


def DRAWBAND(val1: Indicator, color1='m', val2: Indicator = None, color2='b', kdata=None, alpha=0.2, new=False, axes=None, linestyle='-'):
    """Draw the band

    Usage: DRAWBAND(val1, color1, val2, color2); when val1 > val2, the area between val1 and val2 is
    filled with color1; when val1 < val2, it is filled with color2. The colors here all use the
    matplotlib color codes.
    For example: DRAWBAND(OPEN, 'r', CLOSE, 'b')

        val1 (Indicator): the indicator 1
        color1 (str, optional): the color 1. Defaults to 'm'.
        val2 (Indicator, optional): the indicator 2. Defaults to None.
        color2 (str, optional): the color 2. Defaults to 'b'.
        kdata (_type_, optional): the indicator context. Defaults to None.
        alpha (float, optional): the transparency. Defaults to 0.2.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): draw in the specified axes. Defaults to None.
        linestyle (str, optional): the envelope line type. Defaults to '-'.
    """
    hku_check(val1 is not None, "val1 cannot be None")

    if kdata is not None:
        val1 = val1(kdata)
        if val2 is not None:
            val2 = val2(kdata)

    if val2 is None:
        val2 = CVAL(val1, 0.)

    hku_check(len(val1) == len(val2), "val1, val2 length not match")
    hku_warn_if(len(val1) <= 0, "val1, val2 length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    cond = IF(val1 <= val2, val1, val2)
    axes.fill_between(range(cond.discard, len(val1)), val1[cond.discard:], cond[cond.discard:], alpha=alpha,
                      color=color1, facecolor=color1, edgecolor=color1, linestyle=linestyle)

    cond = IF(val1 > val2, val1, val2)
    axes.fill_between(range(cond.discard, len(val1)), val1[cond.discard:], cond[cond.discard:], alpha=alpha,
                      color=color2, facecolor=color2, edgecolor=color2, linestyle=linestyle)

    axes.autoscale_view()
    axes.set_xlim(-1, len(val1) + 1)


def PLOYLINE(cond: Indicator, price: Indicator, kdata: KData = None, color: str = 'm', linewidth=1.0, new=False, axes=None, *args, **kwargs):
    """Draw a polyline on the chart.

    Usage: PLOYLINE(COND, PRICE); when the COND condition is satisfied, a polyline is drawn with
    the PRICE position as the vertices.
    For example: PLOYLINE(HIGH>=HHV(HIGH,20),HIGH, kdata=k) draws a polyline between the points
    that hit a new 20-day high.

        cond (Indicator): the specified condition
        price (Indicator): the position
        kdata (KData, optional): the specified context. Defaults to None.
        color (str, optional): the color. Defaults to 'b'.
        linewidth (float, optional): the width. Defaults to 1.0.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): the specified axes. Defaults to None.
    """
    hku_check(cond is not None and price is not None, "cond, price cannot be None")

    ind = IF(cond, price, constant.null_price)
    if kdata is not None:
        ind = ind(kdata)
        price = price(kdata)
    hku_check(len(ind) == len(price), "cond, price length not match!")
    hku_warn_if(len(ind) <= 0, "cond length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    # ind.plot(new=new, axes=axes, color=color, linewidth=linewidth, *args, **kwargs)
    x, y = [], []
    for i in range(ind.discard, len(ind)):
        val = ind[i]
        if not isnan(val):
            x.append(i)
            y.append(val)
    if len(x) > 0:
        axes.plot(x, y, color=color, linewidth=linewidth, *args, **kwargs)

    axes.autoscale_view()
    axes.set_xlim(-1, len(ind) + 1)


def DRAWLINE(cond1: Indicator, price1: Indicator, cond2: Indicator, price2: Indicator, expand: int = 0, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs):
    """Draw a straight line on the chart.

    Usage: DRAWLINE(cond1, price1, cond2, price2, expand)
    When the COND1 condition is satisfied, the start point of the line is drawn at the PRICE1
    position; when the COND2 condition is satisfied, the end point is drawn at the PRICE2
    position, and EXPAND is the extension type.
    For example: DRAWLINE(HIGH>=HHV(HIGH,20),HIGH,LOW<=LLV(LOW,20),LOW,1) draws a straight line
    between the points that hit a new 20-day high and a new 20-day low, and extends it to the
    right.

        cond1 (Indicator): the condition 1
        price1 (Indicator): the position 1
        cond2 (Indicator): the condition 2
        price2 (Indicator): the position 2
        expand (int, optional): 0: no extension | 1: extend to the right | 10: extend to the left
            | 11: extend to both directions. Defaults to 0.
        kdata (KData, optional): the specified context. Defaults to None.
        color (str, optional): the specified color. Defaults to 'm'.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): the specified axes. Defaults to None.
    """
    hku_check(cond1 is not None and cond2 is not None and price1 is not None and price2 is not None,
              "cond1, cond2, price1, price2 cannot be None")
    hku_check(expand in (0, 1, 10, 11), "expand must be 0, 1, 10 or 11")

    if kdata is not None:
        cond1 = cond1(kdata)
        price1 = price1(kdata)
        cond2 = cond2(kdata)
        price2 = price2(kdata)
    hku_check(len(cond1) == len(cond2) == len(price1) == len(price2), "cond1, cond2, price1, price2 length not match")
    hku_warn_if(len(cond1) <= 0, "cond1, cond2, price1, price2 length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    length = len(cond1)
    x1, y1 = None, None
    for i in range(cond1.discard, length):
        cond1_val = cond1[i]
        if cond1_val > 0.:
            if x1 is None:
                x1, y1 = i, price1[i]
            else:
                x1, y1 = None, None
        cond2_val = cond2[i]
        if cond2_val > 0.:
            if x1 is not None:
                if expand == 0:
                    x = [x1, i]
                    y = [y1, price2[i]]
                elif expand == 1:
                    x = [n for n in range(i, length)]
                    x.insert(0, x1)
                    val = price2[i]
                    y = [val for n in range(i, length)]
                    y.insert(0, y1)
                elif expand == 10:
                    x = [n for n in range(0, i+1)]
                    val = price2[i]
                    y = [val for n in range(0, i+1)]
                elif expand == 11:
                    x = [n for n in range(0, length)]
                    val = price2[i]
                    y = [val for n in range(0, length)]
                axes.plot(x, y, color=color, *args, **kwargs)
                x1, y1 = None, None

    axes.autoscale_view()
    axes.set_xlim(-1, len(cond1) + 1)


def DRAWTEXT(cond: Indicator, price: Indicator, text: str, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs):
    """Display the text on the chart.

    Usage: DRAWTEXT(cond, price, text); when the cond condition is satisfied, the text is drawn
    at the price position.
    For example: DRAWTEXT(CLOSE/OPEN>1.08,LOW,'大阳线') displays the text '大阳线' at the lowest
    price position when the real body of the bullish candle of the day is greater than 8%.

        cond (Indicator): the condition
        price (Indicator): the display position
        text (str): the text to display
        kdata (KData, optional): the specified context. Defaults to None.
        color (str, optional): the specified color. Defaults to 'm'.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): the specified axes. Defaults to None.
    """
    hku_check(cond is not None and price is not None, "cond, price cannot be None")

    if kdata is not None:
        cond = cond(kdata)
        price = price(kdata)
    hku_check(len(cond) == len(price), "cond, price length not match")
    hku_warn_if(len(cond) <= 0, "cond length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    for i in range(cond.discard, len(cond)):
        if cond[i] > 0.:
            axes.text(i, price[i], text, color=color, *args, **kwargs)

    axes.autoscale_view()
    axes.set_xlim(-1, len(cond) + 1)


def DRAWTEXT_FIX(cond: Indicator, x: float, y: float,  type: int, text: str, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs):
    """Display the text at a fixed position

    Usage: DRAWTEXT_FIX(cond, x, y, text); ISLASTBAR is usually added to cond. When the cond
    condition is satisfied, the text is drawn at the (X, Y) position in the current indicator
    window, where X and Y are the percentages of the writing point relative to the upper left
    corner of the window.

    For example: DRAWTEXT_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08),0.5,0.5,0,'大阳线') displays the
    text '大阳线' in the middle of the window when the real body of the bullish candle of the
    last trading day is greater than 8%.

        cond (Indicator): the condition
        x (float): the x coordinate
        y (float): the y coordinate
        type (int, optional): 0 left aligned | 1 right aligned.
        text (str): the text to display
        kdata (KData, optional): the specified context. Defaults to None.
        color (str, optional): the specified color. Defaults to 'm'.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): the specified axes. Defaults to None.
    """
    hku_check(cond is not None, "cond cannot be None")
    if kdata is not None:
        cond = cond(kdata)
    hku_warn_if(len(cond) <= 0, "cond length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    for i in range(cond.discard, len(cond)):
        if cond[i] > 0.:
            axes.text(x, 1-y, text, horizontalalignment='left' if type == 0 else 'right', verticalalignment='top',
                      transform=axes.transAxes, color=color, *args, **kwargs)

    axes.autoscale_view()
    axes.set_xlim(-1, len(cond) + 1)


def DRAWNUMBER(cond: Indicator, price: Indicator, number: Indicator, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs):
    """Draw the number.

    Usage: DRAWNUMBER(cond, price, number); when the cond condition is satisfied, the number is
    drawn at the price position.
    For example: DRAWNUMBER(CLOSE/OPEN>1.08,LOW,C) displays the close price at the lowest price
    position when the real body of the bullish candle of the day is greater than 8%.

        cond (Indicator): the condition
        price (Indicator): the drawing position
        number (Indicator): the number to draw
        kdata (KData, optional): the specified context. Defaults to None.
        color (str, optional): the specified color. Defaults to 'm'.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): the specified axes. Defaults to None.
    """
    hku_check(cond is not None and price is not None, "cond, price cannot be None")

    if kdata is not None:
        cond = cond(kdata)
        price = price(kdata)
        number = number(kdata)
    hku_check(len(cond) == len(price), "cond, price, number length not match")
    hku_warn_if(len(cond) <= 0, "cond length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    for i in range(cond.discard, len(cond)):
        if cond[i] > 0.:
            axes.text(i, price[i], str(number[i]), color=color, *args, **kwargs)

    axes.autoscale_view()
    axes.set_xlim(-1, len(cond) + 1)


def DRAWNUMBER_FIX(cond: Indicator, x: float, y: float, type: int, number: float, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs):
    """Display the number at a fixed position.

    Usage: DRAWNUMBER_FIX(cond, x, y, type, number); ISLASTBAR is usually added to cond. When the
    cond condition is satisfied, the number is drawn at the (x, y) position in the current
    indicator window, where x and y are the percentages of the writing point relative to the
    upper left corner of the window, and type: 0 for left aligned, 1 for right aligned.

    For example: DRAWNUMBER_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08), 0.5,0.5,0,C) displays the close
    price in the middle of the window when the real body of the bullish candle of the last
    trading day is greater than 8%

    Args:
        cond (Indicator): _description_
        x (float): _description_
        y (float): _description_
        type (int): _description_
        number (Indicator): _description_
        kdata (KData, optional): _description_. Defaults to None.
        color (str, optional): _description_. Defaults to 'm'.
        new (bool, optional): _description_. Defaults to False.
        axes (_type_, optional): _description_. Defaults to None.
    """
    DRAWTEXT_FIX(cond, x, y, type, str(number), kdata, color, new, axes, *args, **kwargs)


def DRAWSL(cond: Indicator, price: Indicator, slope: Union[Indicator, float, int], length: Union[Indicator, float, int], direct: int, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs):
    """Draw the slanted line.

    Usage: DRAWSL(cond, price, slope, length, diect); when the cond condition is satisfied, a
    slanted line is drawn at the price position, where slope is the slope, length is the length,
    and direct: 0 for extending to the right, 1 for extending to the left, 2 for extending to
    both directions.

    Note:
    1. The vertical height difference between the K-lines is slope;
    2. When slope is 0, it is a horizontal line;
    3. When slope is 10000, it is a vertical line, length is the pixel height upward, and direct
       indicates extending upward or downward;
    4. slope and length support the variables;

        cond (Indicator): the condition indicator
        price (Indicator): the price
        slope (int|float|Indicator): the slope
        length (int|float|Indicator): the length
        direct (int): the direction
        kdata (KData, optional): the specified context. Defaults to None.
        color (str, optional): the color. Defaults to 'm'.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): the specified axes. Defaults to None.
    """
    hku_check(cond is not None and price is not None, "cond, price cannot be None")
    hku_check(direct in (0, 1, 2), "direct must be 0,1,2")

    if kdata is not None:
        cond = cond(kdata)
        price = price(kdata)
        slope = slope(kdata) if isinstance(slope, Indicator) else [slope for i in range(len(kdata))]
        length = length(kdata) if isinstance(length, Indicator) else [length for i in range(len(kdata))]

    hku_check(len(cond) == len(price), "cond, price length not match")
    hku_warn_if(len(cond) <= 0, "cond length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    for i in range(cond.discard, len(cond)):
        val = price[i]
        if not isnan(val):
            if slope[i] < 10000:
                x = length[i] / math.sqrt(1+slope[i]**2)
                y = x * slope[i]
                if direct == 0:
                    axes.plot([i, i+x], [val, val+y], color=color, *args, **kwargs)
                elif direct == 1:
                    axes.plot([i-x, i], [val-y, val], color=color, *args, **kwargs)
                else:
                    axes.plot([i-x*0.5, i, i+x*0.5], [val-y*0.5, val, val+y*0.5], color=color, *args, **kwargs)
            else:
                y = length[i]
                if direct == 0:
                    axes.plot([i, i], [val, val+y], color=color, *args, **kwargs)
                elif direct == 1:
                    axes.plot([i, i], [val, val-y], color=color, *args, **kwargs)
                else:
                    axes.plot([i, i, i], [val-y*0.5, val, val+y*0.5], color=color, *args, **kwargs)

    axes.autoscale_view()
    axes.set_xlim(-1, len(cond) + 1)


def DRAWIMG(cond: Indicator, price: Indicator, img: str, kdata: KData = None, new=False, axes=None, *args, **kwargs):
    """Draw the image

    Usage: DRAWIMG(cond, price, 'the image file name'); when the cond condition is satisfied,
    the specified image is drawn at the price position.
    For example: DRAWIMG(O>C, CLOSE, '123.png').

        cond (Indicator): the specified condition
        price (Indicator): the specified price
        img (str): the image file name
        kdata (KData, optional): the context. Defaults to None.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): draw in the specified axes. Defaults to None.
    """
    hku_check(cond is not None and price is not None, "cond, price cannot be None")

    if kdata is not None:
        cond = cond(kdata)
        price = price(kdata)
    hku_check(len(cond) == len(price), "cond, price length not match")
    hku_warn_if(len(cond) <= 0, "cond length <=0")

    if axes is None:
        axes = create_figure() if new else gca()

    image = imread(img)

    p = axes.get_window_extent()
    pw = p.x1 - p.x0
    ph = p.y1 - p.y0
    x0, x1 = axes.get_xlim()
    y0, y1 = axes.get_ylim()
    xw = x1 - x0
    yh = y1 - y0
    pixel = 20.  # the display pixel size
    w = xw / pw * pixel
    h = yh / ph * pixel
    for i in range(cond.discard, len(cond)):
        if (not isnan(cond[i])) and cond[i] > 0. and (not isinf(cond[i])) and (not isnan(price[i])) and (not isinf(price[i])):
            axes.imshow(image, extent=[i-w, i+w, price[i]-h, price[i]+h], *args, **kwargs)

    axes.set_aspect('auto')
    axes.autoscale_view()
    axes.set_ylim(y0, y1)
    axes.set_xlim(-1, len(cond) + 1)


DRAWBMP = DRAWIMG


def DRAWICON(cond: Indicator, price: Indicator, type: int, kdata: KData = None, new=False, axes=None, *args, **kwargs):
    """Draw the built-in icon

    Usage: DRAWICON(cond, price, 1); when the cond condition is satisfied, the built-in icon
    with the number 1 is drawn at the price position.
    For example: DRAWICON(O>C, CLOSE, 1).

    SHOWICONS() can be used to display all the built-in icons.

        cond (Indicator): the specified condition
        price (Indicator): the specified price
        type (int): the icon number
        kdata (KData, optional): the context. Defaults to None.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): draw in the specified axes. Defaults to None.
    """
    DRAWIMG(cond, price, f'{ICON_PATH}/icon/{type}.png', kdata, new, axes, *args, **kwargs)


def SHOWICONS():
    """Display all the built-in icons"""
    axes = create_one_axes_figure([8, 6])
    p = axes.get_window_extent()
    pw = p.x1 - p.x0
    ph = p.y1 - p.y0
    x0, x1 = axes.get_xlim()
    y0, y1 = axes.get_ylim()
    xw = x1 - x0
    yh = y1 - y0
    pixel = 100.  # the display pixel size
    w = xw / pw * pixel
    h = yh / ph * pixel

    row, col = 5, 10
    for i in range(row):
        for j in range(col):
            n = i*col+j + 1
            name = f'{ICON_PATH}/icon/{n}.png'
            if os.path.exists(name):
                try:
                    x = j*w
                    y = i*h
                    axes.imshow(imread(name), extent=[x, x+w, 1-(y+h), 1-y])
                except:
                    pass
    axes.set_aspect('auto')
    axes.autoscale_view()
    axes.set_ylim(y0, y1)
    axes.set_xlim(x0, x1)


def DRAWRECTREL(left: int, top: int, right: int, bottom: int, color='m', frame=True, fill=True, alpha=0.1, new=False, axes=None, *args, **kwargs):
    """Draw a rectangle at the relative position.

    Note: the origin is the upper left corner (0, 0) of the axes, which is different from
    matplotlib.
    Usage: DRAWRECTREL(left, top, right, bottom, color); a rectangle is drawn with (left, top)
    of the chart window as the upper left corner and (right, bottom) as the lower right corner.
    The unit of the coordinates is 1/1000 of the window along the horizontal and the vertical
    directions, and the value range is 0-999; out of the range, it may be displayed outside
    the chart window. The middle of the rectangle is filled with the color COLOR, and COLOR=0
    means no filling.
    For example: DRAWRECTREL(0,0,500,500,RGB(255,255,0)) draws a rectangle in yellow at the
    leftmost upper 1/4 of the chart.

        left (int): the x of the upper left corner
        top (int): the y of the upper left corner
        right (int): the x of the lower right corner
        bottom (int): the y of the lower right corner
        color (str, optional): the specified color. Defaults to 'm'.
        frame (bool, optional): add the border. Defaults to False.
        fill (bool, optional): fill the color. Defaults to True.
        alpha (float, optional): the transparency. Defaults to 0.1.
        new (bool, optional): draw in a new window. Defaults to False.
        axes (_type_, optional): the specified axes. Defaults to None.
    """
    if axes is None:
        axes = create_figure() if new else gca()

    x0, x1 = axes.get_xlim()
    y0, y1 = axes.get_ylim()
    w = x1 - x0
    h = y1 - y0

    limit = 1000
    cx = w / limit
    cy = h / limit
    x = left * cx + x0
    y = (limit - bottom) * cy + y0
    width = (right - left) * cx
    height = (bottom - top) * cy
    print(x, y, width, height)
    if frame:
        rect = Rectangle(xy=(x, y), width=width, height=height, facecolor=color, edgecolor=color, fill=fill)
    else:
        rect = Rectangle(xy=(x, y), width=width, height=height, facecolor=color, fill=fill)
    rect.set_alpha(alpha)
    axes.add_patch(rect)
