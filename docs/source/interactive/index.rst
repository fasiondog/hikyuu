交互工具
=========

Hikyuu是研究系统交易的Python量化框架，可以在其基础上构建自己的客户端程序。而Hikyuu库本身自带一个交互式工具，可以在Python Shell环境下进行探索，可以从在Python shell中使用 “from hikyuu.interactive.interactive” 引入该工具。该工具主要提供基于 matplotlib 的图形绘制函数，便于在交互式环境下绘制K线、指标、信号等。

使用 matplotlib 绘制图形，必须了解 matplotlib 绘图中的两个基本对象：figure、axes。窗口（figure）是坐标轴（axes）对象的容器，绘图时首先需指定窗口，在窗口中绘制相应的坐标轴，然后在坐标轴中绘制图形。在 matplotlib 中绘制图形需要指定图形在哪个坐标轴中进行绘制。当绘图函数中没有指定坐标轴时，将默认使用当前的坐标轴，如果不存在任何一个坐标轴时，matplotlib 将创建自动创建一个窗口及其包含的坐标轴。

绘图的详细示例：

`<https://nbviewer.jupyter.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/000-Index.ipynb?flush_cache=True>`_


绘制组合窗口
--------------------

创建组合窗口的快捷函数,返回相应的 axes 对象或tuple。

.. py:function:: create_figure([n = 1, figsize = (10,8)])

    生成含有指定坐标轴数量的窗口，最大只支持4个坐标轴。
    
    :param int n: 坐标轴数量
    :param figsize: (宽, 高)
    :return: (ax1, ax2, ...) 根据指定的坐标轴数量而定，超出[1,4]个坐标轴时，返回None    
    

    
绘制图形的便捷函数
--------------------
    
.. py:function:: ax_set_locator_formatter(axes, dates, typ)

    设置指定坐标轴的日期显示，根据指定的K线类型优化X轴坐标显示
    
    :param axes: 指定的坐标轴
    :param dates: Datetime构成可迭代序列
    :param Query.KType typ: K线类型
    
 
.. py:function:: adjust_axes_show(axeslist)

    用于调整上下紧密相连的坐标轴显示时，其上一坐标轴最小值刻度和下一坐标轴最大值刻度显示重叠的问题
    
    :param axeslist: 上下相连的坐标轴列表 (ax1,ax2,...)
 
 
.. py:function:: ax_draw_macd(axes, kdata[, n1=12, n2=26, n3=9])

    绘制MACD
    
    :param axes: 指定的坐标轴
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
 
 
.. py:function:: ax_draw_macd2(axes, ref, kdata, n1=12, n2=26, n3=9)

    绘制MACD。
    当BAR值变化与参考序列ref变化不一致时，显示为灰色，
    当BAR和参考序列ref同时上涨，显示红色
    当BAR和参考序列ref同时下跌，显示绿色

    :param axes: 指定的坐标轴
    :param ref: 参考序列，EMA
    :param KData kdata: KData
    :param int n1: 指标 MACD 的参数1
    :param int n2: 指标 MACD 的参数2
    :param int n3: 指标 MACD 的参数3
 
 

绘制K线、指标、信号等
----------------------

以下函数已设为相应类型对象的 plot 函数，如 KData kdata 可直接调用 kdata.plot()，等同于没有第一个参数的 kplot()
    
.. py:function:: kplot(kdata[, new=True, axes=None, colorup='r', colordown='g', width=0.6, alpha=1.0])

    绘制K线图
    
    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
    :param colorup:     the color of the rectangle where close >= open
    :param colordown:   the color of the rectangle where close < open
    :param width:       fraction of a day for the rectangle width
    :param alpha:       the rectangle alpha level, 透明度(0.0~1.0) 1.0为不透明


.. py:function:: mkplot(kdata[, new=True, axes=None, colorup='r', colordown='g', ticksize=3])

    绘制美式K线图
    
    :param KData kdata: K线数据
    :param bool new:    是否在新窗口中显示，只在没有指定axes时生效
    :param axes:        指定的坐标轴
    :param colorup:     the color of the lines where close >= open
    :param colordown:   the color of the lines where close < open
    :param ticksize:    open/close tick marker in points

    
.. py:function:: iplot(indicator[, new=True, axes=None, legend_on=False, text_on=False, text_color='k', zero_on=False, label=None, *args, **kwargs])
          
    绘制indicator曲线图
    
    :param Indicator indicator: indicator实例
    :param new:             是否在新窗口中显示，只在没有指定axes时生效
    :param axes:            指定的坐标轴
    :param legend_on:       是否打开图例
    :param text_on:         是否在左上角显示指标名称及其参数
    :param text_color:      指标名称解释文字的颜色，默认为黑色
    :param zero_on:         是否需要在y=0轴上绘制一条直线
    :param str label:       label显示文字信息，text_on 及 legend_on 为 True 时生效
    :param args:            pylab plot参数
    :param kwargs:          pylab plot参数，如：marker（标记类型）、markerfacecolor（标记颜色）、markeredgecolor（标记的边缘颜色）
    

.. py:function:: ibar(indicator[, new=True, axes=None, legend_on=False, text_on=False, text_color='k', label=None, width=0.4, color='r', edgecolor='r', zero_on=False, *args, **kwargs])

    绘制indicator柱状图
    
    :param Indicator indicator: Indicator实例
    :param axes:       指定的坐标轴
    :param new:        是否在新窗口中显示，只在没有指定axes时生效
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

    
.. py:function:: sgplot(sg[, new = True, axes = None,  style = 1, kdata = None])

    绘制买入/卖出信号

    :param SignalBase sg: 信号指示器
    :param new:   仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes:  指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式
    :param KData kdata: 指定的KData（即信号发生器的交易对象），如该值为None，则认为该信号发生器已经指定了交易对象，否则，使用该参数作为交易对象


.. py:function:: cnplot(cn[, new=True, axes=None, kdata=None])

    绘制系统有效条件

    :param ConditionBase cn: 系统有效条件
    :param new:  仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
    :param KData kdata: 指定的KData，如该值为None，则认为该系统有效条件已经指定了交易对象，否则，使用该参数作为交易对象

    
.. py:function:: sysplot(sys[, new=True, axes=None, style=1])

    绘制系统实际买入/卖出信号
    
    :param SystemBase sys: 系统实例
    :param new:   仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes:  指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式

    

内建示例
----------

.. py:function:: vl.draw(stock, query=Query(-130), ma1_n=5, ma2_n=10, ma3_n=20, ma4_n=60, ma5_n=100, ma_type="SMA", vma1_n=5, vma2_n=10)

    绘制普通K线图 + 成交量（成交金额）


.. py:function:: vl.draw2(stock, query=Query(-130), ma1_n=7, ma2_n=20, ma3_n=30, ma4_n=42, ma5_n=100, vma1_n=5, vma2_n=10) 

    绘制普通K线图 + 成交量（成交金额）+ MACD

    
.. py:function:: el.draw(stock, query=QueryByIndex(-130), ma_n=22, ma_w='auto', vigor_n=13)

    绘制亚历山大.艾尔德交易系统图形。参见 [BOOK2]_
    
    
.. py:function:: kf.draw(stock, query=Query(-130), n=10, filter_n=20, filter_p=0.1, sg_type = "CROSS", show_high_low=False,  arrow_style=1)

    绘制佩里.J.考夫曼（Perry J.Kaufman） 自适应移动平均系统(AMA)。参见 [BOOK1]_
    
    
.. py:function:: kf.draw2(block, query=Query(-130), ama1=AMA(n=10, fast_n=2, slow_n=30), ama2=None,n=10, filter_n=20, filter_p=0.1, sg_type='CROSS', show_high_low=True, arrow_style=1)   

    绘制佩里.J.考夫曼（Perry J.Kaufman） 自适应移动平均系统(AMA)。参见 [BOOK1]_
    
