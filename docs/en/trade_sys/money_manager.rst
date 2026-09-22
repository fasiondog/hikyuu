.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Money Management Strategy|MM
============================

Common parameters:

    * **auto-checkin=False** *(bool)* : When the account cash is not enough to buy the number indicated by the money management strategy, automatically deposit (checkin) enough cash into the account.
    * **max-stock=20000** *(int)* : The maximum number of the security types held (i.e. how many stocks are held, not the position number of each stock)
    * **disable_ev_force_clean_position=False** *(bool)* : Disable the forced position clearing when the environment is invalid
    * **disable_cn_force_clean_position=False** *(bool)* : Disable the forced position clearing when the system validity condition is invalid


Built-in Money Management Strategies
------------------------------------

No Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_Nothing()

    A special money management strategy, equivalent to no money management: buy as much as the money allows.


Fixed Trade Number Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCount([n = 100])

    The fixed trade number money management strategy. Buy a fixed number each time.
    
    :param float n: the number bought each time (it should be an integral multiple of the minimum trade number of the trading object; the program does not check this here)
    :return: the money management strategy instance

.. py:function:: MM_FixedCountTps([buy_counts, sell_counts])
          
    The money management strategy of buying/selling a fixed number consecutively.
    
    :param list buy_counts: the list of the buy numbers
    :param list sell_counts: the list of the sell numbers
    :return: the money management strategy instance


Fixed Risk Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedRisk([risk = 1000.00])

    The fixed risk money management strategy limits a predetermined or fixed money risk for each trade, such as a fixed risk of 1000 yuan per trade. Formula: trade number = fixed risk / trade risk.

    :param float risk: the fixed risk
    :return: the money management strategy instance
    

Fixed Capital Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCapital([capital = 10000.0])

    The fixed money management strategy, i.e. controlling the total money invested in each buy. Buy number = current cash / capital.

    :param float capital: the fixed capital unit
    :return: the money management strategy instance

.. py:function:: MM_FixedCapitalFunds([capital = 10000.0])

    The fixed capital money management strategy. Buy number = current total assets / capital.

    :param float capital: the fixed capital unit
    :return: the money management strategy instance    


Fixed Units Risk Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedUnits([n = 33])

    The fixed units money management strategy. Formula: buy number = current cash / n / current risk.

    :param int n: n money units
    :return: the money management strategy instance
    

Williams Fixed Risk Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  

.. py:function:: MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    The Williams fixed risk money management strategy; buy number = (account balance × risk percentage p) ÷ maximum loss (max_loss)
    
    :param float p: the risk percentage
    :param float max_loss: the maximum loss
    :return: the money management strategy instance

    
Fixed Percentage Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedPercent([p = 0.03])

    The fixed percentage risk model. Formula: P (position size) = account balance * percentage / R (the trade risk per share). [BOOK3]_, [BOOK4]_ .
    
    :param float p: the percentage
    :return: the money management strategy instance
    

Fixed Volatility Money Management Strategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


    

Custom Money Management Strategy
--------------------------------

The custom money management strategy interface:

* :py:meth:`MoneyManagerBase.buyNotify` - [Optional] Receive the notification of the actual buy; reserved for handling multiple position increases/decreases
* :py:meth:`MoneyManagerBase.sellNotify` - [Optional] Receive the notification of the actual sell; reserved for handling multiple position increases/decreases
* :py:meth:`MoneyManagerBase._getBuyNumber` - [Required] Get the number that can be bought for the specified trading object
* :py:meth:`MoneyManagerBase._getSellNumber` - [Optional] Get the number that can be sold for the specified trading object; if not overloaded, it defaults to selling all the held number
* :py:meth:`MoneyManagerBase._reset` - [Optional] Reset the private attributes
* :py:meth:`MoneyManagerBase._clone` - [Required] The clone interface

Money Management Strategy Base Class
------------------------------------

.. py:class:: MoneyManagerBase

    The money management strategy base class
    
    .. py:attribute:: name  Name
    .. py:attribute:: tm    Set or get the trade manager object
    .. py:attribute:: query Set or get the query condition
    
    .. py:method:: __init__(self[, name="MoneyManagerBase])
    
        The initialization constructor
        
        :param str name: the name
        
    .. py:method:: get_param(self, name)

        Get the specified parameter
        
        .. note::

            All the money management strategies have the parameter "auto-checkin" (bool type, defaults to False), which means "when the account cash is not enough to buy the number indicated by the money management strategy, automatically deposit (checkin) enough cash into the account."
    
        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter
        
    .. py:method:: set_param(self, name, value)
    
        Set the parameter
        
        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported
        
    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation
        
    .. py:method:: get_buy_num(self, datetime, stock, price, risk, part_from)
    
        Get the number that can be bought for the specified trading object
        
        :param Datetime datetime: the trade time
        :param Stock stock: the trading object
        :param float price: the trade price
        :param float risk: the risk taken by the trade; if it is 0, it means a total loss, i.e. the market value falls to 0
        :param System.Part part_from: the source system component
        :return: the number that can be bought
        :rtype: float
        
    .. py:method:: get_sell_num(self, datetime, stock, price, risk, part_from)
    
        Get the number that can be sold for the specified trading object
        
        :param Datetime datetime: the trade time
        :param Stock stock: the trading object
        :param float price: the trade price
        :param float risk: the risk taken by the new trade; if it is 0, it means a total loss, i.e. the market value falls to 0
        :param System.Part part_from: the source system component
        :return: the number that can be sold
        :rtype: float

    .. py:method:: current_buy_count(self, stock)

        The current consecutive buy count

    .. py:method:: current_sell_count(self, stock)

        The current consecutive sell count
        
    .. py:method:: _buy_notify(self, trade_record)
    
        [Overload interface] When the trade system performs an actual buy operation, notify the trade change; it generally needs to be overloaded only when there are multiple position increases/decreases
        
        :param TradeRecord trade_record: the actual buy trade record when the actual buy occurs
        
    .. py:method:: _sell_notify(self, trade_record)
    
        [Overload interface] When the trade system performs an actual sell operation, notify the actual trade change; it generally needs to be overloaded only when there are multiple position increases/decreases
        
        :param TradeRecord trade_record: the actual sell trade record when the actual sell occurs
    
    .. py:method:: _get_buy_num(self, datetime, stock, price, risk, part_from)

        [Overload interface] Get the number that can be bought for the specified trading object
        
        :param Datetime datetime: the trade time
        :param Stock stock: the trading object
        :param float price: the trade price
        :param float risk: the risk taken by the trade; if it is 0, it means a total loss, i.e. the market value falls to 0
        :param System.Part part_from: the source system component
        :return: the number that can be bought
        :rtype: float

    .. py:method:: _get_sell_num(self, datetime, stock, price, risk, part_from)
    
        [Overload interface] Get the number that can be sold for the specified trading object. If not overloaded, it defaults to selling all the held number.
        
        :param Datetime datetime: the trade time
        :param Stock stock: the trading object
        :param float price: the trade price
        :param float risk: the risk taken by the new trade; if it is 0, it means a total loss, i.e. the market value falls to 0
        :param System.Part part_from: the source system component
        :return: the number that can be sold
        :rtype: float
        
    .. py:method:: _reset(self)
    
        [Overload interface] The subclass reset interface, resetting the internal private variables
    
    .. py:method:: _clone(self)
    
        [Overload interface] The subclass clone interface
