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

* Before August 1, 2015, the Shanghai transfer fee was 0.1% of the traded share quantity, with a minimum of 1 yuan.
* Since August 1, 2015, the Shanghai transfer fee has been 0.002% (0.02‰) of turnover.

The calculation rules are as follows:: python

    1) Shanghai Stock Exchange
        Buy: commission + transfer fee
        Sell: commission + transfer fee + stamp duty
    2) Shenzhen Stock Exchange:
        Buy: commission
        Sell: commission + stamp duty

    Among them, the minimum commission is 5 yuan.

.. py:function:: TC_FixedA2015([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.00002])

    The A-share trade cost algorithm for August 1, 2015 and later; the Shanghai transfer fee was changed to 0.002% (0.02‰) of turnover

    :param float commission: the commission rate
    :param float lowestCommission: the minimum commission
    :param float stamptax: the stamp duty rate
    :param float transferfee: the transfer fee rate
    :return: an instance of a :py:class:`TradeCostBase` subclass
    
.. py:function:: TC_FixedA([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.001, lowestTransferfee=1.0])

    The A-share trade cost algorithm before August 1, 2015

    :param float commission: the commission rate
    :param float lowestCommission: the minimum commission
    :param float stamptax: the stamp duty rate
    :param float transferfee: the transfer fee rate
    :param float lowestTransferfee: the minimum transfer fee
    :return: an instance of a :py:class:`TradeCostBase` subclass


ETF Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^

The calculation rules are as follows:: python

    Buy: commission (minimum 5 yuan)
    Sell: commission (minimum 5 yuan)
    
    No stamp duty or transfer fee.

.. py:function:: TC_FixedETF([commission=0.0001, lowestCommission=5.0])

    The ETF trade cost algorithm; commission is charged on both the buy and sell sides, with no stamp duty or transfer fee

    :param float commission: the commission rate, defaulting to one ten-thousandth (0.01%)
    :param float lowestCommission: the minimum commission, defaulting to 5 yuan per trade
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

    Base class for trade cost algorithms
    
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
    
        Clone the instance

    .. py:method:: get_buy_cost(self, datetime, stock, price, num)
    
        [Override hook] Compute the buy cost
        
        :param Datetime datetime: the buy moment
        :param Stock stock: the security bought
        :param float price: the buy price
        :param int num: the number of shares bought
        :return: the trade cost record
        :rtype: CostRecord
    
    .. py:method:: get_sell_cost(self, datetime, stock, price, num)
    
        [Override hook] Compute the sell cost
        
        :param Datetime datetime: the sell moment
        :param Stock stock: the security sold
        :param float price: the sell price
        :param int num: the number of shares sold
        :return: the trade cost record
        :rtype: CostRecord
        
    .. py:method:: _clone(self)
    
        [Override hook] Subclass clone interface
