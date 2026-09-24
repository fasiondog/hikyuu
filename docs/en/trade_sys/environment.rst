.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Market Environment Strategy|EV
=================================

Built-in Market Environment Strategies
-----------------------------------------

.. py:function:: EV_TwoLine(fast, slow[, market = 'SH'])

    Fast/slow-line strategy: the market is valid when the market index's fast line is above its slow line, and invalid otherwise.

    :param Indicator fast: the fast-line indicator
    :param Indicator slow: the slow-line indicator
    :param string market: the market name

.. py:function:: EV_Bool(ind[, market = 'SH'])

    A market environment condition driven by a boolean indicator: the market is valid where the indicator's value is greater than 0, and invalid elsewhere.

    :param Indicator ind: a boolean indicator; values greater than 0 mark the market as valid, all other values mark it as invalid
    :param str market: the market whose trading calendar is used


Custom Market Environment Strategy
-------------------------------------

The custom market environment strategy interface:

* :py:meth:`EnvironmentBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`EnvironmentBase._clone` - [Required] The clone interface
* :py:meth:`EnvironmentBase._reset` - [Optional] Reset the internal member variables

Market Environment Strategy Base Class
-----------------------------------------

.. py:class:: EnvironmentBase

    Base class for market environment strategies
    
    .. py:attribute:: name The strategy name
    .. py:attribute:: query Set or get the query conditions
    
    .. py:method:: __init__(self[, name='EnvironmentBase'])
    
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
    
        Whether the market is valid at the given time
        
        :param Datetime datetime: the specified time
        :return: True means valid | False means invalid
    
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
