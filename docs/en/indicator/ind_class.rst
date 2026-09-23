.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

Indicator
=========


.. py:class:: Indicator(name)

    Definition of the Indicator base class.

    .. py:attribute:: name Name

    .. py:attribute:: long_name Name

    .. py:attribute:: discard Number of points to discard (warm-up length)

    .. py:method:: have_param(self, name)

        Check whether the specified parameter exists

        :param str name: parameter name

    .. py:method:: get_param(self, name)

        Get the value of the specified parameter
    
        :param str name: parameter name
        :return: parameter value
        :raises out_of_range: raised if no such parameter exists
        
    .. py:method:: set_param(self, name, value)
    
        Set the value of a parameter
        
        :param str name: parameter name
        :param value: parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported

    .. py:method:: support_ind_param(self)

        Check whether dynamic indicator parameters are supported

    .. py:method:: have_ind_param(self, name)

        Check whether the specified dynamic indicator parameter exists

        :param str name: parameter name

    .. py:method:: get_ind_param(self, name)

        Get the specified dynamic indicator parameter

        :param str name: parameter name
        :rtype: IndParam

    .. py:method:: set_ind_param(self, name, val)

        Set a dynamic indicator parameter

        :param str name: parameter name
        :param Indicator|IndParam: parameter value (an Indicator or IndParam instance)    

    .. py:method:: clone(self)
    
        Return a clone of this indicator

    .. py:method:: empty(self)

        Check whether the indicator is empty

        :rtype: bool

    .. py:method:: formula(self)

        Return the formula expression of the indicator

        :rtype: str

    .. py:method:: get_result_num(self)

        Get the number of result sets

        :rtype: int

    .. py:method:: get(self, pos[, result_index=0])

        Get the value at the specified position

        :param int pos: position index
        :param int result_index: index of the result set

    .. py:method:: get_pos(self, date)

        Get the index position corresponding to the specified date

        :param Datetime date: specified date
        :rtype: int

    .. py:method:: get_datetime(self, pos)

        Get the date at the specified position

        :param int pos: position index

    .. py:method:: get_by_datetime(self, datetime[, result_index=0])

        Get the value at the specified date. If there is no result for that date, return constant.null_price

        :param Datetime datetime: specified date
        :param int result_index: index of the result set
        :rtype: float

    .. py:method:: get_result(self, result_index)

        Get the specified result set

        :param int result_index: index of the result set
        :rtype: Indicator

    .. py:method:: get_result_as_price_list(self, result_index)

        Get the specified result set as a list

        :param int result_index: index of the result set
        :rtype: list

    .. py:method:: get_datetime_list(self)

        Return the list of corresponding dates

        :rtype: DatetimeList

    .. py:method:: get_context(self)

        Get the bound context

        :rtype: KData

    .. py:method:: set_context(self, kdata)

        Set the context

        :param KData kdata: context K-line (candlestick) to bind

        set_context(self, stock, query)

        Set the context

        :param Stock stock: specified Stock
        :param Query query: query criteria

    .. py:method:: extend(self)

        When a context is bound, automatically extend the context to the latest available data and perform an incremental calculation.

        During the incremental calculation, the start of the current query is kept fixed and the query is extended backward to the new end point. Some recursive indicators are sensitive to their initial values (such as AMA), so the result may differ slightly.

        Main use case: quickly pulling the latest data for calculation in live trading.

    .. py:method:: to_array(self, result_index=0)

        Convert the specified result set to a numpy.array

    .. py:method:: to_numpy(self)

        Convert to a numpy array; for a time series, the datetime column is included

        :rtype: numpy.ndarray

    .. py:method:: value_to_numpy(self)

        Convert to a numpy array containing only the values

    .. py:method:: to_pandas(self)

        Convert to a pandas DataFrame; for a time series, the datetime column is included

    .. py:method:: value_to_pandas(self)

        Convert to a pandas DataFrame containing only the values

    .. py:method:: to_pyarrow(self)

        Convert to a pyarrow Table; for a time series, the datetime column is included

        :rtype: pyarrow.Table

    .. py:method:: value_to_pyarrow(self)

        Convert to a pyarrow Table containing only the values

        :rtype: pyarrow.Table
