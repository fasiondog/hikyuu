.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

TA-Lib 包装
============

.. note::
    
    talib 包无法在 Windows 下通过 pip 安装，请从 `<https://www.lfd.uci.edu/~gohlke/pythonlibs/#ta-lib/>`_ 上下载相应的 wheel 包，手工安装。


在交互工具里对TA-Lib进行了包装，做为对 hikyuu 指标的补充，其命名方式统一为 TA_FUNC名称。其中，ta-lib指标的lookback属性，用discard属性代替。

::

    x = TA_SMA(CLOSE(k))
    print(x)
    x.plot()
    print(x.discard)

个别的Ta-Lib函数需要两个数组参数，比如BETA、CORREL。此时需要利用特殊的 Indicator WEAVE 将两个数组包装到一个 ind 对象中。

::

    query = Query(-200)
    k1 = sm['sh000001'].getKData(query)
    k2 = sm['sz000001'].getKData(query)

    w = WEAVE(CLOSE(k1))
    w = w(CLOSE(k2))
    print(w.getResultNumber())

    cr = TA_CORREL(w)
    cr.plot()    
    