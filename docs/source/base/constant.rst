.. py:currentmodule:: hikyuu
.. highlight:: python

全局变量与常量定义
===================

全局变量
---------

.. note:: 以下全局变量仅在 hikyuu 交互式工具中存在，hikyuu库中并无定义。

.. py:data:: sm

    :py:class:`StockManager` 的实例
    
.. py:data:: blocka

    :py:class:`Block` 实例，包含全部A股
    
.. py:data:: blocksh

    :py:class:`Block` 实例，包含全部沪市股票
    
.. py:data:: blocksz

    :py:class:`Block` 实例，包含全部深市股票
    
.. py:data:: blockg

    :py:class:`Block` 实例，包含全部创业板股票

.. py:data:: blockstart

    :py:class:`Block` 实例，科创板

.. py:data:: blockzxb

    :py:class:`Block` 实例，中小板

.. py:data:: zsbk_sh50

    :py:class:`Block` 实例，上证500

.. py:data:: zsbk_sh180

    :py:class:`Block` 实例，上证180

.. py:data:: zsbk_hs300

    :py:class:`Block` 实例，沪深300

.. py:data:: zsbk_zz100

    :py:class:`Block` 实例，中证100


Null 值及证券类别
-------------------

.. py:data:: constant

    全局常量, :py:class:`Constant` 的实例，用于判断相关的 null 值及股票类型，如：
    
    ::
        
        a = Datetime(201601010000)
        if (a == constant.null_datetime ):
            print(True)

.. py:class:: Constant

    .. py:attribute:: null_datetime 无效Datetime
    
    .. py:attribute:: inf

    .. py:attribute:: nan

    .. py:attribute:: null_price 同 nan

    .. py:attribute:: null_int 无效int
    
    .. py:attribute:: null_size 无效size
     
    .. py:attribute:: null_int64 无效int64
    
    .. py:attribute:: pickle_support 是否支持 pickle
    
    .. py:attribute:: STOCKTYPE_BLOCK 股票类型-板块
    
    .. py:attribute:: STOCKTYPE_A 股票类型-A股
    
    .. py:attribute:: STOCKTYPE_INDEX 股票类型-指数
    
    .. py:attribute:: STOCKTYPE_B 股票类型-B股
    
    .. py:attribute:: STOCKTYPE_FUND 股票类型-基金
    
    .. py:attribute:: STOCKTYPE_ETF 股票类型-ETF
    
    .. py:attribute:: STOCKTYPE_ND 股票类型-国债
    
    .. py:attribute:: STOCKTYPE_BOND 股票类型-其他债券
    
    .. py:attribute:: STOCKTYPE_GEM 股票类型-创业板

    .. py:attribute:: STOCKTYPE_START 股票类型-创业板

    .. py:attribute:: STOCKTYPE_A_BJ 股票类型-A股北交所
    
    .. py:attribute:: STOCKTYPE_TMP 股票类型-临时CSV
