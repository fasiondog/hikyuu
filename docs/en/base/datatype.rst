.. py:currentmodule:: hikyuu
.. highlight:: python

Basic Data Types
================

Date and Time
-------------

The date-time and its operations mainly involve :py:class:`Datetime` and :py:class:`TimeDelta`.

For the two classes and their related operation rules, refer to the datetime and timedelta help in the python datetime module;
the two can be converted to and from datetime and timedelta, and can be operated on directly.

The operation rules of TimeDelta are basically the same as datetime.timedelta.

.. py:class:: Datetime

    The date-time class (accurate to the microsecond), built in the following ways:
    
    - From a string: Datetime("2010-1-1 10:00:00"), Datetime("2001-1-1"), Datetime("20010101"), Datetime("20010101T232359)
    - From a Python date: Datetime(date(2010,1,1))
    - From a Python datetime: Datetime(datetime(2010,1,1,10)
    - From an integer in the YYYYMMDDHHMMSS, YYYYMMDDHHMM or YYYYMMDD form: Datetime(201001011000), Datetime(20010101)
    - Datetime(year, month, day, hour=0, minute=0, second=0, millisecond=0, microsecond=0)
    
    To get the date list, see: :py:func:`getDateRange`
    
    To get the trading dates, see: :py:meth:`StockManager.getTradingCalendar` 

    .. py:attribute:: year Year; an exception is raised if it is Null
    .. py:attribute:: month Month; an exception is raised if it is Null
    .. py:attribute:: day Day; an exception is raised if it is Null
    .. py:attribute:: hour Hour; an exception is raised if it is Null
    .. py:attribute:: minute Minute; an exception is raised if it is Null
    .. py:attribute:: second Second; an exception is raised if it is Null
    .. py:attribute:: millisecond Millisecond; an exception is raised if it is Null
    .. py:attribute:: microsecond Microsecond; an exception is raised if it is Null
    .. py:attribute:: number The integer in the YYYYMMDDHHMM form, with the accuracy only to the minute
    .. py:attribute:: ym The integer in the YYYYMM form
    .. py:attribute:: ymd The integer in the YYYYMMDD form
    .. py:attribute:: ymdh The integer in the YYYYMMDDhh form
    .. py:attribute:: ymdhm The integer in the YYYYMMDDhhmm form
    .. py:attribute:: ymdhms The integer in the YYYYMMDDhhmmss form
    .. py:attribute:: ticks The microseconds elapsed since the minimum date
    .. py:attribute:: hex Compatible with the oracle datetime representation; returns a 64-bit integer in which the last 7 bytes represent the century, the century year, the month, the day, the hour, the minute and the second

    .. py:method:: date(self)
    
        Convert to a python date
        
    .. py:method:: datetime(self)
    
        Convert to a python datetime
    
    .. py:method:: is_null(self)
    
        Whether it is a Null value, i.e. whether it equals constant.null_datetime

    .. py:method:: __add__(self, td)

        Add the specified time duration; the duration object can be a TimeDelta or a datetime.timedelta type

        :param TimeDelta td: the time duration
        :rtype: Datetime

    .. py:method:: __sub__(self, td)

        Subtract the specified time duration; the duration object can be a TimeDelta or a datetime.timedelta type

        :param TimeDelta td: the specified time duration
        :rtype: Datetime

    .. py:method:: day_of_week(self)
    
        Return the day of the week; Sunday is 0 and Monday is 1
        
        :rtype: int
        
    .. py:method:: date_of_week(self, day)
    
        Return the date of the specified day of this week; Sunday is day 0 and Saturday is day 6
        
        :param int day: indicate the day of this week; if it is less than 0, it is considered day 0, and if it is greater than 6, it is considered day 6
        :rtype: Datetime
        
    .. py:method:: day_of_year(self)
    
        Return the day of the year; January 1st is the 1st day of the year
        
        :rtype: int

    .. py:method:: start_of_day(self)

        Return 00:00:00 of the current day

    .. py:method:: end_of_day(self)

        Return 23:59:59 of the current day
        
    .. py:method:: start_of_week(self)
    
        Return the start date of the week (Monday)
        
    .. py:method:: end_of_week(self)
    
        Return the end date of the week (Sunday)
        
    .. py:method:: start_of_month(self)
    
        Return the start date of the month
        
    .. py:method:: end_of_month(self)
    
        Return the last day of the month
        
    .. py:method:: start_of_quarter(self)
    
        Return the start date of the quarter
        
    .. py:method:: end_of_quarter(self)
    
        Return the end date of the quarter
        
    .. py:method:: start_of_halfyear(self)
    
        Return the start date of the half-year
        
    .. py:method:: end_of_halfyear(self)
    
        Return the end date of the half-year
        
    .. py:method:: start_of_year(self)
    
        Return the start date of the year
        
    .. py:method:: end_of_year(self)
    
        Return the end date of the year
        
    .. py:method:: next_day(self)
    
        Return the next natural day
        
    .. py:method:: next_week(self)
    
        Return the Monday date of the next week
        
    .. py:method:: next_month(self)
    
        Return the first day of the next month
        
    .. py:method:: next_quarter(self)
    
        Return the first day of the next quarter
        
    .. py:method:: next_halfyear(self)
    
        Return the first day of the next half-year
        
    .. py:method:: next_year(self)
    
        Return the first day of the next year
        
    .. py:method:: pre_day(self)

        Return the previous natural day
        
    .. py:method:: pre_week(self)
    
        Return the Monday date of the previous week
        
    .. py:method:: pre_month(self)
    
        Return the first day of the previous month
        
    .. py:method:: pre_quarter(self)
    
        Return the first day of the previous quarter
        
    .. py:method:: pre_halfyear(self)
    
        Return the first day of the previous half-year

    .. py:method:: pre_year(self)
    
        Return the first day of the previous year
        
    .. py:staticmethod:: max()
    
        Get the maximum supported date-time
        
    .. py:staticmethod:: min()
    
        Get the minimum supported date-time
        
    .. py:staticmethod:: now()
    
        Get the current date-time
        
    .. py:staticmethod:: today()
    
        Get the current date


.. py:class:: TimeDelta

    The time duration, used for the time calculation. It can be built in the following ways:

    - Build from a datetime.timedelta. TimeDelta(timedelta instance)
    - TimeDelta(days=0, hours=0, minutes=0, seconds=0, milliseconds=0, microseconds=0)

        - -99999999 <= days <= 99999999
        - -100000 <= hours <= 100000
        - -100000 <= minutes <= 100000
        - -8639900 <= seconds <= 8639900
        - -86399000000 <= milliseconds <= 86399000000
        - -86399000000 <= microseconds <= 86399000000

    The parameter limits above are mainly to prevent a possible overflow when summing the total microseconds. When only one parameter is used and the limits above are not desired, the shortcut functions can be used:
    :py:func:`Days`, :py:func:`Hours`, :py:func:`Minutes`, :py:func:`Seconds`, :py:func:`Milliseconds`, 
    :py:func:`Microseconds`

    .. py:attribute:: days The number of days [-99999999, 99999999]
    .. py:attribute:: hours The number of hours [0, 23]
    .. py:attribute:: minutes The number of minutes [0, 59]
    .. py:attribute:: seconds The number of seconds [0, 59]
    .. py:attribute:: milliseconds The number of milliseconds [0, 999]
    .. py:attribute:: microseconds The number of microseconds [0, 999]
    .. py:attribute:: ticks The same as the total microseconds

    .. py:method:: isNegative(self)

        Whether it is a negative duration

        :rtype: bool

    .. py:method:: total_days(self)

        Get the total number of days with decimals

        :rtype: float

    .. py:method:: total_hours(self)

        Get the total number of hours with decimals

        :rtype: float

    .. py:method:: total_minutes(self)

        Get the total number of minutes with decimals

        :rtype: float

    .. py:method:: total_seconds(self)

        Get the total number of seconds with decimals

        :rtype: float

    .. py:method:: total_milliseconds(self)

        Get the total number of milliseconds with decimals

        :rtype: float

    .. py:staticmethod:: max()

        The maximum supported duration

        :return: TimeDelta(99999999, 23, 59, 59, 999, 999)

    .. py:staticmethod:: min()

        The minimum supported duration

        :return: TimeDelta(-99999999, 0, 0, 0, 0, 0)

    .. py:staticmethod:: resolution()

        The minimum supported precision
        
        :return: TimeDelta(0, 0, 0, 0, 0, 1)

    .. py:staticmethod:: max_ticks()

        The maximum supported ticks (i.e. the number of microseconds)

        :rtype: int

    .. py:staticmethod:: min_ticks()

        The minimum supported ticks (i.e. the number of microseconds)

        :rtype: int

    .. py:staticmethod:: from_ticks(ticks)

        Create with the ticks (i.e. the number of microseconds) value

        :param int ticks: the number of microseconds
        :rtype: TimeDelta


.. py:function:: Days(days)

    Create a TimeDelta with the number of days

    :param int days: the number of days
    :rtype: TimeDelta


.. py:function:: Hours(hours)

    Create a TimeDelta with the number of hours

    :param int hours: the number of hours
    :rtype: TimeDelta


.. py:function:: Minutes(mins)

    Create a TimeDelta with the number of minutes

    :param int mins: the number of minutes
    :rtype: TimeDelta


.. py:function:: Seconds(secs)

    Create a TimeDelta with the number of seconds

    :param int secs: the number of seconds
    :rtype: TimeDelta


.. py:function:: Milliseconds(milliseconds)

    Create a TimeDelta with the number of milliseconds

    :param int milliseconds: the number of milliseconds
    :rtype: TimeDelta


.. py:function:: Microseconds(microsecs)

    Create a TimeDelta with the number of microseconds

    :param int microsecs: the number of microseconds
    :rtype: TimeDelta



K-line Data
-----------       
    
.. py:class:: KRecord

    The K-line record, composing the K-line data; the attributes are readable and writable.
    
    .. py:attribute:: datetime : The date-time
    .. py:attribute:: open     : The open price
    .. py:attribute:: high     : The high price
    .. py:attribute:: low      : The low price
    .. py:attribute:: close    : The close price
    .. py:attribute:: amount   : The amount
    .. py:attribute:: volume   : The volume

    .. py:method:: is_valid(self)

        Judge whether the K-line record is valid

    
.. py:class:: KData

    The K-line data obtained through Stock.get_kdata; it is an array composed of KRecords and can be traversed like a list
    
    .. py:attribute:: start_pos
    
        Get the corresponding start position in the original K-line records; if the KData is empty, return 0
        
    .. py:attribute:: last_pos
    
        Get the position of the last record in the original K-line records; if it is empty, return 0, otherwise it equals endPos - 1
        
    .. py:attribute:: end_pos
    
        Get the position of the next record after the range in the original K-line records; if it is empty, return 0, otherwise it equals lastPos + 1
        
    .. py:method:: get_datetime_list()
    
        Return the trading date list

        :rtype: DatetimeList
        
    .. py:method:: get(pos)

        Get the K-line record at the specified index position
        
        :param int pos: the position index
        :rtype: KRecord        
    
    .. py:method:: get_by_datetime(datetime)    

        Get the K-line record at the specified time.

        :param Datetime datetime: the specified date
        :rtype: KRecord
    
    .. py:method:: get_pos(datetime)

        Get the index position corresponding to the specified time
        
        :param Datetime datetime: the specified time
        :return: the corresponding index position; if it is out of the data range, return None    

    .. py:method:: get_pos_in_stock(datetime)

        Get the index position in the original K-line corresponding to the specified time
        
        :param Datetime datetime: the specified time
        :return: the corresponding index position; if it is out of the data range, return None    

    .. py:method:: empty()
    
        Judge whether it is empty
    
        :rtype: bool
    
    .. py:method:: get_query()
    
        Get the associated query condition
    
        :rtype: Query
    
    .. py:method:: get_stock()
    
        Get the associated Stock
    
        :rtype: Stock

    .. py:method:: get_kdata()

        Get a new KData that keeps the data type and the adjustment type unchanged through the current KData (note that it is not a subset of the original KData)

        :param Datetime start: the new start date
        :param Datetime end: the new end date
        :rtype: KData
    
    .. py:method:: tocsv(filename)
    
        Save the data to a CSV file
    
        :param str filename: the specified file name to save

    .. py:method:: to_np()
    
        Convert to a numpy structured array
    
        :rtype: numpy.array
        
    .. py:method:: to_df(with_stock=False)
    
        Convert to a pandas DataFrame
        
        :param bool with_stock: include the code and the name of the Stock
        :rtype: pandas.DataFrame


Time-line Data
--------------

.. py:class:: TimeLineRecord

    The time-line record; the attributes are readable and writable.
    
    .. py:attribute:: date : The date-time
    .. py:attribute:: price : The price
    .. py:attribute:: vol : The volume


.. py:class:: TimeLineList

    The time-line data obtained through Stock.get_timeline_list; it is an array composed of TimeLineRecords and can be traversed like a list
    
    .. py:method:: to_numpy()
    
        Convert to a numpy structured array
    
        :rtype: numpy.array
        
    .. py:method:: to_pandas()
    
        Convert to a pandas DataFrame
        
        :rtype: pandas.DataFrame

    .. py:method:: to_pyarrow()

        Convert to pyarrow.Table
        

Tick History Data
-----------------

.. py:class:: TransRecord

    The historical tick data record
    
    .. py:attribute:: date : The time
    .. py:attribute:: price : The price
    .. py:attribute:: vol : The volume
    .. py:attribute:: direct : The nature of the buy/sell order: 1--sell 0--buy 2--call auction, others unknown
    
.. py:class:: TransList

    The time-line data list
    
    .. py:method:: to_numpy()
    
        Convert to a numpy structured array
    
        :rtype: numpy.array
        
    .. py:method:: to_pandas()
    
        Convert to a pandas DataFrame
        
        :rtype: pandas.DataFrame

    .. py:method:: to_pyarrow()

        Convert to pyarrow.Table
