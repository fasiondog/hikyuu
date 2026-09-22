.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Slippage Algorithm|SP
=====================

Built-in Slippage Algorithms
----------------------------

.. py:function:: SP_FixedPercent([p=0.001])

    The fixed percentage slippage algorithm; the actual buy price = the planned buy price * (1 + p), the actual sell price = the planned sell price * (1 - p)
    
    :param float p: the fixed percentage of the offset
    :return: the slippage algorithm instance


.. py:function:: SP_FixedValuet([p=0.001])

    The fixed price slippage algorithm; the actual buy price = the planned buy price + the offset price, the actual sell price = the planned sell price - the offset price
    
    :param float p: the offset price
    :return: the slippage algorithm instance

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05])

    The uniformly distributed random price slippage algorithm; for the buy and sell operations the price is a uniformly distributed random offset in the range [min_value, max_value]
    
    :param float min_value: the minimum offset price
    :param float max_value: the maximum offset price
    :return: the slippage algorithm instance

.. py:function:: SP_Normal([mean=0.0, stddev=0.05])

    The normally distributed random price slippage algorithm; for the buy and sell operations the price is a random offset in the normal distribution range [mean, stddev]
    
    :param float mean: the mean of the normal distribution
    :param float stddev: the standard deviation of the normal distribution
    :return: the slippage algorithm instance

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05])

    The log-normal distributed random price slippage algorithm; for the buy and sell operations the price is a random offset in the log-normal distribution range [mean, stddev]

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution
    :return: the slippage algorithm instance

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])
         
    The truncated normal distributed random price slippage algorithm; for the buy and sell operations the price is a random offset in the truncated normal distribution range [mean, stddev, min_value, max_value]
    
    :param float mean: the mean of the truncated normal distribution
    :param float stddev: the standard deviation of the truncated normal distribution
    :param float min_value: the minimum truncation value
    :param float max_value: the maximum truncation value
    :return: the slippage algorithm instance
    

Custom Slippage Algorithm
-------------------------

The custom slippage interface:

* :py:meth:`SlippageBase.get_real_buy_price` - [Required] Calculate the actual buy price
* :py:meth:`SlippageBase.get_real_sell_price` - [Required] Calculate the actual sell price
* :py:meth:`SlippageBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`SlippageBase._clone` - [Required] The clone interface
* :py:meth:`SlippageBase._reset` - [Optional] Reload the private variables


Slippage Algorithm Base Class
-----------------------------

.. py:class:: SlippageBase

    The slippage algorithm base class
    
    .. py:attribute:: name Name
    
    .. py:method:: __init__(self[, name="SlippageBase"])
    
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
        
    .. py:method:: get_real_buy_price(self, datetime, price)

        [Overload interface] Calculate the actual buy price
        
        :param Datetime datetime: the buy time
        :param float price: the planned buy price
        :return: the actual buy price
        :rtype: float
        
    .. py:method:: get_real_sell_price(self, datetime, price)

        [Overload interface] Calculate the actual sell price
        
        :param Datetime datetime: the sell time
        :param float price: the planned sell price
        :return: the actual sell price
        :rtype: float        

    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation        
        
    .. py:method:: _calculate(self)
    
        [Overload interface] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Overload interface] The subclass reset interface, resetting the internal private variables
    
    .. py:method:: _clone(self)
    
        [Overload interface] The subclass clone interface
