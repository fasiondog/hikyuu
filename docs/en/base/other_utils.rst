.. py:currentmodule:: hikyuu
.. highlight:: python

Miscellaneous and Auxiliary
===========================

Functions
---------

.. py:function:: select(cond, start=Datetime(201801010000), end=Datetime.now(), print_out=True)

    Example::
    
        # Select the limit-up stocks
        C = CLOSE()
        x = select(C / REF(C, 1) - 1 >= 0.0995)

    :param Indicator cond: the condition indicator
    :param Datetime start: the start date
    :param Datetime end: the end date
    :param bool print_out: print the selected stocks
    :rtype: the list of the selected stocks


.. py:function:: select2(inds, start=Datetime(201801010000), end=Datetime.now(), stks=None)

    Export all the specified indicator values of the specified securities at the last moment.

    E.g.:
        select2([CLOSE(), VOLUME()], stks=blocka)
    Return a DataFrame, the column names are the indicator names, the rows are the code and the name of the securities:
        证券代码  证券名称  CLOSE  VOLUME
        SH600000 浦发银行  14.09   1000
        SH600001 中国平安  13.09   2000
        SZ000001 平安银行  13.09   3000
        ...

    :param Indicator inds: the indicator list
    :param Datetime start: the start date
    :param Datetime end: the end date (excluding this date)
    :param list stks: the specified securities list
    :rtype: pandas.DataFrame


.. py:function:: get_log_level()

    Get the current log printing level
    
    :rtype: LOG_LEVEL
    
.. py:function:: set_log_level(level)

    Set the log printing level
    
    :param LOG_LEVEL level: the specified log printing level

.. py:function:: hku_save(var, filename)

    Serialization: save a variable of a hikyuu built-in type (such as Stock, TradeManager, etc.) to the specified file, in XML format.
    
    :param var: a variable of a hikyuu built-in type
    :param str filename: the specified file name

.. py:function:: hku_load(filename)

    Read the variable saved by hku_save into var.
    
    :param str filename: the serialization file to be loaded.
    :return: the variable previously saved by serialization
    
.. py:function:: roundUp(number[, ndigits=0])

    Round up, e.g. 10.1 becomes 11 after rounding
    
    :param float number: the data to process
    :param int ndigits: the number of the decimal digits to keep
    :return: the processed data


.. py:function:: roundDown(number[, ndigits=0])

    Round down, e.g. 10.1 becomes 10 after rounding
    
    :param float number: the data to process
    :param int ndigits: the number of the decimal digits to keep
    :return: the processed data
    
    
.. py:function:: get_date_range(start, end)

    Get the list of the natural calendar dates in the specified [start, end) date-time range, supported only up to the day
    
    :param Datetime start: the start date
    :param Datetime end: the end date
    :rtype: DatetimeList

    
.. py:function:: toPriceList(arg)

    Convert a Python iterable object such as a list or a tuple to a PriceList (this function is actually deprecated and no longer needed)
    
    :param arg: the Python sequence to convert
    :rtype: list
    

.. py:function:: set_global_context(stk, query)

    Set the global context

    :param Stock stk: the specified global Stock
    :param Query query: the specified query condition
    
    
.. py:function:: get_global_context()

    Get the current global default context
    
    :rtype: KData

    
Classes
-------

.. py:class:: Parameter

    The parameter class
    
    .. py:method:: get(self, name)
        
        Get the specified parameter
        
        :param str name: the parameter name
        :return: the parameter value
        
    .. py:method:: set(self, name, value)
    
        Set the parameter
        
        :param str name: the parameter name
        :param value: the parameter value (only the int | float | str | bool types are supported)
    
    
.. py:class:: PriceList

    The price sequence, in which the price is represented with a double, corresponding to std::vector<double> in C++.

    .. py:method:: to_np(self)

        Takes effect only when the numpy module is installed; converts to numpy.array

    .. py:method:: to_df(self)

        Takes effect only when the pandas module is installed; converts to pandas.DataFrame
        
        
.. py:class:: DatetimeList

    The date sequence, corresponding to std::vector<Datetime> in C++
    
    .. py:method:: append(self, datetime)
    
        Add an element to the end of the list
        
        :param Datetime datetime: the element to add
    
    .. py:method:: to_np(self)

        Takes effect only when the numpy module is installed; converts to numpy.array

    .. py:method:: to_df(self)

        Takes effect only when the pandas module is installed; converts to pandas.DataFrame


.. py:class:: StringList

    The string list, corresponding to std::vector<String> in C++
    
    
.. py:class:: KRecordList

    A wrapper of the C++ std::vector<KRecord>
    
    .. py:method:: append(self, krecord)
    
        Add an element to the end of the list
        
        :param KRecord krecord: the element to add
    
    .. py:method:: to_np(self)

        Takes effect only when the numpy module is installed; converts to numpy.array

    .. py:method:: to_df(self)

        Takes effect only when the pandas module is installed; converts to pandas.DataFrame
    

.. py:class:: BlockList

    A wrapper of the C++ std::vector<Block>
    
    
.. py:class:: OstreamRedirect

    Redirect the C++ std::cout and std::cerr to python. In a non-command-line mode, some Apps cannot display the C++ iostream output, such as the Jupyter notebook. By default, the construction only specifies whether std::cout or std::cerr needs to be redirected; the redirection must be enabled with the open method or the with syntax.
    
    An example of using with:
    
    .. code-block:: python
    
        with OstreamRedirect():
            your_function() # the wrapped C++ function, which outputs with std::iostream
    

    .. py:method:: init(self[, stdout=True, stderr=True])
        
        :param bool stdout: whether to redirect the C++ std::cout
        :param bool stderr: whether to redirect the C++ std::cerr

    .. py:method:: open(self)
    
        Enable the redirection
        
    .. py:method:: close(self)
    
        Disable the redirection
    
    
Enumerations
------------    

.. py:class:: LOG_LEVEL

    - DEBUG
    - TRACE
    - INFO
    - WARN
    - ERROR
    - FATAL
    - NO_PRINT
