.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Profit Goal Strategy|PG
=======================

Built-in Profit Goal Strategies
-------------------------------

.. py:function:: PG_FixedPercent([p = 0.2])

    The fixed percentage profit goal; the goal price = the buy price * (1 + p)
    
    :param float p: the percentage
    :return: the profit goal strategy instance
    
.. py:function:: PG_FixedHoldDays([days=5])

    The fixed holding days profit goal strategy
    
    :param int days: the allowed holding days (counted by trading days), defaults to 5 days
    :return: the profit goal strategy instance
    
.. py:function:: PG_NoGoal()

    No profit goal strategy, usually for testing or comparison.
    
    :return: the profit goal strategy instance

    
Custom Profit Goal Strategy
---------------------------    

The custom profit goal strategy interface:

* :py:meth:`ProfitGoalBase.getGoal` - [Required] Get the goal price
* :py:meth:`ProfitGoalBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`ProfitGoalBase._clone` - [Required] The clone interface
* :py:meth:`ProfitGoalBase._reset` - [Optional] Reload the private variables
* :py:meth:`ProfitGoalBase.buyNotify` - [Optional] Receive the notification of the actual buy; reserved for handling multiple position increases/decreases
* :py:meth:`ProfitGoalBase.sellNotify` - [Optional] Receive the notification of the actual sell; reserved for handling multiple position increases/decreases
    

Profit Goal Strategy Base Class
-------------------------------

.. py:class:: ProfitGoalBase

    The profit goal strategy base class
    
    .. py:attribute:: name Name
    .. py:attribute:: to Set or get the trading object
    .. py:attribute:: tm Set or get the trade management account
    
    .. py:method:: __init__(self[, name="ProfitGoalBase"])
    
        The initialization constructor
        
        :param str name: the name
        
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
        
    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation        
        
    .. py:method:: get_goal(self, datetime, price)
    
        [Overload interface] Get the profit goal price; returning constant.null_price means no goal is set, and returning 0 means it needs to be sold
        
        :param Datetime datetime: the current time
        :param float price: the current price
        :return: the goal price
        :rtype: float
        
    .. py:method:: buy_notify(self, trade_record)
    
        [Overload interface] When the trade system performs an actual buy operation, notify the trade change; it generally needs to be overloaded only when there are multiple position increases/decreases
        
        :param TradeRecord trade_record: the actual buy trade record when the actual buy occurs
        
    .. py:method:: sell_notify(self, trade_record)
    
        [Overload interface] When the trade system performs an actual sell operation, notify the actual trade change; it generally needs to be overloaded only when there are multiple position increases/decreases
        
        :param TradeRecord trade_record: the actual sell trade record when the actual sell occurs
         
    .. py:method:: _calculate(self)
    
        [Overload interface] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Overload interface] The subclass reset interface, resetting the internal private variables
    
    .. py:method:: _clone(self)
    
        [Overload interface] The subclass clone interface        
