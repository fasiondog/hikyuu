.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

Indicator
=========


.. py:class:: Indicator(name)

    The Indicator definition

    .. py:attribute:: name Name

    .. py:attribute:: long_name Name

    .. py:attribute:: discard The number of the points to discard

    .. py:method:: have_param(self, name)

        Whether the specified parameter exists

        :param str name: the parameter name

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

    .. py:method:: support_ind_param(self)

        Whether the dynamic indicator parameters are supported

    .. py:method:: have_ind_param(self, name)

        Whether the specified parameter exists

        :param str name: the parameter name

    .. py:method:: get_ind_param(self, name)

        Get the specified dynamic indicator parameter

        :param str name: the parameter name
        :rtype: IndParam

    .. py:method:: set_ind_param(self, name, val)

        Set the dynamic indicator parameter

        :param str name: the parameter name
        :param Indicator|IndParam: the parameter value (can be an Indicator or an IndParam instance)    

    .. py:method:: clone(self)
    
        The clone operation

    .. py:method:: empty(self)

        Whether it is empty

        :rtype: bool

    .. py:method:: formula(self)

        Print the indicator formula

        :rtype: str

    .. py:method:: get_result_num(self)

        Get the number of the result sets

        :rtype: int

    .. py:method:: get(self, pos[, result_index=0])

        Get the value at the specified position

        :param int pos: the specified position index
        :param int result_index: the specified result set

    .. py:method:: get_pos(self, date)

        Get the index position corresponding to the specified date

        :param Datetime date: the specified date
        :rtype: int

    .. py:method:: get_datetime(self, pos)

        Get the date at the specified position

        :param int pos: the specified position index

    .. py:method:: get_by_datetime(self, datetime[, result_index=0])

        Get the value of the specified date. If there is no result for the corresponding date, return constant.null_price

        :param Datetime datetime: the specified date
        :param int result_index: the specified result set
        :rtype: float

    .. py:method:: get_result(self, result_index)

        Get the specified result set

        :param int result_index: the specified result set
        :rtype: Indicator

    .. py:method:: get_result_as_price_list(self, result_index)

        Get the specified result set

        :param int result_index: the specified result set
        :rtype: list

    .. py:method:: get_datetime_list(self)

        Return the corresponding date list

        :rtype: DatetimeList

    .. py:method:: get_context(self)

        Get the context

        :rtype: KData

    .. py:method:: set_context(self, kdata)

        Set the context

        :param KData kdata: the associated context K-line

        set_context(self, stock, query)

        Set the context

        :param Stock stock: the specified Stock
        :param Query query: the specified query condition

    .. py:method:: extend(self)

        When there is a context, automatically extend the context to the current latest data and perform the incremental calculation.

        In the incremental calculation, it is equivalent to keeping the start of the current query unchanged and extending backward to the new end point. This feature is sensitive to the initial values for some recursive indicators, such as AMA, so there may be some differences.

        Main usage: quickly get the latest data to calculate in live trading

    .. py:method:: to_array(self, result_index=0)

        Convert the specified result set to numpy.array

    .. py:method:: to_numpy(self)

        Convert to a numpy array; if it is a time series, the datetime column will be included

        :rtype: numpy.ndarray

    .. py:method:: value_to_numpy(self)

        Convert to a numpy array, containing only the value

    .. py:method:: to_pandas(self)

        Convert to a pandas DataFrame; if it is a time series, the datetime column will be included

    .. py:method:: value_to_pandas(self)

        Convert to a pandas DataFrame, containing only the value

    .. py:method:: to_pyarrow(self)

        Convert to a pyarrow Table; if it is a time series, the datetime column will be included

        :rtype: pyarrow.Table

    .. py:method:: value_to_pyarrow(self)

        Convert to a pyarrow Table, containing only the value

        :rtype: pyarrow.Table
