.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

TA-Lib Indicators
=================

Hikyuu has built in all the ta-lib indicators, which can be used directly; the naming convention is uniformly the TA_FUNC name. For details, please refer to the official Ta-lib documentation.

::

    x = TA_SMA(CLOSE(k))
    print(x)
    x.plot()
    print(x.discard)
