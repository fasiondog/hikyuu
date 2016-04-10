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

from hikyuu import Query, StockManager
from hikyuu.indicator import *
from hikyuu.trade_sys.signal import SG_Single, SG_Cross, SG_Flex
from hikyuu.trade_manage import BUSINESS
from .drawplot import (create_two_axes_figure, ax_set_locator_formatter,
                       create_three_axes_figure, adjust_axes_show, 
                       ax_draw_signal, ax_draw_sys_signal, ax_draw_macd)

def draw(stock, query = Query(-130), 
         n = 10, filter_n = 20, filter_p = 0.1,
         sg_type = "CROSS",
         show_high_low = False,
         arrow_style = 1):
    """
    """
    kdata = stock.getKData(query)
    
    ax1, ax2 = create_two_axes_figure()
    kdata.plot(axes = ax1)

    cama = AMA(CLOSE(kdata), n = n )
    cama.name = "CAMA"
    cama.plot(axes = ax1, color = 'b', legend_on = True)
    
    hama = AMA(HIGH(kdata), n = n)
    hama.name = "HAMA"
    hstd = STDEV(hama, n)
    lama = AMA(LOW(kdata), n = n)
    lama.name = "LAMA"
    lstd = STDEV(lama, n)
    fy1 = list(lama - lstd)[lstd.discard:]
    fy2 = list(hama + hstd)[hstd.discard:]
    ax1.fill_between(range(lstd.discard,len(kdata)),fy1,fy2,alpha=0.2, color='y' )

    if show_high_low:
        hama.plot(axes = ax1, color = 'r', legend_on = True)
        lama.plot(axes = ax1, color = 'g', legend_on = True)
    
    if sg_type == 'CROSS':
        fast_op = OP(OP(AMA(n = n)), OP(CLOSE()))
        slow_op = OP(OP(EMA(n = 2*n)), fast_op)
        sg = SG_Cross(fast_op, slow_op)
        sg.plot(axes = ax1, kdata = kdata)
        ind = slow_op(KDATA(kdata))
        ind.name = "EMA(CAMA)"
        ind.plot(axes = ax1, color = 'm', legend_on = True)
                 
    elif sg_type == 'SINGLE':
        sg = SG_Single(cama, filter_n = filter_n, filter_p = filter_p)
        sg.plot(axes = ax1, kdata = kdata)

    else:
        print("sg_type only in ('CORSS', 'SINGLE')")
 
    cer = PRICELIST(cama, 1)
    label = "ER(%s)" % cer[-1]
    cer.plot(axes=ax2, color='b', marker='o', label=label, 
             legend_on=False, text_on=True)
    ax2.hlines(0.8,0,len(kdata),color='r',linestyle='--')    
    ax2.hlines(-0.6,0,len(kdata),color='r',linestyle='--')
    ax2.hlines(-0.8,0,len(kdata),color='r',linestyle='--')
    ax2.hlines(0,0,len(kdata))
    
    ax1.set_xlim((0, len(kdata)))
    ax_set_locator_formatter(ax1, kdata.getDatetimeList(), query.kType)
    adjust_axes_show([ax1, ax2])
 

def draw2(block, query = Query(-130), 
         ama1 = AMA(n=10, fast_n=2, slow_n=30),
         ama2 = None,
         n = 10, filter_n = 20, filter_p = 0.1,
         sg_type = 'CROSS',
         show_high_low = True,
         arrow_style = 1):
    sm = StockManager.instance()
    if block.name == 'SZ':
        kdata = sm['sz000001'].getKData(query)
    elif block.name == 'GEM':
        kdata = sm['sz399006'].getKData(query)
    else:
        kdata = sm['sh000001'].getKData(query)
    
    ax1, ax2, ax3 = create_three_axes_figure()
    kdata.plot(axes = ax1)

    cama = AMA(CLOSE(kdata), n = n )
    cama.name = "CAMA"
    cama.plot(axes = ax1, color = 'b', legend_on = True)

    hama = AMA(HIGH(kdata), n = n)
    hama.name = "HAMA"
    hstd = STDEV(hama, n)
    lama = AMA(LOW(kdata), n = n)
    lama.name = "LAMA"
    lstd = STDEV(lama, n)
    fy1 = list(lama - lstd)[lstd.discard:]
    fy2 = list(hama + hstd)[hstd.discard:]
    ax1.fill_between(range(lstd.discard,len(kdata)),fy1,fy2,alpha=0.2, color='y' )
            
    if show_high_low:
        hama.plot(axes = ax1, color = 'r', legend_on = True)
        lama.plot(axes = ax1, color = 'g', legend_on = True)
    
    if sg_type == 'CROSS':
        fast_op = OP(OP(AMA(n = n)), OP(CLOSE()))
        slow_op = OP(OP(EMA(n = 2*n)), fast_op)
        sg = SG_Cross(fast_op, slow_op)
        sg.plot(axes = ax1, kdata = kdata)
        ind = slow_op(KDATA(kdata))
        ind.name = "EMA(CAMA)"
        ind.plot(axes = ax1, color = 'm', legend_on = True)
                 
    elif sg_type == 'SINGLE':
        sg = SG_Single(cama, filter_n = filter_n, filter_p = filter_p)
        sg.plot(axes = ax1, kdata = kdata)

    else:
        print("sg_type only in ('CORSS', 'SINGLE')")
        
    a = POS(block, query, SG_Flex(AMA(n=3)))
    a.name = "POS(3)"
    a.plot(axes=ax2, color='b', marker='.', legend_on=True)
    a = POS(block, query, SG_Flex(AMA(n=30)))
    a.name = "POS(30)"
    a.plot(axes=ax2, color='g', marker='.', legend_on=True)
    ax2.hlines(0.8,0,len(kdata),color='r',linestyle='--')    
    ax2.hlines(0.2,0,len(kdata),color='r',linestyle='--')
    
    if ama1.name == "AMA":
        cer = PRICELIST(cama, 1)
        label = "ER(%s)" % cer[-1]
        cer.plot(axes=ax3, color='b', marker='.', label=label, 
                 legend_on=False, text_on=True)
        ax3.hlines(0.8,0,len(kdata),color='r',linestyle='--')    
        ax3.hlines(-0.6,0,len(kdata),color='r',linestyle='--')
        ax3.hlines(-0.8,0,len(kdata),color='r',linestyle='--')
        ax3.hlines(0,0,len(kdata))
    else:
        ax_draw_macd(ax2, kdata)
    #ax2.set_ylim(-1, 1)
    
    ax1.set_xlim((0, len(kdata)))
    ax_set_locator_formatter(ax1, kdata.getDatetimeList(), query.kType)
    adjust_axes_show([ax1, ax2])   
    