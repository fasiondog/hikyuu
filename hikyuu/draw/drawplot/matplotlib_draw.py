# -*- coding: utf8 -*-
# cp936
"""
交互模式下绘制相关图形，如K线图，美式K线图
"""
import sys
import datetime
import logging
import numpy as np
import matplotlib
from pylab import Rectangle, gca, figure, ylabel, axes, draw
from matplotlib import rcParams
from matplotlib.font_manager import FontManager, _log as fm_logger
from matplotlib.lines import Line2D, TICKLEFT, TICKRIGHT
from matplotlib.ticker import FuncFormatter, FixedLocator

from hikyuu import *

from .common import get_draw_title


def set_mpl_params():
    '''设置交互及中文环境参数'''
    if in_interactive_session():
        rcParams['interactive'] = True

    rcParams['font.family'] = 'sans-serif'
    rcParams['axes.unicode_minus'] = False

    expected_fonts = ['Microsoft YaHei', 'SimSun', 'SimHei', 'Source Han Sans CN', 'Noto Sans CJK JP']
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
    sep = len(dates) / 8
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
    axes.plot(py_indicatr, '-', label=label, *args, **kwargs)

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


def sysplot(sys, new=True, axes=None, style=1):
    """绘制系统实际买入/卖出信号

    :param SystemBase sys: 系统实例
    :param new:   仅在未指定axes的情况下生效，当为True时，
                   创建新的窗口对象并在其中进行绘制
    :param axes:  指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式
    """
    kdata = sys.to

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
    k = sys.to
    if k is None or k.empty():
        hku_info("sys.to is None or empty!")
        return

    query = Query(k[0].datetime.start_of_day(), k[-1].datetime.start_of_day() + TimeDelta(1), Query.DAY)

    if ref_stk is None:
        ref_stk = get_stock('sh000300')
    ref_k = ref_stk.get_kdata(query)
    ref_dates = ref_k.get_datetime_list()

    profit = sys.tm.get_profit_curve(ref_dates)
    profit = VALUE(profit)
    funds = sys.tm.get_funds_curve(ref_dates)
    funds = VALUE(funds)
    funds_return = profit / REF(funds, 1) + 1
    # funds_return = cum_return(funds)
    funds_return.name = "系统累积收益率"
    cum_return = get_part("default.ind.累积收益率")
    ref_return = cum_return(ref_k.close)
    ref_return.name = ref_stk.name

    per = Performance()
    text = per.report(sys.tm, Datetime(datetime.today()))

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
    t2 = '当前策略收益: {:<.2f}%    年化收益率: {:<.2}%    最大回撤: {:<.2f}%'.format(
        funds_return[-1], per["帐户平均年收益率%"], max_pullback)
    t3 = '系统胜率: {:<.2f}%    盈/亏比: 1 : {:<.2f}    夏普比率: {:<.2f}'.format(
        per['赢利交易比例%'], per['净赢利/亏损比例'], sharp)

    mp_back = matplotlib.get_backend()
    if "nbagg" in mp_back:
        fg = figure(figsize=(13, 10))
        ax1 = fg.add_axes([0.05, 0.35, 0.65, 0.6])
        ax2 = fg.add_axes([0.05, 0.05, 0.65, 0.25], sharex=ax1)

        ref_return.plot(axes=ax1, legend_on=True)
        funds_return.plot(axes=ax1, legend_on=True)

        label = t1 + '\n\n' + t2 + '\n\n' + t3
        ax1.text(1.01,
                 1,
                 text,
                 horizontalalignment='left',
                 verticalalignment='top',
                 transform=ax1.transAxes,
                 # color='r'
                 )
        ax2.text(0.02,
                 1.0,
                 label,
                 horizontalalignment='left',
                 verticalalignment='top',
                 transform=ax2.transAxes,
                 # color='r'
                 )
        # ax2.set_visible(False)
        ax2.xaxis.set_visible(False)
        ax2.yaxis.set_visible(False)
        ax2.set_frame_on(False)

    else:
        ref_return.plot(legend_on=True)
        funds_return.plot(legend_on=True, new=False)
        axis = gca()
        axis.text(-0.05,
                  0.97,
                  text,
                  horizontalalignment='right',
                  verticalalignment='top',
                  transform=axis.transAxes,
                  # color=text_color
                  )

        label = t1 + '\n\n' + t2 + '\n\n' + t3
        axis.text(0.05,
                  -0.06,
                  label,
                  horizontalalignment='left',
                  verticalalignment='top',
                  transform=axis.transAxes,
                  # color=text_color
                  )
