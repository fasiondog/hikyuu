.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

System Validity Condition|CN
============================

Built-in System Validity Conditions
-----------------------------------

.. py:function:: CN_OPLine(ind)

    Trade with the fixed minimum trade number of the stock, and calculate the op value of the equity curve; when the equity curve is above op, the system is valid, otherwise invalid.

    :param Indicator ind: an Indicator instance
    :return: the system validity condition instance
    :rtype: ConditionBase

.. py:function:: CN_Bool(ind)

    The boolean signal generator system validity condition; a position > 0 in the indicator means the system is valid, otherwise invalid.

    :param Indicator ind: a bool-type indicator with KData as the input
    :return: the system validity condition instance
    :rtype: ConditionBase


Custom System Validity Condition
--------------------------------

Quickly create a custom system validity condition without private attributes.

.. py:function:: crtCN(func, params={}, name='crtSG')

    Quickly create a custom system validity condition without private attributes
    
    :param func: the system validity condition function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom system validity condition instance

The custom system validity condition interface:

* :py:meth:`ConditionBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`ConditionBase._clone` - [Required] The clone interface
* :py:meth:`ConditionBase._reset` - [Optional] Reload the private variables


System Validity Condition Base Class
------------------------------------

.. py:class:: ConditionBase

    The system validity condition base class
    
    .. py:attribute:: name Name
    .. py:attribute:: to Set or get the trading object
    .. py:attribute:: tm Set or get the trade management account
    .. py:attribute:: sg Set or get the trade signal generator
    
    .. py:method:: __init__(self[, name="ConditionBase"])
    
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
            
    .. py:method:: is_valid(self, datetime)
    
        Whether the system is valid at the specified time
        
        :param Datetime datetime: the specified time
        :return: True valid | False invalid

    .. py:method:: get_datetime_list(self)

        Get the dates when the system is valid. Note that it only returns the list of the dates when the system is valid, which is not the same length as the trading object.

    .. py:method:: get_values(self)

        Get the actual values in the form of an indicator, with the same length as the trading object; 0 means invalid, and 1 means the system is valid

    .. py:method:: _add_valid(self, datetime)
    
        Add a valid time, called in _calculate
        
        :param Datetime datetime: the valid time
      
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
