.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

TA-Lib 指标
============

Hikyuu已经内置了所有 ta-lib 指标，可以直接使用，其命名方式统一为 TA_FUNC名称。具体可以参考 Ta-lib 的官方文档。

::

    x = TA_SMA(CLOSE(k))
    print(x)
    x.plot()
    print(x.discard)

    