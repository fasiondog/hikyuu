#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20100224, Added by fasiondog
#===============================================================================

"""
绘制亚历山大.艾尔德交易系统图形
参见：《走进我的交易室》（2007年 地震出版社） Alexander Elder
"""
from pylab import plot
from numpy import mean

from hikyuu import QueryByIndex, constant
from hikyuu.indicator import Indicator, CLOSE, EMA, MACD, VIGOR, SAFTYLOSS
from hikyuu.interactive.drawplot import (create_three_axes_figure, 
                                         ax_draw_macd2, 
                                         adjust_axes_show,
                                         ax_set_locator_formatter)

def _find_ema_coefficient(closes, emas, number=66, percent=0.95):
    """计算EMA通道系数。
    在《走进我的交易室》中，艾尔德介绍的价格通道为：
        通道上轨 ＝ EMA ＋ EMA＊通道系数
        通道下轨 ＝ EMA － EMA＊通道系数
    其中一条绘制得恰到好处的通道应能将绝大多数价格包含在内，一般调节通道系数使其能够包含95％的价格
    参数：closes：收盘价序列
          emas：收盘价对应的EMA序列
          number: 以最近多少天的数据来计算，即取最后N天的数据作为计算标准
          percent：通道包含多少的价格，如0.95表示通道将包含95%的价格
    """
    assert len(closes)==len(emas), "数据长度不等"
    assert number>=1, "Number必须大于0"
    
    tmp_closes = closes[-number:]
    tmp_emas = emas[-number:]
    dev = [abs(tmp_closes[i]-tmp_emas[i]) for i in range(len(tmp_closes))]
    sortdev = sorted(dev)
    x = int(number*percent)
    x = x if len(dev)-1>x else len(dev)-1
    ix = dev.index(sortdev[x])
    return float(dev[ix])/tmp_emas[ix] if tmp_emas[ix] != 0.0 else 0.0

def _draw_ema_pipe(axes, kdata, ema, n=22, w=0.10):
    emas = [i for i in ema]
    p = _find_ema_coefficient(list(CLOSE(kdata)), emas, number=66, percent=0.95) if w=='auto' else w
    emas_high = [i+i*p for i in emas]
    emas_low = [i-i*p for i in emas]
    emas_len = len(emas)
    
    axes.plot(emas, '-b', label='%s(%s)  %.2f'%(ema.name, n, emas[-1]))
    axes.plot(emas_high, ':b', label='U%s(%.2f%%) %.2f'%(ema.name, p*100, emas_high[-1]))
    axes.plot(emas_low, ':b', label='L%s(%.2f%%) %.2f'%(ema.name, p*100, emas_low[-1]))
    fy1 = [ i for i in emas_low]
    fy2 = [ i for i in emas_high]
    axes.fill_between(range(emas_len),fy1,fy2,alpha=0.2, color='y' )

def draw(stock, query=QueryByIndex(-130), ma_n=22, ma_w='auto', vigor_n=13):
    kdata = stock.getKData(query)
    close = CLOSE(kdata)
    ema = EMA(close, ma_n)
    sf = SAFTYLOSS(close, 10, 3, 2.0)
    vigor = VIGOR(kdata, vigor_n)

    ax1, ax2, ax3 = create_three_axes_figure()
    kdata.plot(axes=ax1)
    _draw_ema_pipe(ax1,kdata, ema, n=ma_n, w=ma_w)
    sf.plot(axes=ax1, color='y', legend_on=True)
    #ax1.legend(loc='upper left')
    
    ax_draw_macd2(ax2, ema, kdata)
    
    vigor.plot(axes=ax3, marker='.', color='r', zero_on=True, 
               legend_on=False, text_on=True)
    u = [i for i in vigor if i > 0 and i != constant.null_price]
    l = [i for i in vigor if i < 0]
    umean = mean(u)
    umax = max(u)
    lmean = mean(l)
    lmin = min(l)
    up = int(umax / umean)
    lp = int(lmin / lmean)
    #ax3.hlines(umean,0,len(kdata),color='r',linestyle='--')    
    #ax3.hlines(lmean,0,len(kdata),color='r',linestyle='--')
    for i in range(up):
        ax3.hlines(umean * (i + 1),0,len(kdata),color='r',linestyle='--')
        
    for i in range(lp):
        ax3.hlines(lmean * (i + 1),0,len(kdata),color='g',linestyle='--')    
    
    ax1.set_xlim((0, len(kdata)))
    ax_set_locator_formatter(ax1, kdata.getDatetimeList(), kdata.getQuery().kType)
    adjust_axes_show([ax1, ax2, ax3])
       
