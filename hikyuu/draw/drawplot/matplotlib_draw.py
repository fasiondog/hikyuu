# -*- coding: utf8 -*-
# cp936
"""
交互模式下绘制相关图形，如K线图，美式K线图
"""
import sys
import os
import datetime
import logging
import numpy as np
import matplotlib
import math
from typing import Union
from matplotlib.pylab import Rectangle, gca, gcf, figure, ylabel, axes, draw
from matplotlib import rcParams
from matplotlib.font_manager import FontManager, _log as fm_logger
from matplotlib.lines import Line2D, TICKLEFT, TICKRIGHT
from matplotlib.ticker import FuncFormatter, FixedLocator
from matplotlib.image import imread

from hikyuu import *
from hikyuu import constant, isnan, Indicator, KData, IF

from .common import get_draw_title


ICON_PATH = os.path.dirname(os.path.abspath(__file__))


def set_mpl_params():
    '''设置交互及中文环境参数'''
    if in_interactive_session():
        rcParams['interactive'] = True

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
    """生成一个仅含有1个坐标轴的figure，并返回其坐标轴对象

    :param figsize: (宽, 高)
    :return: ax
    """
    rect1 = [0.05, 0.05, 0.9, 0.90]
    fg = figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    return ax1


def create_two_axes_figure(figsize=(10, 8)):
    """生成一个含有2个坐标轴的figure，并返回坐标轴列表

    :param figsize: (宽, 高)
    :return: (ax1, ax2)
    """
    rect1 = [0.05, 0.35, 0.9, 0.60]
    rect2 = [0.05, 0.05, 0.9, 0.30]

    fg = figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    ax2 = fg.add_axes(rect2, sharex=ax1)

    return ax1, ax2


def create_three_axes_figure(figsize=(10, 8)):
    """生成一个含有3个坐标轴的figure，并返回坐标轴列表

    :param figsize: (宽, 高)
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
    """生成一个含有4个坐标轴的figure，并返回坐标轴列表

    :param figsize: (宽, 高)
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
    elif n == 4:
        return create_four_axes_figure(figsize)
    else:
        print("Max support axes number is 4!")
        return None


class StockFuncFormatter(object):
    """用于坐标轴显示日期
    关于matplotlib中FuncFormatter的使用方法，请参见：
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
    """获取显示日线时使用的Major Locator和Major Formatter"""
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
    """获取显示分钟线时使用的Major Locator和Major Formatter"""
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
    """ 设置指定坐标轴的日期显示，根据指定的K线类型优化X轴坐标显示

    :param axes: 指定的坐标轴
    :param dates: Datetime构成可迭代序列
    :param Query.KType typ: K线类型
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
    """用于调整上下紧密相连的坐标轴显示时，其上一坐标轴最小值刻度和下一坐标轴最大值刻度
    显示重叠的问题。

    :param axeslist: 上下相连的坐标轴列表 (ax1,ax2,...)
    """
    for ax in axeslist[:-1]:
        for label in ax.get_xticklabels():
            label.set_visible(False)
        ylabels = ax.get_yticklabels()
        ylabels[0].set_visible(False)


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
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f 涨幅:%.2f%%' % (
        last_record.datetime.number / 10000, last_record.open, last_record.high, last_record.low, last_record.close,
        100 * (last_record.close - kdata[-2].close) / kdata[-2].close
    )
    axes.text(
        0.99, 0.97, text, horizontalalignment='right', verticalalignment='top', transform=axes.transAxes, color=color
    )

    axes.autoscale_view()
    axes.set_xlim(-1, len(kdata) + 1)
    ax_set_locator_formatter(axes, kdata.get_datetime_list(), kdata.get_query().ktype)
    # draw()


def mkplot(kdata, new=True, axes=None, colorup='r', colordown='g', ticksize=3):
    """绘制美式K线图

    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
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
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f' % (
        last_record.datetime.number / 10000, last_record.open, last_record.high, last_record.low, last_record.close
    )
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
        axes = create_figure() if new else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    py_indicatr = [None if x == constant.null_price else x for x in indicator]
    axes.plot(py_indicatr, linestyle=linestyle, label=label, *args, **kwargs)

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
    if kref:
        ax_set_locator_formatter(axes, kref.get_datetime_list(), kref.get_query().ktype)
    else:
        k = indicator.get_context()
        if len(k) > 0:
            ax_set_locator_formatter(axes, k.get_datetime_list(), k.get_query().ktype)
    # draw()


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
        axes = create_figure() if new else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    py_indicatr = [None if x == constant.null_price else x for x in indicator]
    x = [i - 0.2 for i in range(len(indicator))]
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
    if kref:
        ax_set_locator_formatter(axes, kref.get_datetime_list(), kref.get_query().ktype)
    else:
        k = indicator.get_context()
        if len(k) > 0:
            ax_set_locator_formatter(axes, k.get_datetime_list(), k.get_query().ktype)
    # draw()


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
    fmacd.plot(axes=axt, linestyle='--', legend_on=False, text_on=False)
    smacd.plot(axes=axt, legend_on=False, text_on=False)

    for label in axt.get_xticklabels():
        label.set_visible(False)


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
    fmacd.plot(axes=axt, linestyle='--', legend_on=False, text_on=False)
    smacd.plot(axes=axt, legend_on=False, text_on=False)

    for label in axt.get_xticklabels():
        label.set_visible(False)


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


def evplot(ev, ref_kdata, new=True, axes=None):
    """绘制市场有效判断

    :param EnvironmentBase cn: 系统有效条件
    :param KData ref_kdata: 用于日期参考
    :param new: 仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
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
    axes.fill_between(x, y1, y2, where=y2 > y1, facecolor='blue', alpha=0.6)
    axes.fill_between(x, y1, y2, where=y2 < y1, facecolor='red', alpha=0.6)


def cnplot(cn, new=True, axes=None, kdata=None):
    """绘制系统有效条件

    :param ConditionBase cn: 系统有效条件
    :param new: 仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
    :param KData kdata: 指定的KData，如该值为None，则认为该系统有效条件已经
                        指定了交易对象，否则，使用该参数作为交易对象
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
    axes.fill_between(x, y1, y2, where=y2 > y1, facecolor='blue', alpha=0.6)
    axes.fill_between(x, y1, y2, where=y2 < y1, facecolor='red', alpha=0.6)


def sysplot(sys, new=True, axes=None, style=1, only_draw_close=False):
    """绘制系统实际买入/卖出信号

    :param SystemBase sys: 系统实例
    :param new:   仅在未指定axes的情况下生效，当为True时，
                   创建新的窗口对象并在其中进行绘制
    :param axes:  指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式
    :param bool only_draw_close: 不绘制K线，仅绘制 close
    """
    kdata = sys.to

    refdates = kdata.get_datetime_list()
    date_index = dict([(d, i) for i, d in enumerate(refdates)])

    if axes is None:
        if new:
            axes = create_figure()
            if only_draw_close:
                iplot(kdata.close, axes=axes)
            else:
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


def sys_performance(sys, ref_stk=None):
    if ref_stk is None:
        ref_stk = get_stock('sh000300')

    query = sys.query
    sh000001_k = get_kdata('sh000001', query)
    ref_dates = sh000001_k.get_datetime_list()

    ref_k = ref_stk.get_kdata(query)

    funds_list = sys.tm.get_funds_list(ref_dates)
    funds = [f.total_assets for f in funds_list]
    funds = VALUE(funds)
    funds_return = [f.total_assets / f.total_base if f.total_base != 0.0 else constant.null_price for f in funds_list]
    funds_return = VALUE(funds_return, align_dates=ref_dates)
    funds_return.name = "系统累积收益率"
    ref_return = ALIGN(ROCR(ref_k.close, 0), ref_dates)
    ref_return.name = f"{ref_stk.name}({ref_stk.market_code})"

    per = Performance()
    text = per.report(sys.tm, sh000001_k[-1].datetime)

    # 计算最大回撤
    max_pullback = min(MDD(funds).to_np())

    # 计算 sharp
    bond = ZHBOND10(ref_dates)
    sigma = STDEV(ROCP(funds), len(ref_dates))
    sigma = 15.874507866387544 * sigma[-1]  # 15.874 = sqrt(252)
    sharp = (per['帐户平均年收益率%'] - bond[-1]) * 0.01 / sigma if sigma != 0.0 else 0.0

    invest_total = per['累计投入本金'] + per['累计投入资产']
    cur_fund = per['当前总资产']
    t1 = '投入总资产: {:<.2f}    当前总资产: {:<.2f}    当前盈利: {:<.2f}'.format(
        invest_total, cur_fund, cur_fund - invest_total)
    t2 = '当前策略收益: {:<.2f}%    年化收益率: {:<.2f}%    最大回撤: {:<.2f}%'.format(
        funds_return[-1]*100 - 100, per["帐户平均年收益率%"], max_pullback)
    t3 = '系统胜率: {:<.2f}%    盈/亏比: 1 : {:<.2f}    夏普比率: {:<.2f}'.format(
        per['赢利交易比例%'], per['净赢利/亏损比例'], sharp)

    import matplotlib.pyplot as plt
    fg = plt.figure(figsize=(15, 10))
    gs = fg.add_gridspec(5, 4)
    ax1 = fg.add_subplot(gs[:4, :3])
    ax2 = fg.add_subplot(gs[:, 3:])
    ax3 = fg.add_subplot(gs[4:, :3])
    ref_return.plot(axes=ax1, legend_on=True)
    funds_return.plot(axes=ax1, legend_on=True)
    if isinstance(sys, System):
        stk = sys.get_stock()
        ax1.set_title(f"{sys.name} {stk.name}({stk.market_code}) 累积收益率")
    else:
        ax1.set_title(f"{sys.name} 累积收益率")
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


# ============================================================================
# 通达信画图函数
# ============================================================================

DRAWNULL = constant.null_price


def RGB(r: int, g: int, b: int):
    hku_check(0 <= r <= 255 and 0 <= g <= 255 and 0 <= b <= 255, "r,g,b must in [0,255]!")
    return f"#{r:02x}{g:02x}{b:02x}"


def STICKLINE(cond: Indicator, price1: Indicator, price2: Indicator, width: float = 2.0,
              empty: bool = False, color='m', alpha=1.0, kdata=None, new=False, axes=None):
    """在满足cond的条件下，在 price1 和 price2 之间绘制一个宽度为 width 的柱状图。

    注意: cond, price1, price2 应含有数据，否则请指定 kdata 作为指标计算的上下文

    参数说明:
        cond (Indicator): 条件表达式，用于确定是否绘制柱状线
        price1 (Indicator): 第一个价格
        price2 (Indicator): 第二个价格
        width (float, optional): 柱状宽度. Defaults to 2.0.
        empty (bool, optional): 空心. Defaults to False.
        kdata (_type_, optional): 指定的上下文K线. Defaults to None.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 在指定的坐标轴中绘制. Defaults to None.
        color (str, optional): 颜色. Defaults to 'm'.
        alpha (float, optional): 透明度. Defaults to 1.0.
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
    """画出带状线

    用法:DRAWBAND(val1, color1, val2, color2), 当 val1 > val2 时,在 val1 和 val2 之间填充 color1;
    当 val1 < val2 时,填充 color2,这里的颜色均使用 matplotlib 颜色代码.
    例如:DRAWBAND(OPEN, 'r', CLOSE, 'b')

    Args:
        val1 (Indicator): 指标1
        color1 (str, optional): 颜色1. Defaults to 'm'.
        val2 (Indicator, optional): 指标2. Defaults to None.
        color2 (str, optional): 颜色2. Defaults to 'b'.
        kdata (_type_, optional): 指定指标上下文. Defaults to None.
        alpha (float, optional): 透明度. Defaults to 0.2.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 在指定的坐标轴中绘制. Defaults to None.
        linestyle (str, optional): 包络线类型. Defaults to '-'.
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
    """在图形上绘制折线段。

    用法：PLOYLINE(COND，PRICE)，当COND条件满足时，以PRICE位置为顶点画折线连接。
    例如：PLOYLINE(HIGH>=HHV(HIGH,20),HIGH, kdata=k)表示在创20天新高点之间画折线。

    Args:
        cond (Indicator): 指定条件
        price (Indicator): 位置
        kdata (KData, optional): 指定的上下文. Defaults to None.
        color (str, optional): 颜色. Defaults to 'b'.
        linewidth (float, optional): 宽度. Defaults to 1.0.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 指定的axes. Defaults to None.
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
    """在图形上绘制直线段。

    用法：DRAWLINE(cond1, price1, cond2, price2, expand)
    当COND1条件满足时，在PRICE1位置画直线起点，当COND2条件满足时，在PRICE2位置画直线终点，EXPAND为延长类型。
    例如：DRAWLINE(HIGH>=HHV(HIGH,20),HIGH,LOW<=LLV(LOW,20),LOW,1)表示在创20天新高与创20天新低之间画直线并且向右延长

    Args:
        cond1 (Indicator): 条件1
        price1 (Indicator): 位置1
        cond2 (Indicator): 条件2
        price2 (Indicator): 位置2
        expand (int, optional): 0: 不延长 | 1: 向右延长 | 10: 向左延长 | 11: 双向延长. Defaults to 0.
        kdata (KData, optional): 指定的上下文. Defaults to None.
        color (str, optional): 指定颜色. Defaults to 'm'.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 指定的坐标轴. Defaults to None.
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
    """在图形上显示文字。

    用法: DRAWTEXT(cond, price, text), 当 cond 条件满足时, 在 price 位置书写文字 text。
    例如: DRAWTEXT(CLOSE/OPEN>1.08,LOW,'大阳线')表示当日实体阳线大于8%时在最低价位置显示'大阳线'字样.

    Args:
        cond (Indicator): 条件
        price (Indicator): 显示位置
        text (str): 待显示文字
        kdata (KData, optional): 指定的上下文. Defaults to None.
        color (str, optional): 指定颜色. Defaults to 'm'.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 指定的坐标轴. Defaults to None.
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
    """固定位置显示文字

    用法:DRAWTEXT_FIX(cond,x y, text), cond 中一般需要加 ISLASTBAR,当 cond 条件满足时,
    在当前指标窗口内(X,Y)位置书写文字TEXT,X,Y为书写点在窗口中相对于左上角的百分比

    例如:DRAWTEXT_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08),0.5,0.5,0,'大阳线')表示最后一个交易日实体阳线
    大于8%时在窗口中间位置显示'大阳线'字样.

    Args:
        cond (Indicator): 条件
        x (float): x轴坐标
        y (float): y轴坐标
        type (int, optional): 0 左对齐 | 1 右对齐. 
        text (str): 待显示文字
        kdata (KData, optional): 指定的上下文. Defaults to None.
        color (str, optional): 指定颜色. Defaults to 'm'.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 指定坐标轴. Defaults to None.
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
    """画出数字.

    用法:DRAWNUMBER(cond, price, number),当 cond 条件满足时,在 price 位置书写数字 number.
    例如:DRAWNUMBER(CLOSE/OPEN>1.08,LOW,C)表示当日实体阳线大于8%时在最低价位置显示收盘价。

    Args:
        cond (Indicator): 条件
        price (Indicator): 绘制位置
        number (Indicator): 待绘制数字
        kdata (KData, optional): 指定的上下文. Defaults to None.
        color (str, optional): 指定颜色. Defaults to 'm'.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 指定的坐标轴. Defaults to None.
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
    """固定位置显示数字.

    用法:DRAWNUMBER_FIX(cond,x,y,type,number), cond 中一般需要加 ISLASTBAR, 当 cond 条件满足时,
    在当前指标窗口内 (x, y) 位置书写数字 number, x,y为书写点在窗口中相对于左上角的百分比,type:0为左对齐,1为右对齐。

    例如:DRAWNUMBER_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08), 0.5,0.5,0,C)表示最后一个交易日实体阳线大于8%时在窗口中间位置显示收盘价

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
    """绘制斜线.

    用法:DRAWSL(cond,price,slope,length,diect),当 cond 条件满足时,在 price 位置画斜线, slope 为斜率, 
    lengh为长度, direct 为0向右延伸,1向左延伸,2双向延伸。

    注意:
    1. K线间的纵向高度差为 slope;
    2. slope 为 0 时, 为水平线;
    3. slope 为 10000 时, 为垂直线, length 为向上的像素高度, direct 表示向上或向下延伸
    4. slope 和 length 支持变量;

    Args:
        cond (Indicator): 条件指标
        price (Indicator): 价格
        slope (int|float|Indicator): 斜率
        length (int|float|Indicator): 长度
        direct (int): 方向
        kdata (KData, optional): 指定的上下文. Defaults to None.
        color (str, optional): 颜色. Defaults to 'm'.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 指定的坐标轴. Defaults to None.
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
    """画图片

    用法:DRAWIMG(cond,price,'图像文件文件名'),当条件 cond 满足时,在 price 位置画指定的图片
    例如:DRAWIMG(O>C,CLOSE, '123.png')。

    Args:
        cond (Indicator): 指定条件
        price (Indicator): 指定价格
        img (str): 图像文件名
        kdata (KData, optional): 指定上下文. Defaults to None.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 在指定坐标轴中绘制. Defaults to None.
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
    pixel = 20.  # 显示像素大小
    w = xw / pw * pixel
    h = yh / ph * pixel
    for i in range(cond.discard, len(cond)):
        if cond[i] > 0.:
            axes.imshow(image, extent=[i-w, i+w, price[i]-h, price[i]+h], *args, **kwargs)

    axes.set_aspect('auto')
    axes.autoscale_view()
    axes.set_ylim(y0, y1)
    axes.set_xlim(-1, len(cond) + 1)


DRAWBMP = DRAWIMG


def DRAWICON(cond: Indicator, price: Indicator, type: int, kdata: KData = None, new=False, axes=None, *args, **kwargs):
    DRAWIMG(cond, price, f'{ICON_PATH}/icon/{type}.png', kdata, new, axes, *args, **kwargs)


def SHOWICONS():
    """显示所有内置图标"""
    axes = create_one_axes_figure([8, 6])
    p = axes.get_window_extent()
    pw = p.x1 - p.x0
    ph = p.y1 - p.y0
    x0, x1 = axes.get_xlim()
    y0, y1 = axes.get_ylim()
    xw = x1 - x0
    yh = y1 - y0
    pixel = 100.  # 显示像素大小
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
    """相对位置上画矩形.

    注意：原点为坐标轴左上角(0, 0)，和 matplotlib 不同。
    用法: DRAWRECTREL(left,top,right,bottom,color), 以图形窗口 (left, top) 为左上角, (right, bottom) 为
         右下角绘制矩形, 坐标单位是窗口沿水平和垂直方向的1/1000,取值范围是0—999,超出范围则可能显示在图形窗口外,矩形
         中间填充颜色COLOR,COLOR为0表示不填充.
    例如:DRAWRECTREL(0,0,500,500,RGB(255,255,0)) 表示在图形最左上部1/4位置用黄色绘制矩形

    Args:
        left (int): 左上角x
        top (int): 左上角y
        right (int): 右下角x
        bottom (int): 右下角y
        color (str, optional): 指定颜色. Defaults to 'm'.
        frame (bool, optional): 添加边框. Defaults to False.
        fill (bool, optional): 颜色填充. Defaults to True.
        alpha (float, optional): 透明度. Defaults to 0.1.
        new (bool, optional): 在新窗口中绘制. Defaults to False.
        axes (_type_, optional): 指定的坐标轴. Defaults to None.
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
