# -*- coding: utf8 -*-
# cp936
"""
交互模式下绘制相关图形，如K线图，美式K线图
"""
import sys
import datetime
import numpy as np
import matplotlib
from pylab import Rectangle, gca, figure, ylabel, axes, draw
from matplotlib.lines import Line2D, TICKLEFT, TICKRIGHT
from matplotlib.ticker import FuncFormatter, FixedLocator

from hikyuu import *
from hikyuu.indicator import Indicator, MACD, CLOSE
from hikyuu.trade_manage import BUSINESS 
from hikyuu.trade_sys.system import getSystemPartName

def create_one_axes_figure(figsize=(10,6)):                                        
    """生成一个含有1个坐标轴的figure，并返回坐标轴列表
    返回：(ax1,ax2)
    """
    rect1  = [0.05, 0.05, 0.9, 0.90]
    fg=figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    return ax1

def create_two_axes_figure(figsize=(10,8)):                                        
    """生成一个含有2个坐标轴的figure，并返回坐标轴列表
    返回：(ax1,ax2)
    """
    rect1  = [0.05, 0.35, 0.9, 0.60]
    rect2  = [0.05, 0.05, 0.9, 0.30]
    
    fg=figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    ax2 = fg.add_axes(rect2, sharex=ax1)
    
    return ax1,ax2

def create_three_axes_figure(figsize=(10,8)):
    """生成一个含有3个坐标轴的figure，并返回坐标轴列表
    返回：(ax1,ax2,ax3)
    """
    rect1  = [0.05, 0.45, 0.9, 0.50]
    rect2  = [0.05, 0.25, 0.9, 0.20]
    rect3 = [0.05, 0.05, 0.9, 0.20]
    
    fg=figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    ax2 = fg.add_axes(rect2, sharex=ax1)
    ax3 = fg.add_axes(rect3, sharex=ax1)
    
    return ax1,ax2,ax3

def create_four_axes_figure(figsize=(10,8)):
    """生成一个含有4个坐标轴的figure，并返回坐标轴列表
    返回：(ax1,ax2,ax3, ax4)
    """
    rect1  = [0.05, 0.50, 0.9, 0.45]
    rect2  = [0.05, 0.35, 0.9, 0.15]
    rect3 = [0.05, 0.20, 0.9, 0.15]
    rect4 = [0.05, 0.05, 0.9, 0.15]
    
    fg=figure(figsize=figsize)
    ax1 = fg.add_axes(rect1)
    ax2 = fg.add_axes(rect2, sharex=ax1)
    ax3 = fg.add_axes(rect3, sharex=ax1)
    ax4 = fg.add_axes(rect4, sharex=ax1)
    
    return ax1,ax2,ax3,ax4

def create_axes_figure(n = 1, figsize = (10,8)):
    """生成指定轴个数的窗口
    参数：
        n：坐标轴个数，只支持1~4个坐标轴，默认为1
        figsize：窗口大小，默认(10,8)
    返回：
    (ax1, ax2, ...) 根据指定的坐标轴数量而定，超出[1,4]个坐标轴时，返回None
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
        
    def __call__(self, x, pos=None): #IGNORE:W0613
        result = ''
        ix = int(x)
        if ix in self.__ix2date:
            result = self.__ix2date[ix]
        return result

def getDayLocatorAndFormatter(dates):
    """获取显示日线时使用的Major Locator和Major Formatter"""
    premonth, preyear = dates[0].month, dates[0].year
    month = []
    loc = []
    for i,d in enumerate(dates):
        curmonth, curyear = d.month, d.year
        if curyear>preyear:
            strloc = str(curmonth)
            loc.append(i)
            month.append((i, strloc))
            preyear = curyear
            premonth = curmonth
        elif curmonth>premonth:
            strloc = str(curmonth)
            loc.append(i)
            month.append((i,strloc[-2:]))
            premonth = curmonth
        else:
            month.append((i, str(d.number)))
    
    month_loc = FixedLocator(loc)
    month_fm = FuncFormatter(StockFuncFormatter(dict(month)))
    return month_loc, month_fm

def getWeekLocatorAndFormatter(dates):
    """获取显示周线线时使用的Major Locator和Major Formatter"""
    premonth, preyear = dates[0].month, dates[0].year
    month = []
    loc = []
    for i,d in enumerate(dates):
        curmonth, curyear = d.month, d.year
        if curyear>preyear:
            strloc = str(curmonth)
            loc.append(i)
            month.append((i, strloc))
            preyear = curyear
            premonth = curmonth
        elif curmonth>premonth+2:
            strloc = str(curmonth)
            loc.append(i)
            month.append((i,strloc[-2:]))
            premonth = curmonth
        else:
            month.append((i, str(d.number)))    

    month_loc = FixedLocator(loc)
    month_fm = FuncFormatter(StockFuncFormatter(dict(month)))
    return month_loc, month_fm

def getMonthLocatorAndFormatter(dates):
    """获取显示月线时使用的Major Locator和Major Formatter"""
    preyear = dates[0].year
    month = []
    loc = []
    for i,d in enumerate(dates):
        curyear = d.year
        if curyear>preyear:
            month.append((i, curyear))
            loc.append(i)
            preyear = curyear
        else:
            month.append((i, str(d.number)))

    month_loc = FixedLocator(loc)
    month_fm = FuncFormatter(StockFuncFormatter(dict(month)))
    return month_loc, month_fm

def get_draw_title(kdata):
    """根据typ值，返回相应的标题，如 上证指数（日线）
    参数：kdata: KData实例
    返回：一个包含stock名称的字符串，可用作绘图时的标题
    """
    if not kdata:
        return ""

    query = kdata.getQuery()
    stock = kdata.getStock()
    if stock.isNull():
        return ""
    
    s1 = ''
    if query.kType == KQuery.KType.DAY:
        s1 = u' （日线）'
    elif query.kType == KQuery.KType.WEEK:
        s1 = u' （周线）'
    elif query.kType == KQuery.KType.MONTH:
        s1 = u' （月线）'
    elif query.kType == KQuery.KType.QUARTER:
        s1 = u' （季线）'
    elif query.kType == KQuery.KType.HALFYEAR:
        s1 = u' （半年线）'
    elif query.kType == KQuery.KType.YEAR:
        s1 = u' （年线）'
    elif query.kType == KQuery.KType.MIN:
        s1 = u' （1分钟线）'
    elif query.kType == KQuery.KType.MIN5:
        s1 = u' （5分钟线）'
    elif query.kType == KQuery.KType.MIN15:
        s1 = u' （15分钟线）'
    elif query.kType == KQuery.KType.MIN30:
        s1 = u' （30分钟线）'
    elif query.kType == KQuery.KType.MIN60:
        s1 = u' （60分钟线）'

    if IS_PY3:
        name = stock.name
    else:        
        if sys.platform == 'win32':
            name = stock.name.decode('gb2312')
        else:
            name = stock.name.decode('utf8')

    if stock.code == "":
        stitle = "Block(%s) %s" % (stock.id, name) + s1
    else:
        stitle = stock.market + stock.code + ' ' + name + s1

    return stitle

def ax_set_locator_formatter(axes, dates, typ):
    """设置指定坐标轴的日期显示
    参数：axes：指定的坐标轴
          dates: 日期序列[date1, date2, ...]
          typ: 同Stock.DATA()中的参数typ
    """
    major_loc, major_fm = None, None
    if typ == KQuery.KType.DAY:
        major_loc, major_fm = getDayLocatorAndFormatter(dates)
    elif typ == KQuery.KType.WEEK:
        major_loc, major_fm = getWeekLocatorAndFormatter(dates)
    elif typ == KQuery.KType.MONTH:
        major_loc, major_fm = getMonthLocatorAndFormatter(dates)
    elif typ == KQuery.KType.QUARTER:
        major_loc, major_fm = getMonthLocatorAndFormatter(dates)
    elif typ == KQuery.KType.HALFYEAR:
        major_loc, major_fm = getMonthLocatorAndFormatter(dates)
    elif typ == KQuery.KType.YEAR:
        major_loc, major_fm = getMonthLocatorAndFormatter(dates)

    if major_loc:
        axes.xaxis.set_major_locator(major_loc)
        axes.xaxis.set_major_formatter(major_fm)
    
def adjust_axes_show(axeslist):
    """
    用于调整上下紧密相连的坐标轴显示时，其上一坐标轴最小值刻度和下一坐标轴最大值刻度显示重叠的问题
    参数：axeslist：上下相连的坐标轴列表 (ax1,ax2,ax3)
    """
    for ax in axeslist[:-1]:
        for label in ax.get_xticklabels():
            label.set_visible(False)
        ylabels = ax.get_yticklabels()
        ylabels[0].set_visible(False)
        
        
def ax_draw_signal(axes, kdata, dates, direct="BUY", style = 1):
    """
    """
    refdates = kdata.getDatetimeList()
    date_index = dict([(d,i) for i,d in enumerate(refdates)])
    ylim = axes.get_ylim()
    height = ylim[1]-ylim[0]
    
    if style == 1:
        arrow = dict(arrowstyle="->")
    else:
        if direct == "BUY": 
            arrow = dict(facecolor='red', frac=0.5)
        else:
            arrow = dict(facecolor='blue', frac=0.5)
    
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        if direct == "BUY":
            axes.annotate('B', 
                          (pos, krecord.lowPrice - height*0.01), 
                          (pos, krecord.lowPrice - height*0.1), 
                          arrowprops = arrow,
                          horizontalalignment = 'center', 
                          verticalalignment = 'bottom',
                          color='red')
        else:
            axes.annotate('S', 
                          (pos, krecord.highPrice + height*0.01), 
                          (pos, krecord.highPrice + height*0.1), 
                          arrowprops = arrow,
                          horizontalalignment = 'center', 
                          verticalalignment = 'top',
                          color='blue')
            
        #axes.plot([pos],[krecord.closePrice],'rh')        
        

def ax_draw_sys_signal(axes, kdata, sys, style = 2):
    """
    """
    refdates = kdata.getDatetimeList()
    date_index = dict([(d, i) for i,d in enumerate(refdates)])
    ylim = axes.get_ylim()
    height = ylim[1]-ylim[0]
    
    if style == 1:
        arrow_buy = dict(arrowstyle="->")
        arrow_sell = arrow_buy
    else:
        arrow_buy = dict(facecolor='red', frac=0.5)
        arrow_sell = dict(facecolor='blue', frac=0.5)
    
    buy_request = sys.getBuyTradeRequest()
    sell_request = sys.getSellTradeRequest()
    text_request = ''
    if buy_request.valid:
        text_request = u'建议买入  止损： %.2f' % (buy_request.stoploss)
        color = 'r'
    if sell_request.valid:
        text_request = u'建议卖出  来源: %s' % (getSystemPartName(sell_request.part))
        color = 'b'
        
    if buy_request.valid or sell_request.valid:
        axes.text(0.99,0.03, text_request, horizontalalignment='right', 
                  verticalalignment='bottom', 
                  transform=axes.transAxes, color=color)
    
    dates = sys.getTradeRecordList()
    for d in dates:
        if not date_index.has_key(d.datetime):
            continue
        pos = date_index[d.datetime]
        krecord = kdata[pos]
        if d.business == BUSINESS.BUY:
            axes.annotate('SG', 
                          (pos, krecord.lowPrice - height*0.01), 
                          (pos, krecord.lowPrice - height*0.1), 
                          arrowprops = arrow_buy,
                          horizontalalignment = 'center', 
                          verticalalignment = 'bottom',
                          color='red')
        elif d.business == BUSINESS.SELL:
            text = getSystemPartName(d.part)
            axes.annotate(text, 
                          (pos, krecord.highPrice + height*0.01), 
                          (pos, krecord.highPrice + height*0.1), 
                          arrowprops = arrow_sell,
                          horizontalalignment = 'center', 
                          verticalalignment = 'top',
                          color='blue')
        else:
            None
            
        #axes.plot([pos],[krecord.closePrice],'rh')           
            

def kplot(kdata, width=0.6, colorup='r', colordown='g', alpha=1.0, axes=None, new=True):
    """
    绘制K线图，x轴使用数据的自然索引做下标，并不使用quotes中time作为x轴
    kdata     : KData
    width     : fraction of a day for the rectangle width
    colorup   : the color of the rectangle where close >= open
    colordown : the color of the rectangle where close <  open
    alpha     : the rectangle alpha level, 透明度(0.0~1.0) 1.0为不透明
    axes      : 指定的坐标轴
    new       : 是否在新窗口中显示，只在没有指定axes时生效
    """
    if not kdata:
        print("kdata is None")
        return
    
    if not axes:
        axes = create_one_axes_figure() if new else gca()
        
    OFFSET = width/2.0
    rfcolor = matplotlib.rcParams['axes.facecolor']
    for i in range(len(kdata)):
        record = kdata[i]
        open, high, low, close = record.openPrice, record.highPrice, record.lowPrice, record.closePrice
        if close>=open:
            color = colorup
            lower = open
            height = close-open
            rect = Rectangle(xy=(i-OFFSET, lower), width=width, height=height, facecolor=rfcolor, edgecolor=color)
        else:
            color = colordown
            lower = close
            height = open-close
            rect = Rectangle(xy=(i-OFFSET, lower), width=width, height=height, facecolor=color, edgecolor=color)

        vline1 = Line2D(xdata=(i, i), ydata=(low, lower), color=color, linewidth=0.5, antialiased=True)
        vline2 = Line2D(xdata=(i, i), ydata=(lower+height, high), color=color, linewidth=0.5, antialiased=True)
        rect.set_alpha(alpha)

        axes.add_line(vline1)
        axes.add_line(vline2)
        axes.add_patch(rect)
        
    title = get_draw_title(kdata)
    axes.set_title(title)  
    last_record = kdata[-1]
    color = 'r' if last_record.closePrice>kdata[-2].closePrice else 'g'
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f 涨幅:%.2f%%' % (
        last_record.datetime.number/10000, 
        last_record.openPrice, last_record.highPrice,
        last_record.lowPrice,  last_record.closePrice,
        100*(last_record.closePrice-kdata[-2].closePrice)/kdata[-2].closePrice)
    axes.text(0.99,0.97, text, horizontalalignment='right', verticalalignment='top', 
              transform=axes.transAxes, color=color)
        
    axes.autoscale_view()
    axes.set_xlim(-1, len(kdata)+1)
    ax_set_locator_formatter(axes, kdata.getDatetimeList(), kdata.getQuery().kType)
    #draw()
    
def mkplot(kdata, ticksize=3, colorup='r', colordown='g', axes=None, new=True):
    """绘制美式K线图，x轴使用数据的自然索引做下标，并不使用quotes中的time
    kdata       : KData实例
    ticksize    : open/close tick marker in points
    colorup     : the color of the lines where close >= open
    colordown   : the color of the lines where close <  open
    axes        : 指定的坐标轴
    new       : 是否在新窗口中显示，只在没有指定axes时生效
    """
    if not kdata:
        print("kdata is None")
        return

    if not axes:
        axes = create_one_axes_figure() if new else gca()
        
    for t in range(len(kdata)):
        record = kdata[t]
        open, high, low, close = record.openPrice, record.highPrice, record.lowPrice, record.closePrice 
        color = colorup if close>=open else colordown

        vline = Line2D(xdata=(t, t), ydata=(low, high), color=color, antialiased=False)
        oline = Line2D(xdata=(t, t), ydata=(open, open), color=color, antialiased=False,
                       marker=TICKLEFT, markersize=ticksize)
        cline = Line2D(xdata=(t, t), ydata=(close, close), color=color, antialiased=False,
                       markersize=ticksize, marker=TICKRIGHT)

        axes.add_line(vline)
        axes.add_line(oline)
        axes.add_line(cline)
        
    title = get_draw_title(kdata)
    axes.set_title(title)  
    last_record = kdata[-1]
    color = 'r' if last_record.closePrice>kdata[-2].closePrice else 'g'
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f' % (last_record.datetime.number/10000, 
                                                last_record.openPrice, 
                                                last_record.highPrice,
                                                last_record.lowPrice, 
                                                last_record.closePrice)
    axes.text(0.99,0.97, text, horizontalalignment='right', verticalalignment='top', 
              transform=axes.transAxes, color=color)
        
    axes.autoscale_view()
    #axes.set_xlim(0, len(kdata))
    ax_set_locator_formatter(axes, kdata.getDatetimeList(), kdata.getQuery().kType)
    #draw()

def iplot(indicator, axes=None, new=True, legend_on=False, text_on=False, text_color='k',  
                   zero_on=False, label=None, *args, **kwargs):
    """绘制indicator曲线
    indicator: indicator实例
    axes: 指定的坐标轴
    new: 是否在新窗口中显示，只在没有指定axes时生效
    legend_on: 是否打开图例
    text_on: 是否在左上角显示指标名称及其参数
    text_color: 指标名称解释文字的颜色，默认为黑色
    marker：标记类型
    markerfacecolor：标记颜色
    markeredgecolor: 标记的边缘颜色
    zero_on: 是否需要在y=0轴上绘制一条直线
    label  : label
    *args, **kwargs : pylab plot参数
    """
    if not indicator:
        print("indicator is None")
        return
    
    if not axes:
        axes = create_one_axes_figure() if new else gca()
    
    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    py_indicatr = [ None if x == constant.null_price else x for x in indicator]
    axes.plot(py_indicatr, '-', label=label, *args, **kwargs)
        
    if legend_on:
        leg = axes.legend(loc='upper left')
        leg.get_frame().set_alpha(0.5)
        
    if text_on:
        if not axes.texts:
            axes.text(0.01,0.97, label, horizontalalignment='left', verticalalignment='top', 
                      transform=axes.transAxes, color=text_color)
        else:
            temp_str = axes.texts[0].get_text() + '  ' + label
            axes.texts[0].set_text(temp_str)
        
    if zero_on:
        ylim = axes.get_ylim()
        if ylim[0]<0<ylim[1]:
            axes.hlines(0,0,len(indicator))

    axes.autoscale_view()
    axes.set_xlim(-1, len(indicator)+1)
    #draw()

def ibar(indicator, axes=None, width=0.4, color='r', edgecolor='r',
         new=True, legend_on=False, text_on=False, text_color='k', label=None, 
                   zero_on=False, *args, **kwargs):
    """绘制indicator曲线
    indicator: Indicator实例
    axes: 指定的坐标轴
    new: 是否在新窗口中显示，只在没有指定axes时生效
    legend_on : 是否打开图例
    text_on: 是否在左上角显示指标名称及其参数
    text_color: 指标名称解释文字的颜色，默认为黑色
    zero_on: 是否需要在y=0轴上绘制一条直线
    *args, **kwargs : pylab bar参数
    """
    if not indicator:
        print("indicator is None")
        return
    
    if not axes:
        axes = create_one_axes_figure() if new else gca()

    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])
    
    py_indicatr = [ None if x == constant.null_price else x for x in indicator]
    x = [i-0.2 for i in range(len(indicator))]
    y = py_indicatr
    
    axes.bar(x, py_indicatr, width=width, color=color, edgecolor=edgecolor, 
             *args, **kwargs)
        
    if legend_on:
        leg = axes.legend(loc='upper left')
        leg.get_frame().set_alpha(0.5)
        
    if text_on:
        if not axes.texts:
            axes.text(0.01,0.97, label, horizontalalignment='left', verticalalignment='top', 
                      transform=axes.transAxes, color=text_color)
        else:
            temp_str = axes.texts[0].get_text() + '  ' + label
            axes.texts[0].set_text(temp_str)
        
    if zero_on:
        ylim = axes.get_ylim()
        if ylim[0]<0<ylim[1]:
            axes.hlines(0,0,len(indicator))

    axes.autoscale_view()
    axes.set_xlim(-1, len(indicator)+1)
    #draw()    

def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """绘制MACD
    参数：
        axes：指定的坐标轴
        kdata：KData
        n1: MACD参数1，参见MACD
        n2: MACD参数2，参见MACD
        n3: MACD参数3，参见MACD
    """
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.getResult(0), macd.getResult(1), macd.getResult(2)
    
    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f'%(n1,n2,n3,fmacd[-1],smacd[-1],bmacd[-1])
    axes.text(0.01,0.97, text, horizontalalignment='left', verticalalignment='top', transform=axes.transAxes)
    total = len(kdata)
    x = [i-0.2 for i in range(total)]
    x1 = [x[i] for i,d in enumerate(bmacd) if d>0]
    y1 = [i for i in bmacd if i>0]
    x2 = [x[i] for i,d in enumerate(bmacd) if d<=0]
    y2 = [i for i in bmacd if i<=0]
    axes.bar(x1,y1, width=0.4, color='r', edgecolor='r')
    axes.bar(x2,y2, width=0.4, color='g', edgecolor='g')
    
    axt = axes.twinx()
    axt.grid(False)
    axt.set_yticks([])
    fmacd.plot(axes=axt, linestyle='--', legend_on=False, text_on=False)
    smacd.plot(axes=axt, legend_on=False, text_on=False)
    
    for label in axt.get_xticklabels():
        label.set_visible(False)
        
        
def ax_draw_macd2(axes, ref, kdata, n1=12, n2=26, n3=9):
    """
    绘制MACD，当BAR值变化与参考序列ref变化不一致时，显示为灰色，
    当BAR和参考序列ref同时上涨，显示红色
    当BAR和参考序列ref同时下跌，显示绿色
    参数:
        axes：指定的坐标轴
        ref: 参考序列，EMA
        kdata：KData
        n1: MACD参数1，参见MACD
        n2: MACD参数2，参见MACD
        n3: MACD参数3，参见MACD
    """
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.getResult(0), macd.getResult(1), macd.getResult(2)

    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f'%(n1,n2,n3,fmacd[-1],smacd[-1],bmacd[-1])
    axes.text(0.01,0.97, text, horizontalalignment='left', verticalalignment='top', transform=axes.transAxes)
    total = len(kdata)
    x = [i-0.2 for i in range(0, total)]
    y = bmacd
    x1,x2,x3 = [x[0]],[],[]
    y1,y2,y3 = [y[0]],[],[]
    for i in range(1, total):
        if ref[i]-ref[i-1]>0 and y[i]-y[i-1]>0:
            x2.append(x[i])
            y2.append(y[i])
        elif ref[i]-ref[i-1]<0 and y[i]-y[i-1]<0:
            x3.append(x[i])
            y3.append(y[i])
        else:
            x1.append(x[i])
            y1.append(y[i])
    
    axes.bar(x1,y1, width=0.4, color='#BFBFBF', edgecolor='#BFBFBF')
    axes.bar(x2,y2, width=0.4, color='r', edgecolor='r')
    axes.bar(x3,y3, width=0.4, color='g', edgecolor='g')
    
    axt = axes.twinx()
    axt.grid(False)
    axt.set_yticks([])
    fmacd.plot(axes=axt, linestyle='--', legend_on=False, text_on=False)
    smacd.plot(axes=axt, legend_on=False, text_on=False)
    
    for label in axt.get_xticklabels():
        label.set_visible(False)  
        
        
def sgplot(sg, kdata = None, style = 1, axes = None, new = True):
    """
    绘制买入/卖出信号
    参数：
        sg：   信号发生器
        kdata：指定的KData（即信号发生器的交易对象），如该值为None，则认为该信号
               发生器已经指定了交易对象，否则，使用该参数作为交易对象
        style: 1 | 2 信号箭头绘制样式
        axes： 指定在那个轴对象中进行绘制
        new：  仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    """
    if kdata is None:
        kdata = sg.getTO()
    else:
        sg.setTO(kdata)
        
    refdates = kdata.getDatetimeList()
    date_index = dict([(d,i) for i,d in enumerate(refdates)])
        
    if axes is None:
        if new:
            axes = create_one_axes_figure()
            kplot(kdata, axes=axes)
        else:
            axes = gca()        
            
    ylim = axes.get_ylim()
    height = ylim[1]-ylim[0]
    
    if style == 1:
        arrow_buy = dict(arrowstyle="->")
        arrow_sell = arrow_buy
    else:
        arrow_buy = dict(facecolor='red', frac=0.5)
        arrow_sell = dict(facecolor='blue', frac=0.5)  

    dates = sg.getBuySignal()
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        axes.annotate('B', 
                      (pos, krecord.lowPrice - height*0.01), 
                      (pos, krecord.lowPrice - height*0.1), 
                      arrowprops = arrow_buy,
                      horizontalalignment = 'center', 
                      verticalalignment = 'bottom',
                      color='red')
              
    dates = sg.getSellSignal()
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        axes.annotate('S', 
                      (pos, krecord.highPrice + height*0.01), 
                      (pos, krecord.highPrice + height*0.1), 
                      arrowprops = arrow_sell,
                      horizontalalignment = 'center', 
                      verticalalignment = 'top',
                      color='blue')            

                      
def cnplot(cn, kdata = None, axes = None, new = True):
    """
    绘制系统有效条件
    参数：
        cn：   系统有效条件
        kdata：指定的KData，如该值为None，则认为该系统有效条件已经指定了交易对象，否则，使用该参数作为交易对象
        axes： 指定在那个轴对象中进行绘制
        new：  仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    """
    if kdata is None:
        kdata = cn.getTO()
    else:
        cn.setTO(kdata)
        
    refdates = kdata.getDatetimeList()
    date_index = dict([(d,i) for i,d in enumerate(refdates)])
        
    if axes is None:
        if new:
            axes = create_one_axes_figure()
            kplot(kdata, axes=axes)
        else:
            axes = gca()        

    x = np.array([i for i in range(len(refdates))])
    y1 = np.array([1 if cn.isValid(d) else -1 for d in refdates])
    y2 = np.array([-1 if cn.isValid(d) else 1 for d in refdates])
    axes.fill_between(x, y1, y2, where=y2 > y1, facecolor='blue', alpha=0.6)
    axes.fill_between(x, y1, y2, where=y2 < y1, facecolor='red', alpha=0.6)


def sysplot(sys, style = 1, axes = None, new = True):
    """
    绘制系统实际买入/卖出信号
    参数：
        sys：  系统实例
        style: 1 | 2 信号箭头绘制样式
        axes： 指定在那个轴对象中进行绘制
        new：  仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    """
    kdata = sys.getTO()
        
    refdates = kdata.getDatetimeList()
    date_index = dict([(d,i) for i,d in enumerate(refdates)])
        
    if axes is None:
        if new:
            axes = create_one_axes_figure()
            kplot(kdata, axes=axes)
        else:
            axes = gca()        
            
    ylim = axes.get_ylim()
    height = ylim[1]-ylim[0]
    
    if style == 1:
        arrow_buy = dict(arrowstyle="->")
        arrow_sell = arrow_buy
    else:
        arrow_buy = dict(facecolor='red', frac=0.5)
        arrow_sell = dict(facecolor='blue', frac=0.5)  

    tds = sys.tm.getTradeList()
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
        axes.annotate('B', 
                      (pos, krecord.lowPrice - height*0.01), 
                      (pos, krecord.lowPrice - height*0.1), 
                      arrowprops = arrow_buy,
                      horizontalalignment = 'center', 
                      verticalalignment = 'bottom',
                      color='red')
              
    for d in sell_dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        axes.annotate('S', 
                      (pos, krecord.highPrice + height*0.01), 
                      (pos, krecord.highPrice + height*0.1), 
                      arrowprops = arrow_sell,
                      horizontalalignment = 'center', 
                      verticalalignment = 'top',
                      color='blue')            
        
