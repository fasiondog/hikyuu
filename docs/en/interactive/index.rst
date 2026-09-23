Interactive Tools
=================

Hikyuu is a Python quantitative framework for researching the system trading, and you can build your own client programs on top of it. The Hikyuu library itself comes with an interactive tool, which can be explored in the Python Shell environment, and can be imported in the Python shell with "from hikyuu.interactive". This tool mainly provides the plotting functions based on matplotlib, making it easy to draw the K-lines, the indicators, the signals, etc. in the interactive environment.

To draw the figures with matplotlib, you must understand the two basic objects in the matplotlib drawing: figure and axes. The window (figure) is the container of the axes object; when drawing, you first need to specify the window, draw the corresponding axes in the window, and then draw the figures in the axes. Drawing a figure in matplotlib requires specifying in which axes the figure is drawn. When the axes is not specified in the plotting function, the current axes will be used by default; if no axes exists, matplotlib will automatically create a window and its contained axes.

For the detailed examples of the drawing:

`<https://nbviewer.jupyter.org/github/fasiondog/hikyuu/blob/master/hikyuu/examples/notebook/en/000-Index.ipynb?flush_cache=True>`_


Drawing the Combined Window
---------------------------

The shortcut functions for creating the combined window return the corresponding axes object or tuple.

.. py:function:: create_figure([n = 1, figsize = (10,8)])

    Generate a window containing the specified number of the axes; at most 4 axes are supported.
    
    :param int n: the number of the axes
    :param figsize: (width, height)
    :return: (ax1, ax2, ...) depending on the specified number of the axes; when it exceeds [1, 4] axes, return None    
    
    
    
The Convenient Functions for Drawing
------------------------------------
    
.. py:function:: ax_set_locator_formatter(axes, dates, typ)

    Set the date display of the specified axes, optimizing the X-axis display according to the specified K-line type
    
    :param axes: the specified axes
    :param dates: an iterable sequence composed of Datetime
    :param Query.KType typ: the K-line type
    

.. py:function:: adjust_axes_show(axeslist)

    Used to adjust the display of the axes closely connected vertically, solving the problem that the minimum scale of the upper axes and the maximum scale of the lower axes are displayed overlapping
    
    :param axeslist: the list of the vertically connected axes (ax1, ax2, ...)
 

.. py:function:: ax_draw_macd(axes, kdata[, n1=12, n2=26, n3=9])

    Draw the MACD
    
    :param axes: the specified axes
    :param KData kdata: KData
    :param int n1: the parameter 1 of the MACD indicator
    :param int n2: the parameter 2 of the MACD indicator
    :param int n3: the parameter 3 of the MACD indicator
 

.. py:function:: ax_draw_macd2(axes, ref, kdata, n1=12, n2=26, n3=9)

    Draw the MACD.
    When the change of the BAR value is inconsistent with the change of the reference sequence ref, it is displayed in gray;
    when the BAR and the reference sequence ref both rise, it is displayed in red;
    when the BAR and the reference sequence ref both fall, it is displayed in green.

    :param axes: the specified axes
    :param ref: the reference sequence, EMA
    :param KData kdata: KData
    :param int n1: the parameter 1 of the MACD indicator
    :param int n2: the parameter 2 of the MACD indicator
    :param int n3: the parameter 3 of the MACD indicator


TDX-compatible Plotting Functions
---------------------------------

The TDX-compatible plotting functions (only supporting the matplotlib engine)

.. py:function:: RGB(r, g, b)

    The color RGB value, e.g. RGB(255, 0, 0)


.. py:function:: STICKLINE(cond: Indicator, price1: Indicator, price2: Indicator, width: int = 2.0,
              empty: bool = False, color='m', alpha=1.0, kdata=None, new=False, axes=None)

    When the condition cond is satisfied, draw a bar chart with the width width between price1 and price2.

    Note: cond, price1, price2 should contain data; otherwise, please specify kdata as the context of the indicator calculation

    :param Indicator cond: the condition expression, used to determine whether to draw the bar line
    :param Indicator price1: the first price
    :param Indicator price2: the second price
    :param int width: (optional) the bar width. Defaults to 2.0.
    :param bool empty: (optional): hollow. Defaults to False.
    :param KData kdata: (optional): the specified context K-line. Defaults to None.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): draw in the specified axes. Defaults to None.
    :param str color: (optional): the color. Defaults to 'm'.
    :param float alpha: (optional): the transparency. Defaults to 1.0. 


.. py:function:: DRAWBAND(val1: Indicator, color1='m', val2: Indicator = None, color2='b', kdata=None, alpha=0.2, new=False, axes=None, linestyle='-')

    Draw the band line.

    Usage: DRAWBAND(val1, color1, val2, color2); when val1 > val2, fill color1 between val1 and val2;
    when val1 < val2, fill color2; the colors here all use the matplotlib color codes.
    For example: DRAWBAND(OPEN, 'r', CLOSE, 'b')

    :param Indicator val1: the indicator 1
    :param str color1: (optional): the color 1. Defaults to 'm'.
    :param Indicator val2: (optional): the indicator 2. Defaults to None.
    :param str color2: (optional): the color 2. Defaults to 'b'.
    :param KData kdata: (optional): specify the indicator context. Defaults to None.
    :param float alpha: (optional): the transparency. Defaults to 0.2.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): draw in the specified axes. Defaults to None.
    :param str linestyle: (optional): the envelope line type. Defaults to '-'.


.. py:function:: PLOYLINE(cond: Indicator, price: Indicator, kdata: KData = None, color: str = 'm', linewidth=1.0, new=False, axes=None, *args, **kwargs)


    Draw the polyline segments on the figure.

    Usage: PLOYLINE(COND, PRICE); when the COND condition is satisfied, draw the polyline connections with the PRICE position as the vertex.
    For example: PLOYLINE(HIGH>=HHV(HIGH,20),HIGH, kdata=k) means drawing the polylines between the points making a new 20-day high.

    :param Indicator cond: the specified condition
    :param Indicator price: the position
    :param KData kdata: (optional): the specified context. Defaults to None.
    :param str color: (optional): the color. Defaults to 'b'.
    :param float linewidth: (optional): the width. Defaults to 1.0.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.


.. py:function:: DRAWLINE(cond1: Indicator, price1: Indicator, cond2: Indicator, price2: Indicator, expand: int = 0, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    Draw the straight line segments on the figure.

    Usage: DRAWLINE(cond1, price1, cond2, price2, expand)
    When the COND1 condition is satisfied, draw the start point of the straight line at the PRICE1 position; when the COND2 condition is satisfied, draw the end point of the straight line at the PRICE2 position; EXPAND is the extension type.
    For example: DRAWLINE(HIGH>=HHV(HIGH,20),HIGH,LOW<=LLV(LOW,20),LOW,1) means drawing a straight line between the point making a new 20-day high and the point making a new 20-day low and extending it to the right.

    :param Indicator cond1: the condition 1
    :param Indicator price1: the position 1
    :param Indicator cond2: the condition 2
    :param Indicator price2: the position 2
    :param int expand: (optional): 0: no extension | 1: extend to the right | 10: extend to the left | 11: extend in both directions. Defaults to 0.
    :param KData kdata: (optional): the specified context. Defaults to None.
    :param str color: (optional): the specified color. Defaults to 'm'.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.


.. py:function:: DRAWTEXT(cond: Indicator, price: Indicator, text: str, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    Display the text on the figure.

    Usage: DRAWTEXT(cond, price, text); when the cond condition is satisfied, write the text at the price position.
    For example: DRAWTEXT(CLOSE/OPEN>1.08,LOW,'大阳线') means that when the real body of the bullish candle of the day is greater than 8%, display the text '大阳线' at the lowest price position.

    :param Indicator cond: the condition
    :param Indicator price: the display position
    :param str text: the text to display
    :param KData kdata: (optional): the specified context. Defaults to None.
    :param str color: (optional): the specified color. Defaults to 'm'.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.


.. py:function:: DRAWTEXT_FIX(cond: Indicator, x: float, y: float,  type: int, text: str, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    Display the text at a fixed position.

    Usage: DRAWTEXT_FIX(cond, x, y, text); the cond generally needs to add ISLASTBAR; when the cond condition is satisfied,
    write the text at the (X, Y) position within the current indicator window; X, Y are the percentages of the writing point relative to the top-left corner of the window.

    For example: DRAWTEXT_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08),0.5,0.5,0,'大阳线') means that when the real body of the bullish candle of the last trading day
    is greater than 8%, display the text '大阳线' at the middle position of the window.

    :param Indicator cond: the condition
    :param float x: the x-axis coordinate
    :param float y: the y-axis coordinate
    :param int type: (optional): 0 left-aligned | 1 right-aligned. 
    :param str text: the text to display
    :param KData kdata: (optional): the specified context. Defaults to None.
    :param str color: (optional): the specified color. Defaults to 'm'.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.


.. py:function:: DRAWNUMBER(cond: Indicator, price: Indicator, number: Indicator, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    Draw the number.

    Usage: DRAWNUMBER(cond, price, number); when the cond condition is satisfied, write the number at the price position.
    For example: DRAWNUMBER(CLOSE/OPEN>1.08,LOW,C) means that when the real body of the bullish candle of the day is greater than 8%, display the close price at the lowest price position.

    :param Indicator cond:: the condition
    :param Indicator price: the drawing position
    :param Indicator number: the number to draw
    :param KData kdata: (optional): the specified context. Defaults to None.
    :param str color: (optional): the specified color. Defaults to 'm'.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.


.. py:function:: DRAWNUMBER_FIX(cond: Indicator, x: float, y: float, type: int, number: float, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    Display the number at a fixed position.

    Usage: DRAWNUMBER_FIX(cond, x, y, type, number); the cond generally needs to add ISLASTBAR; when the cond condition is satisfied,
    write the number at the (x, y) position within the current indicator window; x, y are the percentages of the writing point relative to the top-left corner of the window; type: 0 is left-aligned, 1 is right-aligned.

    For example: DRAWNUMBER_FIX(ISLASTBAR() & (CLOSE/OPEN>1.08), 0.5,0.5,0,C) means that when the real body of the bullish candle of the last trading day is greater than 8%, display the close price at the middle position of the window.

    :param Indicator cond: the condition
    :param float x: the x-axis coordinate
    :param float y: the y-axis coordinate
    :param int type: the alignment; 0 is left-aligned, 1 is right-aligned
    :param Indicator number: the number to draw
    :param KData kdata: (optional): the specified context. Defaults to None.
    :param str color: (optional): the specified color. Defaults to 'm'.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.


.. py:function:: DRAWSL(cond: Indicator, price: Indicator, slope: Union[Indicator, float, int], length: Union[Indicator, float, int], direct: int, kdata: KData = None, color: str = 'm', new=False, axes=None, *args, **kwargs)

    Draw the slanted line.

    Usage: DRAWSL(cond, price, slope, length, direct); when the cond condition is satisfied, draw a slanted line at the price position; slope is the slope,
    length is the length, direct being 0 extends to the right, 1 extends to the left, and 2 extends in both directions.

    Notes:
    1. The vertical height difference between the K-lines is slope;
    2. When slope is 0, it is a horizontal line;
    3. When slope is 10000, it is a vertical line, length is the pixel height upward, and direct indicates extending upward or downward;
    4. slope and length support the variables;

    :param Indicator cond: the condition indicator
    :param Indicator price:: the price
    :param slope: the slope
    :param length: the length
    :param int direct: the direction
    :param KData kdata: (optional): the specified context. Defaults to None.
    :param str color: (optional): the color. Defaults to 'm'.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.


.. py:function:: DRAWIMG(cond: Indicator, price: Indicator, img: str, kdata: KData = None, new=False, axes=None, *args, **kwargs)    

    Draw the image.

    Usage: DRAWIMG(cond, price, 'image file name'); when the condition cond is satisfied, draw the specified image at the price position.
    For example: DRAWIMG(O>C,CLOSE, '123.png').

    :param Indicator cond: the specified condition
    :param Indicator price: the specified price
    :param str img: the image file name
    :param KData kdata: (optional): specify the context. Defaults to None.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): draw in the specified axes. Defaults to None.


.. py:function:: DRAWICON(cond: Indicator, price: Indicator, type: int, kdata: KData = None, new=False, axes=None, *args, **kwargs)

    Draw the built-in icons


.. py:function:: SHOWICONS()

    Display all the built-in icons


.. py:function:: DRAWRECTREL(left: int, top: int, right: int, bottom: int, color='m', frame=True, fill=True, alpha=0.1, new=False, axes=None, *args, **kwargs)

    Draw a rectangle at the relative position.

    Note: the origin is the top-left corner of the axes (0, 0), which is different from matplotlib.
    Usage: DRAWRECTREL(left, top, right, bottom, color); draw a rectangle with (left, top) of the figure window as the top-left corner and (right, bottom) as the bottom-right corner; the coordinate unit is 1/1000 of the window along the horizontal and the vertical directions, and the value range is 0-999; exceeding the range may be displayed outside the figure window; the middle of the rectangle is filled with the color COLOR, and COLOR being 0 means no filling.

    For example: DRAWRECTREL(0,0,500,500,RGB(255,255,0)) means drawing a rectangle in yellow at the top-left 1/4 position of the figure.

    :param int left: the top-left x
    :param int top: the top-left y
    :param int right: the bottom-right x
    :param int bottom: the bottom-right y
    :param str color: (optional): the specified color. Defaults to 'm'.
    :param bool frame: (optional): add the frame. Defaults to False.
    :param bool fill: (optional): the color filling. Defaults to True.
    :param float alpha: (optional): the transparency. Defaults to 0.1.
    :param bool new: (optional): draw in a new window. Defaults to False.
    :param axes: (optional): the specified axes. Defaults to None.



Drawing the K-lines, the Indicators, the Signals, etc.
------------------------------------------------------

The following functions have been set as the plot functions of the corresponding type objects; e.g. KData kdata can directly call kdata.plot(), which is equivalent to kplot() without the first parameter.
    
.. py:function:: kplot(kdata[, new=True, axes=None, colorup='r', colordown='g', width=0.6, alpha=1.0])

    Draw the K-line chart
    
    :param KData kdata: the K-line data
    :param bool new:    whether to display in a new window, only taking effect when the axes is not specified
    :param axes:        the specified axes
    :param colorup:     the color of the rectangle where close >= open
    :param colordown:   the color of the rectangle where close < open
    :param width:       the fraction of a day for the rectangle width
    :param alpha:       the rectangle alpha level, the transparency (0.0~1.0); 1.0 is opaque


.. py:function:: mkplot(kdata[, new=True, axes=None, colorup='r', colordown='g', ticksize=3])

    Draw the American-style candlestick chart
    
    :param KData kdata: the K-line data
    :param bool new:    whether to display in a new window, only taking effect when the axes is not specified
    :param axes:        the specified axes
    :param colorup:     the color of the lines where close >= open
    :param colordown:   the color of the lines where close < open
    :param ticksize:    the open/close tick marker in points

    
.. py:function:: iplot(indicator[, new=True, axes=None, legend_on=False, text_on=False, text_color='k', zero_on=False, label=None, *args, **kwargs])
          
    Draw the indicator curve chart
    
    :param Indicator indicator: the indicator instance
    :param new:             whether to display in a new window, only taking effect when the axes is not specified
    :param axes:            the specified axes
    :param legend_on:       whether to turn on the legend
    :param text_on:         whether to display the indicator name and its parameters in the top-left corner
    :param text_color:      the color of the explanatory text of the indicator name, defaulting to black
    :param zero_on:         whether to draw a straight line on the y=0 axis
    :param str label:       the text information displayed by the label, taking effect when text_on and legend_on are True
    :param args:            the pylab plot parameters
    :param kwargs:          the pylab plot parameters, such as: marker (the marker type), markerfacecolor (the marker color), markeredgecolor (the edge color of the marker)
    

.. py:function:: ibar(indicator[, new=True, axes=None, legend_on=False, text_on=False, text_color='k', label=None, width=0.4, color='r', edgecolor='r', zero_on=False, *args, **kwargs])

    Draw the indicator bar chart
    
    :param Indicator indicator: the Indicator instance
    :param axes:       the specified axes
    :param new:        whether to display in a new window, only taking effect when the axes is not specified
    :param legend_on:  whether to turn on the legend
    :param text_on:    whether to display the indicator name and its parameters in the top-left corner
    :param text_color: the color of the explanatory text of the indicator name, defaulting to black
    :param str label:  the text information displayed by the label, taking effect when text_on and legend_on are True
    :param zero_on:    whether to draw a straight line on the y=0 axis
    :param width:      the width of the Bar
    :param color:      the color of the Bar
    :param edgecolor:  the edge color of the Bar
    :param args:       the pylab plot parameters
    :param kwargs:     the pylab plot parameters

    
.. py:function:: iheatmap(ind, axes=None):

    Draw the indicator year-month return heatmap. Not all indicators can draw a heatmap; see the heatmap formula for the details:

    The indicator return rate = (the current month-end value - the last month-end value) / the last month-end value * 100

    The indicator should have been calculated (i.e. has values) and be a time series

    :param ind: the specified indicator
    :param axes: the axes object to draw, defaulting to None, which means creating a new axes object
    :return: None


.. py:function:: sgplot(sg[, new = True, axes = None,  style = 1, kdata = None])

    Draw the buy/sell signals

    :param SignalBase sg: the signal generator
    :param new:   only taking effect when the axes is not specified; when True, create a new window object and draw in it
    :param axes:  specify in which axes object to draw
    :param style: 1 | 2 the signal arrow drawing style
    :param KData kdata: the specified KData (i.e. the traded K-line data (TO) of the signal generator); if this value is None, it is considered that the signal generator has already specified the traded K-line data (TO); otherwise, use this parameter as the traded K-line data (TO)


.. py:function:: cnplot(cn[, new=True, axes=None, kdata=None, upcolor='red', downcolor='blue', alpha=0.2])

    Draw the system valid condition

    :param ConditionBase cn: the system valid condition
    :param new:  only taking effect when the axes is not specified; when True, create a new window object and draw in it
    :param axes: specify in which axes object to draw
    :param KData kdata: the specified KData; if this value is None, it is considered that the system valid condition has already specified the traded K-line data (TO); otherwise, use this parameter as the traded K-line data (TO)
    :param upcolor: the color when valid
    :param downcolor: the color when invalid
    :param alpha: the transparency    


.. py:function:: evplot(ev, ref_kdata, new=True, axes=None, upcolor='red', downcolor='blue', alpha=0.2)

    Draw the market valid judgment

    :param EnvironmentBase cn: the system valid condition
    :param KData ref_kdata: used as the date reference
    :param new: only taking effect when the axes is not specified; when True, create a new window object and draw in it
    :param axes: specify in which axes object to draw
    :param upcolor: the color when valid
    :param downcolor: the color when invalid
    :param alpha: the transparency

    
.. py:function:: sysplot(sys[, new=True, axes=None, style=1])

    Draw the actual buy/sell signals of the system
    
    :param SystemBase sys: the system instance
    :param new:   only taking effect when the axes is not specified; when True, create a new window object and draw in it
    :param axes:  specify in which axes object to draw
    :param style: 1 | 2 the signal arrow drawing style


.. py:function:: sys_performance(sys, ref_stk=None)

    Draw the system performance, i.e. the account cumulative return curve. It is usually not called directly, but to view the performance details after the sys, pf calculation is completed.
    E.g.: sys.performance()

    :param SystemBase | PortfolioBase sys: the SYS or PF instance
    :param Stock ref_stk: the reference stock, defaulting to the CSI 300: sh000300, drawing the return curve of the reference target
    

.. py:function:: sys_heatmap(sys, axes=None):

    Draw the system year-month return heatmap

    :param sys: the SYS or PF instance
    :param axes: the axes object to draw, defaulting to None, which means creating a new axes object


Built-in Examples
-----------------

.. py:function:: vl.draw(stock, query=Query(-130), ma1_n=5, ma2_n=10, ma3_n=20, ma4_n=60, ma5_n=100, ma_type="SMA", vma1_n=5, vma2_n=10)

    Draw the ordinary K-line chart + the volume (the amount)


.. py:function:: vl.draw2(stock, query=Query(-130), ma1_n=7, ma2_n=20, ma3_n=30, ma4_n=42, ma5_n=100, vma1_n=5, vma2_n=10) 

    Draw the ordinary K-line chart + the volume (the amount) + MACD

    
.. py:function:: el.draw(stock, query=QueryByIndex(-130), ma_n=22, ma_w='auto', vigor_n=13)

    Draw the figure of the Alexander Elder trading system. See [BOOK2]_
    
    
.. py:function:: kf.draw(stock, query=Query(-130), n=10, filter_n=20, filter_p=0.1, sg_type = "CROSS", show_high_low=False,  arrow_style=1)

    Draw the adaptive moving average system (AMA) of Perry J. Kaufman. See [BOOK1]_
