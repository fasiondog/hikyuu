.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Trade Cost Algorithms
=====================

Built-in Trade Cost Algorithms
------------------------------

Zero Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: TC_Zero()

    :return: an instance of a :py:class:`TradeCostBase` subclass


Shanghai-Shenzhen A-share Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Before August 1, 2015, the Shanghai transfer fee was one thousandth of the trade number; if it was less than 1 yuan, it was counted as 1 yuan.
* After August 1, 2015, the Shanghai transfer fee is 0.02 thousandths of the transaction amount.

The calculation rules are as follows:: python

    1) Shanghai Stock Exchange
        Buy: commission + transfer fee
        Sell: commission + transfer fee + stamp tax
    2) Shenzhen Stock Exchange:
        Buy: commission
        Sell: commission + stamp tax

    Among them, the minimum commission is 5 yuan.

.. py:function:: TC_FixedA2015([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.00002])

    The A-share trade cost algorithm for August 1, 2015 and later; the Shanghai transfer fee was changed to 0.02 thousandths of the transaction amount

    :param float commission: the commission ratio
    :param float lowestCommission: the minimum commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :return: an instance of a :py:class:`TradeCostBase` subclass
    
.. py:function:: TC_FixedA([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.001, lowestTransferfee=1.0])

    The A-share trade cost algorithm before August 1, 2015

    :param float commission: the commission ratio
    :param float lowestCommission: the minimum commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :param float lowestTransferfee: the minimum transfer fee
    :return: an instance of a :py:class:`TradeCostBase` subclass


ETF Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^

The calculation rules are as follows:: python

    Buy: commission (minimum 5 yuan)
    Sell: commission (minimum 5 yuan)
    
    No stamp tax and transfer fee.

.. py:function:: TC_FixedETF([commission=0.0001, lowestCommission=5.0])

    The ETF trade cost algorithm; the commission is charged in both directions of buying and selling, with no stamp tax and transfer fee

    :param float commission: the commission ratio, defaults to one ten-thousandth
    :param float lowestCommission: the minimum commission value, defaults to 5 yuan per trade
    :return: an instance of a :py:class:`TradeCostBase` subclass


Custom Trade Cost Algorithm
---------------------------

The custom trade cost algorithm interface:

* :py:meth:`TradeCostBase.getBuyCost` - [Required] Get the buy cost
* :py:meth:`TradeCostBase.getSellCost` - [Required] Get the sell cost
* :py:meth:`TradeCostBase._clone` - [Required] The subclass clone interface



Trade Cost Algorithm Base Class
-------------------------------

.. py:class:: TradeCostBase(name)

    The trade cost algorithm base class
    
    .. py:attribute:: name Name
        
    .. py:method:: get_param(self, name)

        Get the specified parameter
    
        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter
        
    .. py:method:: set_param(self, name, value)
    
        Set the parameter
        
        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported

    .. py:method:: clone(self)
    
        The clone operation

    .. py:method:: get_buy_cost(self, datetime, stock, price, num)
    
        [Override hook] Get the buy cost
        
        :param Datetime datetime: the buy moment
        :param Stock stock: the buy object
        :param float price: the buy price
        :param int num: the buy number
        :return: the trade cost record
        :rtype: CostRecord
    
    .. py:method:: get_sell_cost(self, datetime, stock, price, num)
    
        [Override hook] Get the sell cost
        
        :param Datetime datetime: the sell moment
        :param Stock stock: the sell object
        :param float price: the sell price
        :param int num: the sell number
        :return: the trade cost record
        :rtype: CostRecord
        
    .. py:method:: _clone(self)
    
        [Override hook] The subclass clone interface
