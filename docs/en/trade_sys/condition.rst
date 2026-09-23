.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

System Validity Condition|CN
============================

Built-in System Validity Conditions
-----------------------------------

.. py:function:: CN_OPLine(ind)

    Trades are always sized at the instrument's minimum tradable quantity. The condition computes the op value of the equity curve: the system is valid while the equity curve is above op, and invalid otherwise.

    :param Indicator ind: an Indicator instance
    :return: the system validity condition instance
    :rtype: ConditionBase

.. py:function:: CN_Bool(ind)

    A system validity condition driven by a boolean indicator: the system is valid where the indicator's value is greater than 0, and invalid elsewhere.

    :param Indicator ind: a boolean indicator computed over KData (bar data)
    :return: the system validity condition instance
    :rtype: ConditionBase


Custom System Validity Condition
--------------------------------

Quickly define a custom system validity condition that holds no private attributes.

.. py:function:: crtCN(func, params={}, name='crtSG')

    Quickly create a custom system validity condition that holds no private attributes
    
    :param func: the system validity condition function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom system validity condition instance

The custom system validity condition interface:

* :py:meth:`ConditionBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`ConditionBase._clone` - [Required] The clone interface
* :py:meth:`ConditionBase._reset` - [Optional] Reset the internal member variables


System Validity Condition Base Class
------------------------------------

.. py:class:: ConditionBase

    Base class for system validity conditions
    
    .. py:attribute:: name The condition name
    .. py:attribute:: to Set or get the traded KData (trade object, TO)
    .. py:attribute:: tm Set or get the associated trade manager account
    .. py:attribute:: sg Set or get the associated signal generator
    
    .. py:method:: __init__(self[, name="ConditionBase"])
    
        Constructor
        
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
    
        Whether the system is valid at the given time
        
        :param Datetime datetime: the specified time
        :return: True means valid | False means invalid

    .. py:method:: get_datetime_list(self)

        Return the dates on which the system is valid. Note that the list contains only valid dates, so its length does not match that of the traded KData (TO).

    .. py:method:: get_values(self)

        Return the validity values as an indicator, aligned in length with the traded KData (TO); 0 means invalid, and 1 means valid

    .. py:method:: _add_valid(self, datetime)
    
        Register a time as valid; called from _calculate
        
        :param Datetime datetime: the valid time
      
    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation        
        
    .. py:method:: _calculate(self)
    
        [Override hook] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Override hook] The subclass reset interface; resets the internal private variables
    
    .. py:method:: _clone(self)
    
        [Override hook] The subclass clone interface
