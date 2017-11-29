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
# 1. 20100227, Added by fasiondog
#===============================================================================

"""
绘制普通K线图 + 成交量（成交金额）
"""
from hikyuu import Query
from hikyuu.util.mylog import escapetime
from hikyuu.indicator import Indicator, MA, CLOSE, VOL, OP, CVAL, PRICELIST
from hikyuu.trade_sys.signal import SG_Cross
from hikyuu.interactive.drawplot import (create_figure, 
                                         get_current_draw_engine,
                                         ax_set_locator_formatter, 
                                         adjust_axes_show,
                                         ax_draw_macd,
                                         show_gcf)

def draw(stock, query=Query(-130), ma1_n=5, ma2_n=10, ma3_n=20, ma4_n=60, 
         ma5_n=100, ma_type="SMA", vma1_n=5, vma2_n=10):
    """绘制普通K线图 + 成交量（成交金额）"""
    kdata = stock.getKData(query)
    close = CLOSE(kdata,)
    ma1 = MA(close, ma1_n, ma_type)
    ma2 = MA(close, ma2_n, ma_type)
    ma3 = MA(close, ma3_n, ma_type)
    ma4 = MA(close, ma4_n, ma_type)
    ma5 = MA(close, ma5_n, ma_type)

    ax1, ax2 = create_figure(2)
    kdata.plot(axes=ax1)
    ma1.plot(axes=ax1, legend_on=True)
    ma2.plot(axes=ax1, legend_on=True)
    ma3.plot(axes=ax1, legend_on=True)
    ma4.plot(axes=ax1, legend_on=True)
    ma5.plot(axes=ax1, legend_on=True)
    
    sg = SG_Cross(OP(MA(n=ma1_n, type=ma_type)), OP(MA(n=ma2_n, type=ma_type)))
    sg.setTO(kdata)
    sg.plot(axes=ax1, kdata=kdata)
    
    vol = VOL(kdata)
    total = len(kdata)
    
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        rg = range(total)
        x = [i-0.2 for i in rg]
        x1 = [x[i] for i in rg if kdata[i].closePrice > kdata[i].openPrice]
        y1 = [vol[i] for i in rg if kdata[i].closePrice > kdata[i].openPrice]
        x2 = [x[i] for i in rg if kdata[i].closePrice <= kdata[i].openPrice]
        y2 = [vol[i] for i in rg if kdata[i].closePrice <= kdata[i].openPrice]
        ax2.bar(x1, y1, width=0.4, color='r', edgecolor='r')
        ax2.bar(x2, y2, width=0.4, color='g', edgecolor='g')
    
    elif engine == 'echarts':
        vol.bar(axes=ax2, color='r')
    else:
        pass
    
    vma1 = MA(vol, vma1_n)
    vma2 = MA(vol, vma2_n)
    vma1.plot(axes=ax2, legend_on=True)
    vma2.plot(axes=ax2, legend_on=True)
    
    if query.kType == Query.WEEK and stock.market == 'SH' and stock.code=='000001':
        CVAL(0.16e+009, total, color='b',linestyle='--')
        #ax2.hlines(0.16e+009,0,len(kdata),color='b',linestyle='--')

    ax_set_locator_formatter(ax1, kdata.getDatetimeList(), kdata.getQuery().kType)
    adjust_axes_show([ax1, ax2])
    return show_gcf()


def draw2(stock, query=Query(-130), ma1_n=7, ma2_n=20, ma3_n=30, 
          ma4_n=42, ma5_n=100, vma1_n=5, vma2_n=10):
    """绘制普通K线图 + 成交量（成交金额）+ MACD"""
    kdata = stock.getKData(query)
    close = CLOSE(kdata)
    ma1 = MA(close, ma1_n)
    ma2 = MA(close, ma2_n)
    ma3 = MA(close, ma3_n)
    ma4 = MA(close, ma4_n)
    ma5 = MA(close, ma5_n)

    ax1, ax2, ax3 = create_figure(3)
    kdata.plot(axes=ax1)
    ma1.plot(axes=ax1, legend_on=True)
    ma2.plot(axes=ax1, legend_on=True)
    ma3.plot(axes=ax1, legend_on=True)
    ma4.plot(axes=ax1, legend_on=True)
    ma5.plot(axes=ax1, legend_on=True)
    
    vol = VOL(kdata)
    total = len(kdata)
    
    engine = get_current_draw_engine()
    if engine == 'matplotlib':
        rg = range(total)
        x = [i-0.2 for i in rg]
        x1 = [x[i] for i in rg if kdata[i].closePrice > kdata[i].openPrice]
        y1 = [vol[i] for i in rg if kdata[i].closePrice > kdata[i].openPrice]
        x2 = [x[i] for i in rg if kdata[i].closePrice < kdata[i].openPrice]
        y2 = [vol[i] for i in rg if kdata[i].closePrice < kdata[i].openPrice]
        ax2.bar(x1, y1, width=0.4, color='r', edgecolor='r')
        ax2.bar(x2, y2, width=0.4, color='g', edgecolor='g')
   
    elif engine == 'echarts':
        vol.bar(axes=ax2, color='r', legend_on=True)
    else:
        pass
        
    vma1 = MA(vol, vma1_n)
    vma2 = MA(vol, vma2_n)
    vma1.plot(axes=ax2, legend_on=True)
    vma2.plot(axes=ax2, legend_on=True)
    
    ax_draw_macd(ax3, kdata)

    ax1.set_xlim((0, len(kdata)))
    ax_set_locator_formatter(ax1, kdata.getDatetimeList(), kdata.getQuery().kType)
    adjust_axes_show([ax1, ax2, ax3])
    return show_gcf()
    