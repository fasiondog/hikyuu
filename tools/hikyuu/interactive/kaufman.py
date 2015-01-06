#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20100224, Added by fasiondog
#===============================================================================

"""
绘制佩里.J.考夫曼（Perry J.Kaufman） 自适应移动平均系统(AMA)
参见：《精明交易者》（2006年 广东经济出版社） 
"""

from hikyuu import Query
from hikyuu.indicator import (AMA, KDATA_PART, PRICELIST, SAFTYLOSS)
from hikyuu.trade_sys.signal import AMA_SG
from hikyuu.trade_manage import BUSINESS
from .drawplot import (create_two_axes_figure, ax_set_locator_formatter,
                      adjust_axes_show, ax_draw_signal, ax_draw_sys_signal)

def draw(stock, query = Query(-130), n = 10, fast_n = 2, slow_n = 30,
         filter_n = 20, filter_p = 0.1, arrow_style = 1, kpart = "CLOSE",
         sys = None):
    kdata = stock.getKData(query)
    ind = KDATA_PART(kdata, kpart)
    cama = AMA(ind, n, fast_n, slow_n)
    sp = SAFTYLOSS(ind, n)
    
    ax1, ax2 = create_two_axes_figure()
    kdata.plot(axes = ax1)
    cama.plot(axes = ax1, color = 'b', legend_on = True)
    sp.plot(axes = ax1, color = 'y', legend_on = True)
    
    sg = AMA_SG(n, fast_n, slow_n, filter_n, filter_p, kpart)
    sg.setTO(kdata)
    ax_draw_signal(ax1, kdata, sg.getBuySignal(), 'BUY', arrow_style)
    ax_draw_signal(ax1, kdata, sg.getSellSignal(), 'SELL', arrow_style)
    
    if sys:
        ax_draw_sys_signal(ax1, kdata, sys, 2)
    
    cer = PRICELIST(cama, 1, 0, len(cama))
    label = "ER(%s)" % cer[-1]
    cer.plot(axes=ax2, color='b', marker='o', label=label, 
             legend_on=False, text_on=True)
    ax2.hlines(0.8,0,len(kdata),color='r',linestyle='--')    
    ax2.hlines(0.4,0,len(kdata),color='r',linestyle='--')    
    ax2.hlines(-0.6,0,len(kdata),color='r',linestyle='--')
    ax2.hlines(-0.8,0,len(kdata),color='r',linestyle='--')
    ax2.hlines(0,0,len(kdata))
    
    ax1.set_xlim((0, len(kdata)))
    ax_set_locator_formatter(ax1, kdata.getDatetimeList(), query.kType)
    adjust_axes_show([ax1, ax2])
    
    