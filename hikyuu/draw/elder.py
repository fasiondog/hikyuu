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
# History:
# 1. 20100224, Added by fasiondog
#===============================================================================
"""
Draw the trading system chart of Alexander Elder
See: 《走进我的交易室》（2007年 地震出版社） Alexander Elder
"""
from pylab import plot
from numpy import mean

from hikyuu import (Query, constant, Indicator, CLOSE, EMA, MACD, VIGOR, SAFTYLOSS, CVAL, PRICELIST, htr)
from .drawplot import (
    create_figure, show_gcf, ax_draw_macd2, adjust_axes_show, ax_set_locator_formatter
)


def _find_ema_coefficient(closes, emas, number=66, percent=0.95):
    """Calculate the coefficient of the EMA channel.

    In 《走进我的交易室》（2007年 地震出版社）, the price channel introduced by Elder is:

        the upper rail of the channel = EMA + EMA * the channel coefficient
        the lower rail of the channel = EMA - EMA * the channel coefficient

    A well-drawn channel should contain the vast majority of the prices, and the coefficient is
    generally adjusted so that the channel contains 95% of the prices.

    :param closes: the close price sequence
    :param emas: the EMA sequence corresponding to the close prices
    :param number: how many recent days are used for the calculation, i.e. the last N days
    :param percent: the ratio of the prices contained in the channel, such as 0.95 for 95%
    """
    assert len(closes) == len(emas), htr("The lengths of the two sequences are not equal")
    assert number >= 1, htr("Number must be greater than 0")

    tmp_closes = closes[-number:]
    tmp_emas = emas[-number:]
    dev = [abs(tmp_closes[i] - tmp_emas[i]) for i in range(len(tmp_closes))]
    sortdev = sorted(dev)
    x = int(number * percent)
    x = x if len(dev) - 1 > x else len(dev) - 1
    ix = dev.index(sortdev[x])
    return float(dev[ix]) / tmp_emas[ix] if tmp_emas[ix] != 0.0 else 0.0


def _draw_ema_pipe(axes, kdata, ema, n=22, w=0.10):
    emas = [i for i in ema]
    p = _find_ema_coefficient(
        list(CLOSE(kdata)), emas, number=66, percent=0.95
    ) if w == 'auto' else w
    emas_high = [i + i * p for i in emas]
    emas_low = [i - i * p for i in emas]
    emas_len = len(emas)

    PRICELIST(emas).plot(
        axes=axes, color='b', linestyle='-', label='%s(%s)  %.2f' % (ema.name, n, emas[-1]), kref=kdata
    )
    PRICELIST(emas_high).plot(
        axes=axes,
        color='b',
        linestyle=':',
        label='U%s(%.2f%%) %.2f' % (ema.name, p * 100, emas_high[-1]),
        kref=kdata
    )
    PRICELIST(emas_low).plot(
        axes=axes,
        color='b',
        linestyle=':',
        label='L%s(%.2f%%) %.2f' % (ema.name, p * 100, emas_low[-1]),
        kref=kdata
    )
    #axes.plot(emas, '-b', label='%s(%s)  %.2f'%(ema.name, n, emas[-1]))
    #axes.plot(emas_high, ':b', label='U%s(%.2f%%) %.2f'%(ema.name, p*100, emas_high[-1]))
    #axes.plot(emas_low, ':b', label='L%s(%.2f%%) %.2f'%(ema.name, p*100, emas_low[-1]))
    fy1 = [i for i in emas_low]
    fy2 = [i for i in emas_high]
    axes.fill_between(range(emas_len), fy1, fy2, alpha=0.2, color='y')


def draw(stock, query=Query(-130), ma_n=22, ma_w='auto', vigor_n=13):
    """Draw the trading system chart of Alexander Elder"""
    kdata = stock.get_kdata(query)
    close = CLOSE(kdata)
    ema = EMA(close, ma_n)
    sf = SAFTYLOSS(close, 10, 3, 2.0)
    vigor = VIGOR(kdata, vigor_n)

    ax1, ax2, ax3 = create_figure(3)
    kdata.plot(axes=ax1)
    _draw_ema_pipe(ax1, kdata, ema, n=ma_n, w=ma_w)
    sf.plot(axes=ax1, color='y', legend_on=True, kref=kdata)

    ax_draw_macd2(ax2, ema, kdata)

    vigor.plot(axes=ax3, marker='.', color='r', zero_on=True, legend_on=False, text_on=True, kref=kdata)
    u = [i for i in vigor if i > 0 and i != constant.null_price]
    l = [i for i in vigor if i < 0]
    umean = mean(u)
    umax = max(u)
    lmean = mean(l)
    lmin = min(l)
    up = int(umax / umean)
    lp = int(lmin / lmean)
    for i in range(up):
        CVAL(close, umean * (i + 1)).plot(axes=ax3, color='r', linestyle='--', kref=kdata)

    for i in range(lp):
        CVAL(close, lmean * (i + 1)).plot(axes=ax3, color='g', linestyle='--', kref=kdata)

    ax1.set_xlim((0, len(kdata)))
    ax_set_locator_formatter(ax1, kdata.get_datetime_list(), kdata.get_query().ktype)
    adjust_axes_show([ax1, ax2, ax3])
    return show_gcf()
