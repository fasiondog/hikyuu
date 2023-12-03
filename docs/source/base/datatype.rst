.. py:currentmodule:: hikyuu
.. highlight:: python

基础数据类型
============

日期时间
-----------

日期时间及其运算主要涉及 :py:class:`Datetime`，:py:class:`TimeDelta`。

两者及其相关运算规则可参考 python datetime 模块中的 datetime 和 timedelta 帮助，
并且两者可和 datetime、timedelta 相互转换，并直接进行运算操作。

TimeDelta 的运算规则基本与 datetime.timedelta 相同。

.. py:class:: Datetime

    日期时间类（精确到微秒），通过以下方式构建：
    
    - 通过字符串：Datetime("2010-1-1 10:00:00")、Datetime("2001-1-1")、Datetime("20010101")、Datetime("20010101T232359)
    - 通过 Python 的date：Datetime(date(2010,1,1))
    - 通过 Python 的datetime：Datetime(datetime(2010,1,1,10)
    - 通过 YYYYMMDDHHMMSS 或 YYYYMMDDHHMM 或 YYYYMMDD 形式的整数：Datetime(201001011000)、Datetime(20010101)
    - Datetime(year, month, day, hour=0, minute=0, second=0, millisecond=0, microsecond=0)
    
    获取日期列表参见： :py:func:`getDateRange`
    
    获取交易日日期参见： :py:meth:`StockManager.getTradingCalendar` 

    .. py:attribute:: year 年，如果是 Null 将抛出异常
    .. py:attribute:: month 月，如果是 Null 将抛出异常
    .. py:attribute:: day 日，如果是 Null 将抛出异常
    .. py:attribute:: hour 时，如果是 Null 将抛出异常
    .. py:attribute:: minute 分，如果是 Null 将抛出异常
    .. py:attribute:: second 秒，如果是 Null 将抛出异常
    .. py:attribute:: millisecond 毫秒，如果是 Null 将抛出异常
    .. py:attribute:: microsecond 微秒，如果是 Null 将抛出异常
    .. py:attribute:: number YYYYMMDDHHMM 形式的整数，精度只到分钟
    .. py:attribute:: ym YYYYMM 形式的整数
    .. py:attribute:: ymd YYYYMMDD 形式的整数
    .. py:attribute:: ymdh YYYYMMDDhh 形式的整数
    .. py:attribute:: ymdhm YYYYMMDDhhmm 形式的整数
    .. py:attribute:: ymdhms YYYYMMDDhhmmss 形式的整数
    .. py:attribute:: hex 兼容oracle datetime 表示, 返回用后7个字节表示世纪、世纪年、月、日、时、分、秒的64位整数

    .. py:method:: date(self)
    
        转化生成 python 的 date
        
    .. py:method:: datetime(self)
    
        转化生成 python 的datetime
    
    .. py:method:: is_null(self)
    
        是否是Null值, 即是否等于 constant.null_datetime

    .. py:method:: __add__(self, td)

        加上指定时长，时长对象可为 TimeDelta 或 datetime.timedelta 类型

        :param TimeDelta td: 时长
        :rtype: Datetime

    .. py:method:: __sub__(self, td)

        减去指定的时长, 时长对象可为 TimeDelta 或 datetime.timedelta 类型

        :param TimeDelta td: 指定时长
        :rtype: Datetime

    .. py:method:: day_of_week(self)
    
        返回是一周中的第几天，周日为0，周一为1
        
        :rtype: int
        
    .. py:method:: date_of_week(self, day)
    
        返回指定的本周中第几天的日期，周日为0天，周六为第6天
        
        :param int day: 指明本周的第几天，如小于则认为为第0天，如大于6则认为为第6天
        :rtype: Datetime
        
    .. py:method:: day_of_year(self)
    
        返回一年中的第几天，1月1日为一年中的第1天
        
        :rtype: int

    .. py:method:: start_of_day(self)

        返回当天 0点0分0秒

    .. py:method:: end_of_day(self)

        返回当日 23点59分59秒
        
    .. py:method:: start_of_week(self)
    
        返回周起始日期（周一）
        
    .. py:method:: end_of_week(self)
    
        返回周结束日期（周日）
        
    .. py:method:: start_of_month(self)
    
        返回月度起始日期
        
    .. py:method:: end_of_month(self)
    
        返回月末最后一天日期
        
    .. py:method:: start_of_quarter(self)
    
        返回季度起始日期
        
    .. py:method:: end_of_quarter(self)
    
        返回季度结束日期
        
    .. py:method:: start_of_halfyear(self)
    
        返回半年度起始日期
        
    .. py:method:: end_of_halfyear(self)
    
        返回半年度结束日期
        
    .. py:method:: start_of_year(self)
    
        返回年度起始日期
        
    .. py:method:: end_of_year(self)
    
        返回年度结束日期
        
    .. py:method:: next_day(self)
    
        返回下一自然日
        
    .. py:method:: next_week(self)
    
        返回下周周一日期
        
    .. py:method:: next_month(self)
    
        返回下月首日日期
        
    .. py:method:: next_quarter(self)
    
        返回下一季度首日日期
        
    .. py:method:: next_halfyear(self)
    
        返回下一半年度首日日期
        
    .. py:method:: next_year(self)
    
        返回下一年度首日日期
        
    .. py:method:: pre_day(self)

        返回前一自然日日期
        
    .. py:method:: pre_week(self)
    
        返回上周周一日期
        
    .. py:method:: pre_month(self)
    
        返回上月首日日期
        
    .. py:method:: pre_quarter(self)
    
        返回上一季度首日日期
        
    .. py:method:: pre_halfyear(self)
    
        返回上一半年度首日日期

    .. py:method:: pre_year(self)
    
        返回上一年度首日日期
        
    .. py:staticmethod:: max()
    
        获取支持的最大日期时间
        
    .. py:staticmethod:: min()
    
        获取支持的最小日期时间
        
    .. py:staticmethod:: now()
    
        获取当前的日期时间
        
    .. py:staticmethod:: today()
    
        获取当前的日期


.. py:class:: TimeDelta

    时间时长，用于时间计算。可通过以下方式构建：

    - 通过 datetime.timedelta 构建。TimdeDelta(timedelta实例)
    - TimeDelta(days=0, hours=0, minutes=0, seconds=0, milliseconds=0, microseconds=0)

        - -99999999 <= days <= 99999999
        - -100000 <= hours <= 100000
        - -100000 <= minutes <= 100000
        - -8639900 <= seconds <= 8639900
        - -86399000000 <= milliseconds <= 86399000000
        - -86399000000 <= microseconds <= 86399000000

    以上参数限制，主要为防止求总微秒数时可能出现溢出的情况。如只使用一个参数不希望存在上述限制时，可使用快捷函数：
    :py:func:`Days`, :py:func:`Hours`, :py:func:`Minutes`, :py:func:`Seconds`, :py:func:`Milliseconds`, 
    :py:func:`Microseconds`

    .. py:attribute:: days 天数 [-99999999, 99999999]
    .. py:attribute:: hours 小时数 [0, 23]
    .. py:attribute:: minutes 分钟数 [0, 59]
    .. py:attribute:: seconds 秒数 [0, 59]
    .. py:attribute:: milliseconds 毫秒数 [0, 999]
    .. py:attribute:: microseconds 微秒数 [0, 999]
    .. py:attribute:: ticks 同总微秒数

    .. py:method:: isNegative(self)

        是否为负时长

        :rtype: bool

    .. py:method:: total_days(self)

        获取带小数的总天数

        :rtype: float

    .. py:method:: total_hours(self)

        获取带小数的总小时数

        :rtype: float

    .. py:method:: total_minutes(self)

        获取带小数的总分钟数

        :rtype: float

    .. py:method:: total_seconds(self)

        获取带小数的总秒数

        :rtype: float

    .. py:method:: total_milliseconds(self)

        获取带小数的总毫秒数

        :rtype: float

    .. py:staticmethod:: max()

        支持的最大时长

        :return: TimeDelta(99999999, 23, 59, 59, 999, 999)

    .. py:staticmethod:: min()

        支持的最小时长

        :return: TimeDelta(-99999999, 0, 0, 0, 0, 0)

    .. py:staticmethod:: resolution()

        支持的最小精度
        
        :return: TimeDelta(0, 0, 0, 0, 0, 1)

    .. py:staticmethod:: max_ticks()

        支持的最大 ticks （即微秒数）

        :rtype: int

    .. py:staticmethod:: min_ticks()

        支持的最小 ticks （即微秒数）

        :rtype: int

    .. py:staticmethod:: from_ticks(ticks)

        使用 ticks（即微秒数） 值创建

        :param int ticks: 微秒数
        :rtype: TimeDelta


.. py:function:: Days(days)

    以天数创建 TimeDelta

    :param int days: 天数
    :rtype: TimeDelta


.. py:function:: Hours(hours)

    以小时数创建 TimeDelta

    :param int hours: 小时数
    :rtype: TimeDelta


.. py:function:: Minutes(minutes)

    以分钟数创建 TimeDelta

    :param int minutes: 分钟数
    :rtype: TimeDelta


.. py:function:: Seconds(seconds)

    以秒数创建 TimeDelta

    :param int seconds: 秒数
    :rtype: TimeDelta


.. py:function:: Milliseconds(milliseconds)

    以毫秒数创建 TimeDelta

    :param int milliseconds: 毫秒数
    :rtype: TimeDelta


.. py:function:: Microseconds(microseconds)

    以微秒数创建 TimeDelta

    :param int microseconds: 微秒数
    :rtype: TimeDelta



K线数据
----------       
       
.. py:class:: KRecord

    K线记录，组成K线数据，属性可读写。
    
    .. py:attribute:: datetime : 日期时间
    .. py:attribute:: open     : 开盘价
    .. py:attribute:: high     : 最高价
    .. py:attribute:: low      : 最低价
    .. py:attribute:: close    : 收盘价
    .. py:attribute:: amount   : 成交金额
    .. py:attribute:: volume   : 成交量

    
.. py:class:: KData

    通过 Stock.get_kdata 获取的K线数据，由 KRecord 组成的数组，可象 list 一样进行遍历
    
    .. py:attribute:: start_pos
    
        获取在原始K线记录中对应的起始位置，如果KData为空返回0
        
    .. py:attribute:: last_pos
    
        获取在原始K线记录中对应的最后一条记录的位置，如果为空返回0,其他等于endPos - 1
        
    .. py:attribute:: end_pos
    
        获取在原始K线记录中对应范围的下一条记录的位置，如果为空返回0,其他等于lastPos + 1
        
    .. py:method:: get_datetime_list()
    
        返回交易日期列表

        :rtype: DatetimeList
        
    .. py:method:: get(pos)

        获取指定索引位置的K线记录
        
        :param int pos: 位置索引
        :rtype: KRecord        
    
    .. py:method:: get_by_datetime(datetime)    

        获取指定时间的K线记录。
    
        :param Datetime datetime: 指定的日期
        :rtype: KRecord
    
    .. py:method:: get_pos(datetime)

        获取指定时间对应的索引位置
        
        :param Datetime datetime: 指定的时间
        :return: 对应的索引位置，如果不在数据范围内，则返回 None    

    .. py:method:: get_pos_in_stock(datetime)

        获取指定时间对应原始 K 线中的索引位置
        
        :param Datetime datetime: 指定的时间
        :return: 对应的索引位置，如果不在数据范围内，则返回 None    

    .. py:method:: empty()
    
        判断是否为空
    
        :rtype: bool
    
    .. py:method:: get_query()
    
        获取关联的查询条件
    
        :rtype: Query
    
    .. py:method:: get_stock()
    
        获取关联的Stock
    
        :rtype: Stock
    
    .. py:method:: tocsv(filename)
    
        将数据保存至CSV文件
    
        :param str filename: 指定保存的文件名称

    .. py:method:: to_np()
    
        转化为numpy结构数组
    
        :rtype: numpy.array
        
    .. py:method:: to_df()
    
        转化为pandas的DataFrame
        
        :rtype: pandas.DataFrame


分时线数据
--------------

.. py:class:: TimeLineRecord

    分时线记录，属性可读写。
    
    .. py:attribute:: date : 日期时间
    .. py:attribute:: price : 价格
    .. py:attribute:: vol : 成交量


.. py:class:: TimeLineList

    通过 Stock.getTimeLineList 获取的分时线数据，由 TimeLineRecord 组成的数组，可象 list 一样进行遍历
    
    .. py:method:: to_np()
    
        转化为numpy结构数组
    
        :rtype: numpy.array
        
    .. py:method:: to_df()
    
        转化为pandas的DataFrame
        
        :rtype: pandas.DataFrame
        

分笔历史数据
----------------

.. py:class:: TransRecord

    历史分笔数据记录
    
    .. py:attribute:: date : 时间
    .. py:attribute:: price : 价格
    .. py:attribute:: vol : 成交量
    .. py:attribute:: direct : 买卖盘性质
    
.. py:class:: TransList

    分时线数据列表
    
    .. py:method:: to_np()
    
        转化为numpy结构数组
    
        :rtype: numpy.array
        
    .. py:method:: to_df()
    
        转化为pandas的DataFrame
        
        :rtype: pandas.DataFrame
    
