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
 

通达信兼容绘图函数
----------------------------

通达信兼容绘图函数（仅支持 matplotlib 引擎）

.. py:function:: RGB(r, g, b)

    颜色RGB值，如 RGB(255, 0, 0)


.. py:function:: STICKLINE(cond: Indicator, price1: Indicator, price2: Indicator, width: int = 2.0,
              empty: bool = False, color='m', alpha=1.0, kdata=None, new=False, axes=None)

    在满足cond的条件下，在 price1 和 price2 之间绘制一个宽度为 width 的柱状图。

    注意: cond, price1, price2 应含有数据，否则请指定 kdata 作为指标计算的上下文

    :param Indicator cond: 条件表达式，用于确定是否绘制柱状线
    :param Indicator price1: 第一个价格
    :param Indicator price2: 第二个价格
    :param int width: (optional) 柱状宽度. Defaults to 2.0.
    :param bool empty: (optional): 空心. Defaults to False.
    :param KData kdata: (optional): 指定的上下文K线. Defaults to None.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 在指定的坐标轴中绘制. Defaults to None.
    :param str color: (optional): 颜色. Defaults to 'm'.
    :param float alpha: (optional): 透明度. Defaults to 1.0. 


.. py:function:: DRAWBAND(val1: Indicator, color1='m', val2: Indicator = None, color2='b', kdata=None, alpha=0.2, new=False, axes=None, linestyle='-')

    画出带状线

    用法:DRAWBAND(val1, color1, val2, color2), 当 val1 > val2 时,在 val1 和 val2 之间填充 color1;
    当 val1 < val2 时,填充 color2,这里的颜色均使用 matplotlib 颜色代码.
    例如:DRAWBAND(OPEN, 'r', CLOSE, 'b')

    :param Indicator val1: 指标1
    :param str color1: (optional): 颜色1. Defaults to 'm'.
    :param Indicator val2: (optional): 指标2. Defaults to None.
    :param str color2: (optional): 颜色2. Defaults to 'b'.
    :param KData kdata: (optional): 指定指标上下文. Defaults to None.
    :param float alpha: (optional): 透明度. Defaults to 0.2.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 在指定的坐标轴中绘制. Defaults to None.
    :param str linestyle: (optional): 包络线类型. Defaults to '-'.


.. py:function:: PLOYLINE(cond: Indicator, price: Indicator, kdata: KData = None, color: str = 'm', linewidth=1.0, new=False, axes=None, *args, **kwargs)


    在图形上绘制折线段。

    用法：PLOYLINE(COND，PRICE)，当COND条件满足时，以PRICE位置为顶点画折线连接。
    例如：PLOYLINE(HIGH>=HHV(HIGH,20),HIGH, kdata=k)表示在创20天新高点之间画折线。

    :param Indicator cond: 指定条件
    :param Indicator price: 位置
    :param KData kdata: (optional): 指定的上下文. Defaults to None.
    :param str color: (optional): 颜色. Defaults to 'b'.
    :param float linewidth: (optional): 宽度. Defaults to 1.0.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 指定的axes. Defaults to None.


.. py:function:: DRAWLINE(cond1: Indicator, price1: Indicator, cond2: Indicator, price2: Indicator, expand: int = 0, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    在图形上绘制直线段。

    用法：DRAWLINE(cond1, price1, cond2, price2, expand)
    当COND1条件满足时，在PRICE1位置画直线起点，当COND2条件满足时，在PRICE2位置画直线终点，EXPAND为延长类型。
    例如：DRAWLINE(HIGH>=HHV(HIGH,20),HIGH,LOW<=LLV(LOW,20),LOW,1)表示在创20天新高与创20天新低之间画直线并且向右延长

    :param Indicator cond1: 条件1
    :param Indicator price1: 位置1
    :param Indicator cond2: 条件2
    :param Indicator price2: 位置2
    :param int expand: (optional): 0: 不延长 | 1: 向右延长 | 10: 向左延长 | 11: 双向延长. Defaults to 0.
    :param KData kdata: (optional): 指定的上下文. Defaults to None.
    :param str color: (optional): 指定颜色. Defaults to 'm'.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 指定的坐标轴. Defaults to None.


.. py:function:: DRAWTEXT(cond: Indicator, price: Indicator, text: str, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    在图形上显示文字。

    用法: DRAWTEXT(cond, price, text), 当 cond 条件满足时, 在 price 位置书写文字 text。
    例如: DRAWTEXT(CLOSE/OPEN>1.08,LOW,'大阳线')表示当日实体阳线大于8%时在最低价位置显示'大阳线'字样.

    :param Indicator cond: 条件
    :param Indicator price: 显示位置
    :param str text: 待显示文字
    :param KData kdata: (optional): 指定的上下文. Defaults to None.
    :param str color: (optional): 指定颜色. Defaults to 'm'.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 指定的坐标轴. Defaults to None.


.. py:function:: DRAWTEXT_FIX(cond: Indicator, x: float, y: float,  type: int, text: str, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    固定位置显示文字

    用法:DRAWTEXT_FIX(cond,x y, text), cond 中一般需要加 ISLASTBAR,当 cond 条件满足时,
    在当前指标窗口内(X,Y)位置书写文字TEXT,X,Y为书写点在窗口中相对于左上角的百分比

    例如:DRAWTEXT_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08),0.5,0.5,0,'大阳线')表示最后一个交易日实体阳线
    大于8%时在窗口中间位置显示'大阳线'字样.

    :param Indicator cond: 条件
    :param float x: x轴坐标
    :param float y: y轴坐标
    :param int type: (optional): 0 左对齐 | 1 右对齐. 
    :param str text: 待显示文字
    :param KData kdata: (optional): 指定的上下文. Defaults to None.
    :param str color: (optional): 指定颜色. Defaults to 'm'.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 指定坐标轴. Defaults to None.


.. py:function:: DRAWNUMBER(cond: Indicator, price: Indicator, number: Indicator, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    画出数字.

    用法:DRAWNUMBER(cond, price, number),当 cond 条件满足时,在 price 位置书写数字 number.
    例如:DRAWNUMBER(CLOSE/OPEN>1.08,LOW,C)表示当日实体阳线大于8%时在最低价位置显示收盘价。

    :param Indicator cond:: 条件
    :param Indicator price: 绘制位置
    :param Indicator number: 待绘制数字
    :param KData kdata: (optional): 指定的上下文. Defaults to None.
    :param str color: (optional): 指定颜色. Defaults to 'm'.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 指定的坐标轴. Defaults to None.


.. py:function:: DRAWNUMBER_FIX(cond: Indicator, x: float, y: float, type: int, number: float, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    固定位置显示数字.

    用法:DRAWNUMBER_FIX(cond,x,y,type,number), cond 中一般需要加 ISLASTBAR, 当 cond 条件满足时,
    在当前指标窗口内 (x, y) 位置书写数字 number, x,y为书写点在窗口中相对于左上角的百分比,type:0为左对齐,1为右对齐。

    例如:DRAWNUMBER_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08), 0.5,0.5,0,C)表示最后一个交易日实体阳线大于8%时在窗口中间位置显示收盘价

    :param Indicator cond: _description_
    :param float x: _description_
    :param float y: _description_
    :param int type: _description_
    :param Indicator number: _description_
    :param KData kdata: (optional): _description_. Defaults to None.
    :param str color: (optional): _description_. Defaults to 'm'.
    :param bool new: (optional): _description_. Defaults to False.
    :param axes: (optional): _description_. Defaults to None.


.. py:function:: DRAWSL(cond: Indicator, price: Indicator, slope: Union[Indicator, float, int], length: Union[Indicator, float, int], direct: int, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    绘制斜线.

    用法:DRAWSL(cond,price,slope,length,diect),当 cond 条件满足时,在 price 位置画斜线, slope 为斜率, 
    lengh为长度, direct 为0向右延伸,1向左延伸,2双向延伸。

    注意:
    1. K线间的纵向高度差为 slope;
    2. slope 为 0 时, 为水平线;
    3. slope 为 10000 时, 为垂直线, length 为向上的像素高度, direct 表示向上或向下延伸
    4. slope 和 length 支持变量;

    :param Indicator cond: 条件指标
    :param Indicator price:: 价格
    :param slope: 斜率
    :param length: 长度
    :param int direct: 方向
    :param KData kdata: (optional): 指定的上下文. Defaults to None.
    :param str color: (optional): 颜色. Defaults to 'm'.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 指定的坐标轴. Defaults to None.


.. py:function:: DRAWIMG(cond: Indicator, price: Indicator, img: str, kdata: KData = None, new=False, axes=None, *args, **kwargs)    

    画图片

    用法:DRAWIMG(cond,price,'图像文件文件名'),当条件 cond 满足时,在 price 位置画指定的图片
    例如:DRAWIMG(O>C,CLOSE, '123.png')。

    :param Indicator cond: 指定条件
    :param Indicator price: 指定价格
    :param str img: 图像文件名
    :param KData kdata: (optional): 指定上下文. Defaults to None.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 在指定坐标轴中绘制. Defaults to None.


.. py:function:: DRAWICON(cond: Indicator, price: Indicator, type: int, kdata: KData = None, new=False, axes=None, *args, **kwargs)

    绘制内建图标


.. py:function:: SHOWICONS()

    显示所有内置图标


.. py:function:: DRAWRECTREL(left: int, top: int, right: int, bottom: int, color='m', frame=True, fill=True, alpha=0.1, new=False, axes=None, *args, **kwargs)

    相对位置上画矩形.

    注意：原点为坐标轴左上角(0, 0)，和 matplotlib 不同。
    用法: DRAWRECTREL(left,top,right,bottom,color), 以图形窗口 (left, top) 为左上角, (right, bottom) 为右下角绘制矩形, 坐标单位是窗口沿水平和垂直方向的1/1000,取值范围是0—999,超出范围则可能显示在图形窗口外,矩形中间填充颜色COLOR,COLOR为0表示不填充.

    例如:DRAWRECTREL(0,0,500,500,RGB(255,255,0)) 表示在图形最左上部1/4位置用黄色绘制矩形

    :param int left: 左上角x
    :param int top: 左上角y
    :param int right: 右下角x
    :param int bottom: 右下角y
    :param str color: (optional): 指定颜色. Defaults to 'm'.
    :param bool frame: (optional): 添加边框. Defaults to False.
    :param bool fill: (optional): 颜色填充. Defaults to True.
    :param float alpha: (optional): 透明度. Defaults to 0.1.
    :param bool new: (optional): 在新窗口中绘制. Defaults to False.
    :param axes: (optional): 指定的坐标轴. Defaults to None.



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

    
.. py:function:: iheatmap(ind, axes=None):

    绘制指标收益年-月收益热力图。并非所有指标均可绘制出热力图，详见热力图公式:

    指标收益率 = (当前月末值 - 上月末值) / 上月末值 * 100

    指标应已计算（即有值），且为时间序列

    :param ind: 指定指标
    :param axes: 绘制的轴对象，默认为None，表示创建新的轴对象
    :return: None


.. py:function:: sgplot(sg[, new = True, axes = None,  style = 1, kdata = None])

    绘制买入/卖出信号

    :param SignalBase sg: 信号指示器
    :param new:   仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes:  指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式
    :param KData kdata: 指定的KData（即信号发生器的交易对象），如该值为None，则认为该信号发生器已经指定了交易对象，否则，使用该参数作为交易对象


.. py:function:: cnplot(cn[, new=True, axes=None, kdata=None, upcolor='red', downcolor='blue', alpha=0.2])

    绘制系统有效条件

    :param ConditionBase cn: 系统有效条件
    :param new:  仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
    :param KData kdata: 指定的KData，如该值为None，则认为该系统有效条件已经指定了交易对象，否则，使用该参数作为交易对象
    :param upcolor: 有效数时的颜色
    :param downcolor: 无效时的颜色
    :param alpha: 透明度    


.. py:function:: evplot(ev, ref_kdata, new=True, axes=None, upcolor='red', downcolor='blue', alpha=0.2)

    绘制市场有效判断

    :param EnvironmentBase cn: 系统有效条件
    :param KData ref_kdata: 用于日期参考
    :param new: 仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes: 指定在那个轴对象中进行绘制
    :param upcolor: 有效时的颜色
    :param downcolor: 无效时的颜色
    :param alpha: 透明度

    
.. py:function:: sysplot(sys[, new=True, axes=None, style=1])

    绘制系统实际买入/卖出信号
    
    :param SystemBase sys: 系统实例
    :param new:   仅在未指定axes的情况下生效，当为True时，创建新的窗口对象并在其中进行绘制
    :param axes:  指定在那个轴对象中进行绘制
    :param style: 1 | 2 信号箭头绘制样式


.. py:function:: sys_performance(sys, ref_stk=None)

    绘制系统绩效，即账户累积收益率曲线。通常不直接调用，而是在 sys, pf 计算完成后，查看绩效详情。
    如: sys.performance()

    :param SystemBase | PortfolioBase sys: SYS或PF实例
    :param Stock ref_stk: 参考股票, 默认为沪深300: sh000300, 绘制参考标的的收益曲线
    

.. py:function:: sys_heatmap(sys, axes=None):

    绘制系统收益年-月收益热力图

    :param sys: SYS 或 PF 实例
    :param axes: 绘制的轴对象, 默认为None, 表示创建新的轴对象


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
    
