.. py:currentmodule:: hikyuu
.. highlight:: python

Global Variables and Constants
==============================

Global Variables
----------------

.. note:: The following global variables exist only in the hikyuu interactive tools; they are not defined in the hikyuu library.

.. py:data:: sm

    An instance of :py:class:`StockManager`
    
.. py:data:: blocka

    A :py:class:`Block` instance, containing all the A-shares
    
.. py:data:: blocksh

    A :py:class:`Block` instance, containing all the Shanghai stocks
    
.. py:data:: blocksz

    A :py:class:`Block` instance, containing all the Shenzhen stocks
    
.. py:data:: blockg

    A :py:class:`Block` instance, containing all the ChiNext (GEM) stocks

.. py:data:: blockstart

    A :py:class:`Block` instance, the STAR Market

.. py:data:: blockzxb

    A :py:class:`Block` instance, the SME Board

.. py:data:: zsbk_sh50

    A :py:class:`Block` instance, the SSE 50

.. py:data:: zsbk_sh180

    A :py:class:`Block` instance, the SSE 180

.. py:data:: zsbk_hs300

    A :py:class:`Block` instance, the CSI 300

.. py:data:: zsbk_zz100

    A :py:class:`Block` instance, the CSI 100


Null Values and Security Types
------------------------------

.. py:data:: constant

    The global constants, an instance of :py:class:`Constant`, used to judge the related null values and the stock types, e.g.:
    
    ::
        
        a = Datetime(201601010000)
        if (a == constant.null_datetime ):
            print(True)

.. py:class:: Constant

    .. py:attribute:: null_datetime An invalid Datetime
    
    .. py:attribute:: inf

    .. py:attribute:: nan

    .. py:attribute:: null_price The same as nan

    .. py:attribute:: null_int An invalid int
    
    .. py:attribute:: null_size An invalid size
     
    .. py:attribute:: null_int64 An invalid int64
    
    .. py:attribute:: pickle_support Whether pickle is supported
    
    .. py:attribute:: STOCKTYPE_BLOCK Stock type - Block
    
    .. py:attribute:: STOCKTYPE_A Stock type - A-share
    
    .. py:attribute:: STOCKTYPE_INDEX Stock type - Index
    
    .. py:attribute:: STOCKTYPE_B Stock type - B-share
    
    .. py:attribute:: STOCKTYPE_FUND Stock type - Fund
    
    .. py:attribute:: STOCKTYPE_ETF Stock type - ETF
    
    .. py:attribute:: STOCKTYPE_ND Stock type - Treasury bond
    
    .. py:attribute:: STOCKTYPE_BOND Stock type - Other bonds
    
    .. py:attribute:: STOCKTYPE_GEM Stock type - ChiNext (GEM)

    .. py:attribute:: STOCKTYPE_START Stock type - ChiNext (GEM)

    .. py:attribute:: STOCKTYPE_A_BJ Stock type - A-share of the Beijing Stock Exchange
    
    .. py:attribute:: STOCKTYPE_TMP Stock type - Temporary CSV
