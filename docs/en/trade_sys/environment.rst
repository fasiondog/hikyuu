.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Market Environment Strategy|EV
=================================

Built-in Market Environment Strategies
-----------------------------------------

.. py:function:: EV_TwoLine(fast, slow[, market = 'SH'])

    The fast/slow line strategy; when the fast line of the market index is above the slow line, the market is valid, otherwise invalid.

    :param Indicator fast: the fast line indicator
    :param Indicator slow: the slow line indicator
    :param string market: the market name

.. py:function:: EV_Bool(ind[, market = 'SH'])

    The boolean signal generator market environment

    :param Indicator ind: a bool-type indicator; a position greater than 0 in the indicator means the market is valid, otherwise invalid
    :param str market: the specified market, used to get the corresponding trading calendar


Custom Market Environment Strategy
-------------------------------------

The custom market environment strategy interface:

* :py:meth:`EnvironmentBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`EnvironmentBase._clone` - [Required] The clone interface
* :py:meth:`EnvironmentBase._reset` - [Optional] Reload the private variables

Market Environment Strategy Base Class
-----------------------------------------

.. py:class:: EnvironmentBase

    The market environment strategy base class
    
    .. py:attribute:: name Name
    .. py:attribute:: query Set or get the query condition
    
    .. py:method:: __init__(self[, name='EnvironmentBase'])
    
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
    
        [Overload interface] The subclass reset interface, used to reset the internal private variables
    
    .. py:method:: _clone(self)
    
        [Overload interface] The subclass clone interface
