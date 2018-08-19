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
# history:
# 1. 20171121, Added by fasiondog
#===============================================================================

from hikyuu import KQuery, constant
from hikyuu.indicator import MACD, CLOSE, HIGH, LOW
from hikyuu.trade_manage import BUSINESS 

from pyecharts import Overlap, Line, Kline, Bar, Grid, Style, EffectScatter
from .common import get_draw_title


def ax_set_xlim(self, *args, **kwargs):
    pass

def ax_set_ylim(self, *args, **kwargs):
    pass

def ax_fill_between(self, *args, **kwargs):
    pass

Overlap.set_xlim = ax_set_xlim
Overlap.set_ylim = ax_set_ylim
Overlap.fill_between = ax_fill_between


class HKUFigure(object):
    def __init__(self, width, height, num=1):
        if num < 1:
            self._num = 1
        elif num > 4:
            self._num = 4
        else:
            self._num = num
            
        self._added_flag = [False, False, False, False]
        self._grid = Grid(width=width, height=height)
        self._width = width
        self._height = height
        self._axis_num = {}
        self._axis_list = [None, None, None, None]
        for i in range(self._num):
            overlap = Overlap(width=width, height=height)
            self._axis_num[overlap] = i
            self._axis_list[i] = overlap
        self._current_axis = 0
        self._xaxis = None
        self._label_color = []
        
    def _repr_html_(self):
        return self.get_grid()._repr_html_()
        
    def get_current_axis(self):
        return self._axis_list[self._current_axis]
    
    def get_axis(self, pos):
        return self._axis_list[pos]
    
    def get_grid(self):
        return self._grid
    
    def set_xaxis(self, xaxis):
        self._xaxis = xaxis
        
    def get_xaxis(self):
        return self._xaxis
    
    def add_axis(self, axis):
        if axis not in self._axis_num:
            return
        
        pos = self._axis_num[axis]
        self._current_axis = pos
        self._added_flag[pos] = True
        self._flush()
        return axis

    def _flush(self):
        self._grid = Grid(width=self._width, height=self._height)
        for pos, axis in enumerate(self._axis_list):
            if axis is None or axis.options is None \
                or axis.options.get('yAxis') is None:
                continue
            
            if self._num == 1:
                self._grid.add(axis, grid_top="8%")
            
            elif self._num == 2:
                if pos == 0:
                    self._grid.add(axis, grid_top="8%", grid_height='52%')
                else:
                    self._grid.add(axis, grid_top="62%")
                
            elif self._num == 3:
                if pos == 0:
                    self._grid.add(axis, grid_top='8%', grid_height='40%')
                elif pos == 1:
                    self._grid.add(axis, grid_top='50%', grid_height='19%')
                else:
                    self._grid.add(axis, grid_bottom='10%', grid_height='19%')
                
            else:
                if pos == 0:
                    self._grid.add(axis, grid_top='8%', grid_height='30%')
                elif pos == 1:
                    self._grid.add(axis, grid_top='40%', grid_height='18%')
                elif pos == 2:
                    self._grid.add(axis, grid_top='60%', grid_height='15%')
                else:
                    self._grid.add(axis, grid_top='77%', grid_height='15%')        

    def get_style(self, axis, **kwargs):
        pos  = self._axis_num[axis]
        num = self._num
        style = Style()
        default_datazoom_type = 'both'
        result = None
        if num == 1:
            result = style.add(legend_text_size='10',
                               legend_top='8%',
                               legend_pos='10%',
                               legend_orient='vertical',
                               is_symbol_show=False,
                               is_datazoom_show=True,
                               datazoom_type=default_datazoom_type)
    
        elif num == 2:
            if pos == 0:
                result = style.add(legend_text_size='10',
                                   legend_top='8%',
                                   legend_pos='10%',
                                   legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1],
                                   is_xaxis_show=False)
            else:
                result = style.add(legend_text_size='10',
                                   legend_top='62%',
                                   legend_pos='10%',
                                   #legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1],
                                   is_xaxis_show=True)            
        
        elif num == 3:
            if pos == 0:
                result = style.add(legend_text_size='10',
                                   legend_top='8%',
                                   legend_pos='10%',
                                   legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1,2],
                                   is_xaxis_show=False)
            elif pos == 1:
                result = style.add(legend_text_size='10',
                                   legend_top='50%',
                                   legend_pos='10%',
                                   #legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1,2],
                                   is_xaxis_show=False)
            else:
                result = style.add(legend_text_size='10',
                                   legend_top='71%',
                                   legend_pos='10%',
                                   #legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1,2],
                                   is_xaxis_show=True)
            
        else:
            if pos == 0:
                result = style.add(legend_text_size='10',
                                   legend_top='8%',
                                   legend_pos='10%',
                                   legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1,2,3],
                                   is_xaxis_show=False)
            elif pos == 1:
                result = style.add(legend_text_size='10',
                                   legend_top='40%',
                                   legend_pos='10%',
                                   #legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1,2,3],
                                   is_xaxis_show=False)
            elif pos == 2:
                result = style.add(legend_text_size='10',
                                   legend_top='60%',
                                   legend_pos='10%',
                                   #legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1,2,3],
                                   is_xaxis_show=False)
            else:
                result = style.add(legend_text_size='10',
                                   legend_top='77%',
                                   legend_pos='10%',
                                   #legend_orient='vertical',
                                   is_symbol_show=False,
                                   is_datazoom_show=True,
                                   datazoom_type=default_datazoom_type,
                                   datazoom_xaxis_index=[0,1,2,3],
                                   is_xaxis_show=True)                

        color_map = {'r': '#CD0000',
                     'g': '#008B00',
                     'b': '#0000EE',
                     'c':'#40E0D0',
                     'm': '#CD00CD',
                     'y': '#EE9A00',
                     'k': '#000000',
                     'w': '#FFFFFF'
                     }

        line_map = {'-' : 'solid',
                    '--': 'dashed',
                    ':' : 'dotted'}
        
        for item in kwargs.items():
            if 'color' == item[0]:
                color = color_map[item[1]] if item[1] in color_map else color_map['r']
                result['line_color'] = color
                self._label_color += [color]
                result['label_color'] = self._label_color
            
            elif 'linestyle' == item[0]:
                result['line_type'] = line_map[item[1]] if item[1] in line_map else line_map['-']
            
            else:
                result[item[0]] = item[1]
        
        return result


g_figure = None

def gca():
    global g_figure
    if g_figure is None:
        g_figure = HKUFigure(width=800, height=450, num=1)
    return g_figure.get_current_axis()

def gcf():
    global g_figure
    if g_figure is None:
        g_figure = HKUFigure(width=800, height=450, num=1)
    return g_figure

def get_style(axis):
    return gcf().get_style(axis)

    
def create_one_axes_figure(figsize=(800,450)):
    """生成一个仅含有1个坐标轴的figure，并返回其坐标轴对象
    
    :param figsize: (宽, 高)
    :return: ax
    """
    global g_figure
    g_figure = HKUFigure(width=figsize[0], height=figsize[1])
    return g_figure.get_current_axis()

def create_two_axes_figure(figsize=(800,600)):                                        
    """生成一个含有2个坐标轴的figure，并返回坐标轴列表
    
    :param figsize: (宽, 高)
    :return: (ax1, ax2)    
    """
    global g_figure
    g_figure = HKUFigure(width=figsize[0], height=figsize[1], num=2)
    return g_figure.get_axis(0), g_figure.get_axis(1)

def create_three_axes_figure(figsize=(800,600)):                                        
    """生成一个含有3个坐标轴的figure，并返回坐标轴列表
    
    :param figsize: (宽, 高)
    :return: (ax1, ax2)    
    """
    global g_figure
    g_figure = HKUFigure(width=figsize[0], height=figsize[1], num=3)
    return g_figure.get_axis(0), g_figure.get_axis(1), g_figure.get_axis(2)

def create_four_axes_figure(figsize=(800,600)):                                        
    """生成一个含有4个坐标轴的figure，并返回坐标轴列表
    
    :param figsize: (宽, 高)
    :return: (ax1, ax2)    
    """
    global g_figure
    g_figure = HKUFigure(width=figsize[0], height=figsize[1], num=4)
    return g_figure.get_axis(0), g_figure.get_axis(1), g_figure.get_axis(2), g_figure.get_axis(3)

def create_figure(n=1, figsize=(10,8)):
    """生成含有指定坐标轴数量的窗口，最大只支持4个坐标轴。

    :param int n: 坐标轴数量
    :param figsize: (宽, 高)
    :return: (ax1, ax2, ...) 根据指定的坐标轴数量而定，超出[1,4]个坐标轴时，返回None
    """
    new_figsize = (figsize[0]*100, figsize[1]*100)
    if n == 1:
        return create_one_axes_figure(new_figsize)
    elif n == 2:
        return create_two_axes_figure(new_figsize)
    elif n == 3:
        return create_three_axes_figure(new_figsize)
    elif n == 4:
        return create_four_axes_figure(new_figsize)
    else:
        print("Max support axes number is 4!")
        return None
    

def iplot(indicator, new=True, axes=None, 
          legend_on=False, text_on=False, text_color='k',  
          zero_on=False, label=None, *args, **kwargs):
    """绘制indicator曲线
    
    :param Indicator indicator: indicator实例
    :param axes:            指定的坐标轴
    :param new:             pyecharts中无效
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
    
    if axes is None:
        axes = create_figure() if new else gca()
    
    if not label:
        label = "%s %.2f" % (indicator.long_name, indicator[-1])

    x_list = gcf().get_xaxis()
    if x_list is None:
        x_list = [i for i in range(len(indicator))]
        
    y_list = [ '-' if x == constant.null_price else round(x,3) for x in indicator]
    line = Line()
    
    style = gcf().get_style(axes, **kwargs)
    line.add(label, x_list, y_list, 
             yaxis_min=min(indicator),
             is_legend_show=legend_on,
             *args, **style)
    
    axes.add(line)
    
    gcf().add_axis(axes)
    return gcf()


def ibar(indicator, new=True, axes=None, 
          legend_on=False, text_on=False, text_color='k',  
          zero_on=False, label=None, *args, **kwargs):
    """绘制indicator曲线
    
    :param Indicator indicator: indicator实例
    :param axes:            指定的坐标轴
    :param new:             pyecharts中无效
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

    x_list = gcf().get_xaxis()
    if x_list is None:
        x_list = [i for i in range(len(indicator))]
        
    y_list = [ '-' if x == constant.null_price else round(x,3) for x in indicator]
    bar = Bar()
        
    style = gcf().get_style(axes, **kwargs)
    bar.add(label, x_list, y_list, 
             yaxis_min=min(indicator),
             is_legend_show=legend_on,
             *args, **style)
    
    axes.add(bar)
        
    gcf().add_axis(axes)
    return gcf()


def kplot(kdata, new=True, axes=None, 
          colorup='r', colordown='g', width=0.6, alpha=1.0):
    """绘制K线图
    
    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
    :param colorup:     the color of the rectangle where close >= open
    :param colordown:   the color of the rectangle where close < open
    :param width:       fraction of a day for the rectangle width
    :param alpha:       the rectangle alpha level, 透明度(0.0~1.0) 1.0为不透明
    """
    if not kdata:
        print("kdata is None")
        return
    
    if axes is None:
        axes = create_figure() if new else gca()
        
    title = get_draw_title(kdata)
    last_record = kdata[-1]
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f 涨幅:%.2f%%' % (
        last_record.datetime.date(), 
        last_record.openPrice, last_record.highPrice,
        last_record.lowPrice,  last_record.closePrice,
        100*(last_record.closePrice-kdata[-2].closePrice)/kdata[-2].closePrice)
    
    date_list = kdata.getDatetimeList()
    if kdata.getQuery().kType == KQuery.DAY:
        x_list = [d.date() for d in date_list]
    else:
        x_list = [d.datetime() for d in date_list]
        
    y_list = [[k.openPrice, k.closePrice, k.lowPrice, k.highPrice] for k in kdata] 

    style = gcf().get_style(axes)
    kline = Kline(title, text, title_pos='center', subtitle_color='#FF0000')
    kline.add(None, x_axis=x_list, y_axis=y_list,
              mark_line=["max"], mark_line_symbolsize=0, 
              mark_line_valuedim='highest',
              **style)
    axes.add(kline)
    
    gcf().set_xaxis(x_list)
    gcf().add_axis(axes)
    return gcf()


def mkplot(kdata, new=True, axes=None, colorup='r', colordown='g', ticksize=3):
    """绘制美式K线图
    
    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
    :param colorup:     the color of the lines where close >= open
    :param colordown:   the color of the lines where close < open
    :param ticksize:    open/close tick marker in points
    """
    print('Pyecharts暂不支持美式K线图')
    return None

def ax_draw_macd(axes, kdata, n1=12, n2=26, n3=9):
    """绘制MACD
    
    :param axes: 指定的坐标轴
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
    """
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.getResult(0), macd.getResult(1), macd.getResult(2)
    
    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f'%(n1,n2,n3,fmacd[-1],smacd[-1],bmacd[-1])
    #axes.text(0.01,0.97, text, horizontalalignment='left', verticalalignment='top', transform=axes.transAxes)
    total = len(kdata)
    x_list = [i for i in range(total)]
    y1_list = [round(x) if x > 0 else '-' for x in bmacd]
    y2_list = [round(x) if x <= 0 else '-' for x in bmacd]
    
    style = gcf().get_style(axes)
    bar = Bar(subtitle=text, title_pos='10%', title_top='8%')
    bar.add('1', x_list, y1_list, is_stack=True, is_legend_show=False, **style)
    bar.add('2', x_list, y2_list, is_stack=True, is_legend_show=False, **style)
    
    axes.add(bar)
    fmacd.plot(axes=axes, line_type='dotted')
    smacd.plot(axes=axes)

    gcf().add_axis(axes)
    return gcf()

def ax_draw_macd2(axes, ref, kdata, n1=12, n2=26, n3=9):
    """绘制MACD
    
    :param axes: 指定的坐标轴
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
    """
    macd = MACD(CLOSE(kdata), n1, n2, n3)
    bmacd, fmacd, smacd = macd.getResult(0), macd.getResult(1), macd.getResult(2)
    
    text = 'MACD(%s,%s,%s) DIF:%.2f, DEA:%.2f, BAR:%.2f'%(n1,n2,n3,fmacd[-1],smacd[-1],bmacd[-1])
    #axes.text(0.01,0.97, text, horizontalalignment='left', verticalalignment='top', transform=axes.transAxes)
    total = len(kdata)
    x = [i for i in range(total)]
    y = bmacd
    y1,y2,y3 = [y[0]],[y[0]],[y[0]]
    for i in range(1, total):
        if ref[i]-ref[i-1]>0 and y[i]-y[i-1]>0:
            y2.append(y[i])
            y1.append('-')
            y3.append('-')
        elif ref[i]-ref[i-1]<0 and y[i]-y[i-1]<0:
            y3.append(y[i])
            y1.append('-')
            y2.append('-')
        else:
            y1.append(y[i])
            y2.append('-')
            y3.append('-')

    
    style = gcf().get_style(axes)
    bar = Bar(subtitle=text, title_pos='10%', title_top='8%')
    bar.add('1', x, y1, is_stack=True, is_legend_show=False, **style)
    bar.add('2', x, y2, is_stack=True, is_legend_show=False, **style)
    bar.add('3', x, y3, is_stack=True, is_legend_show=False, **style)
    
    axes.add(bar)
    fmacd.plot(axes=axes, line_type='dotted')
    smacd.plot(axes=axes)
    
    gcf().add_axis(axes)
    return gcf()


def sgplot(sg, new=True, axes=None,  style=1, kdata=None):
    """绘制买入/卖出信号

    :param SignalBase sg: 信号指示器
    :param new: 仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式
    :param KData kdata: 指定的KData（即信号发生器的交易对象），
                       如该值为None，则认为该信号发生器已经指定了交易对象，
                       否则，使用该参数作为交易对象
    """
    if kdata is None:
        kdata = sg.getTO()
    else:
        sg.setTO(kdata)
        
    refdates = kdata.getDatetimeList()
    if kdata.getQuery().kType == KQuery.DAY:
        x_list = [d.date() for d in refdates]
    else:
        x_list = [d.datetime() for d in refdates]

    date_index = dict([(d,i) for i,d in enumerate(refdates)])
    
    if axes is None:
        if new:
            axes = create_figure()
            kplot(kdata, axes=axes)
        else:
            axes = gca()        

    es = EffectScatter()
            
    highest = round(max(HIGH(kdata)),2)
    lowest = round(min(LOW(kdata)), 2)
    height = highest - lowest
    
    dates = sg.getBuySignal()
    buy_y_list = ['-' for i in range(len(refdates))]
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        buy_y_list[pos] = round(krecord.lowPrice - height*0.02, 2)
        
    es.add("", x_list, buy_y_list, 
           symbol_size=12, effect_scale=2.5, effect_period=0,symbol="triangle",
           is_label_show=True,
           label_formatter='B',
           label_pos = 'bottom',
           label_text_color = '#CD0000',
           label_color=['#CD0000', '#008B00'])
              
    dates = sg.getSellSignal()
    sell_y_list = ['-' for i in range(len(refdates))]
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        sell_y_list[pos] = round(krecord.highPrice + height*0.015, 2)

    es.add("", x_list, sell_y_list, 
           symbol_size=20, effect_scale=2.5, effect_period=0,symbol="pin",
           is_label_show=True,
           label_formatter='S',
           label_pos = 'top',
           label_text_color = '#008B00',           
           label_color=['#CD0000', '#008B00', '#008B00'])
    
    axes.add(es)
    
    gcf().set_xaxis(x_list)
    gcf().add_axis(axes)
    return gcf()


def cnplot(cn, new=True, axes=None, kdata=None):
    """绘制系统有效条件

    :param ConditionBase cn: 系统有效条件
    :param new: 仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
    :param KData kdata: 指定的KData，如该值为None，则认为该系统有效条件已经
                        指定了交易对象，否则，使用该参数作为交易对象
    """
    if kdata is None:
        kdata = cn.getTO()
    else:
        cn.setTO(kdata)
        
    refdates = kdata.getDatetimeList()
    if kdata.getQuery().kType == KQuery.DAY:
        x_list = [d.date() for d in refdates]
    else:
        x_list = [d.datetime() for d in refdates]
    
    axes2 = None        
    if axes is None:
        if new:
            axes, axes2 = create_figure(2)
            kplot(kdata, axes=axes)
        else:
            axes = gca()

    max_value = max(HIGH(kdata))
    line = Line()
    y1 = [max_value if cn.isValid(d) else 0 for d in refdates]
    y2 = [0 if cn.isValid(d) else max_value for d in refdates]
    line.add("", x_list, y1, 
             is_step=True,
             is_fill=True,
             yaxis_max = max_value,
             is_symbol_show=False,
             line_opacity=0,
             label_color='#CD0000',
             area_color='#CD0000', 
             area_opacity=0.2)
    line.add("", x_list, y2, 
             is_step=True, 
             is_fill=True,
             yaxis_max = max_value,
             is_symbol_show=False,
             line_opacity=0,
             label_color='#0000FF',
             area_color='#0000FF', 
             area_opacity=0.2)
    
    gcf().set_xaxis(x_list)

    if axes2 is not None:
        axes2.add(line)
        gcf().add_axis(axes2)
    else:
        axes.add(line)
        gcf().add_axis(axes)
    
    return gcf()


def sysplot(sys, new=True, axes=None, style=1):
    """绘制系统实际买入/卖出信号
    
    :param SystemBase sys: 系统实例
    :param new:   仅在未指定axes的情况下生效，当为True时，
                   创建新的窗口对象并在其中进行绘制
    :param axes:  指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式
    """
    kdata = sys.getTO()
        
    refdates = kdata.getDatetimeList()
    if kdata.getQuery().kType == KQuery.DAY:
        x_list = [d.date() for d in refdates]
    else:
        x_list = [d.datetime() for d in refdates]

    date_index = dict([(d,i) for i,d in enumerate(refdates)])
    
    if axes is None:
        if new:
            axes = create_figure()
            kplot(kdata, axes=axes)
        else:
            axes = gca()        

    es = EffectScatter()
            
    highest = round(max(HIGH(kdata)),2)
    lowest = round(min(LOW(kdata)), 2)
    height = highest - lowest
    
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
    
    dates = buy_dates
    buy_y_list = ['-' for i in range(len(refdates))]
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        buy_y_list[pos] = round(krecord.lowPrice - height*0.02, 2)
        
    es.add("", x_list, buy_y_list, 
           symbol_size=12, effect_scale=2.5, effect_period=0,symbol="triangle",
           is_label_show=True,
           label_formatter='B',
           label_pos = 'bottom',
           label_text_color = '#CD0000',
           label_color=['#CD0000', '#008B00'])
              
    dates = sell_dates
    sell_y_list = ['-' for i in range(len(refdates))]
    for d in dates:
        if d not in date_index:
            continue
        pos = date_index[d]
        krecord = kdata[pos]
        sell_y_list[pos] = round(krecord.highPrice + height*0.015, 2)

    es.add("", x_list, sell_y_list, 
           symbol_size=20, effect_scale=2.5, effect_period=0,symbol="pin",
           is_label_show=True,
           label_formatter='S',
           label_pos = 'top',
           label_text_color = '#008B00',           
           label_color=['#CD0000', '#008B00', '#008B00'])
    
    axes.add(es)
    
    gcf().set_xaxis(x_list)
    gcf().add_axis(axes)
    return gcf()     
   
