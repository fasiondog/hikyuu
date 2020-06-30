#
# 对 C++ 引出类和函数进行扩展
#

from datetime import *
from hikyuu.cpp.core import *

# ------------------------------------------------------------------
# 常量定义，各种C++中Null值
# ------------------------------------------------------------------

constant = Constant()

# ------------------------------------------------------------------
# 增加Datetime、Stock的hash支持，以便可做为dict的key
# ------------------------------------------------------------------

Datetime.__hash__ = lambda self: self.number * 1000000 + self.millisecond * 1000 + self.microsecond
TimeDelta.__hash__ = lambda self: self.ticks
Stock.__hash__ = lambda self: self.id

# ------------------------------------------------------------------
# 增强 Datetime
# ------------------------------------------------------------------

__old_Datetime_init__ = Datetime.__init__
__old_Datetime_add__ = Datetime.__add__
__old_Datetime_sub__ = Datetime.__sub__


def __new_Datetime_init__(self, *args, **kwargs):
    """
    日期时间类（精确到秒），通过以下方式构建：

    - 通过字符串：Datetime("2010-1-1 10:00:00")
    - 通过 Python 的date：Datetime(date(2010,1,1))
    - 通过 Python 的datetime：Datetime(datetime(2010,1,1,10)
    - 通过 YYYYMMDDHHMM 形式的整数：Datetime(201001011000)
    - Datetime(year, month, day, hour=0, minute=0, second=0, millisecond=0, microsecond=0)

    获取日期列表参见： :py:func:`getDateRange`

    获取交易日日期参见： :py:meth:`StockManager.getTradingCalendar` 
    """
    if not args:
        __old_Datetime_init__(self, **kwargs)

    # datetime实例同时也是date的实例，判断必须放在date之前
    elif isinstance(args[0], datetime):
        d = args[0]
        milliseconds = d.microsecond // 1000
        microseconds = d.microsecond - milliseconds * 1000
        __old_Datetime_init__(
            self, d.year, d.month, d.day, d.hour, d.minute, d.second, milliseconds, microseconds
        )
    elif isinstance(args[0], date):
        d = args[0]
        __old_Datetime_init__(self, d.year, d.month, d.day, 0, 0, 0, 0)

    elif isinstance(args[0], str):
        __old_Datetime_init__(self, args[0])
    else:
        __old_Datetime_init__(self, *args)


def __new_Datetime_add__(self, td):
    """加上指定时长，时长对象可为 TimeDelta 或 datetime.timedelta 类型

    :param TimeDelta td: 时长
    :rtype: Datetime
    """
    if isinstance(td, TimeDelta):
        return __old_Datetime_add__(self, td)
    elif isinstance(td, timedelta):
        return __old_Datetime_add__(self, TimeDelta(td))
    else:
        raise TypeError("unsupported operand type(s) for +: 'TimeDelta' and '{}'".format(type(td)))


def __new_Datetime_sub__(self, td):
    """减去指定的时长, 时长对象可为 TimeDelta 或 datetime.timedelta 类型

    :param TimeDelta td: 指定时长
    :rtype: Datetime
    """
    if isinstance(td, TimeDelta):
        return __old_Datetime_sub__(self, td)
    elif isinstance(td, timedelta):
        return __old_Datetime_sub__(self, TimeDelta(td))
    else:
        raise TypeError("unsupported operand type(s) for +: 'TimeDelta' and '{}'".format(type(td)))


def Datetime_date(self):
    """转化生成 python 的 date"""
    return date(self.year, self.month, self.day)


def Datetime_datetime(self):
    """转化生成 python 的 datetime"""
    return datetime(
        self.year, self.month, self.day, self.hour, self.minute, self.second, self.microsecond
    )


Datetime.__init__ = __new_Datetime_init__
Datetime.__add__ = __new_Datetime_add__
Datetime.__radd__ = __new_Datetime_add__
Datetime.__sub__ = __new_Datetime_sub__
Datetime.date = Datetime_date
Datetime.datetime = Datetime_datetime

# ------------------------------------------------------------------
# 增强 TimeDelta
# ------------------------------------------------------------------

__old_TimeDelta_init__ = TimeDelta.__init__
__old_TimeDelta_add__ = TimeDelta.__add__
__old_TimeDelta_sub__ = TimeDelta.__sub__


def __new_TimeDelta_init__(self, *args, **kwargs):
    """
    可通过以下方式构建：

    - 通过 datetime.timedelta 构建。TimdeDelta(timedelta实例)
    - TimeDelta(days=0, hours=0, minutes=0, seconds=0, milliseconds=0, microseconds=0)

        - -99999999 <= days <= 99999999
        - -100000 <= hours <= 100000
        - -100000 <= minutes <= 100000
        - -8639900 <= seconds <= 8639900
        - -86399000000 <= milliseconds <= 86399000000
        - -86399000000 <= microseconds <= 86399000000
    """
    if not args:
        __old_TimeDelta_init__(self, **kwargs)
    elif isinstance(args[0], timedelta):
        days = args[0].days
        secs = args[0].seconds
        hours = secs // 3600
        mins = secs // 60 - hours * 60
        secs = secs - mins * 60 - hours * 3600
        microsecs = args[0].microseconds
        millisecs = microsecs // 1000
        microsecs = microsecs - millisecs * 1000
        __old_TimeDelta_init__(self, days, hours, mins, secs, millisecs, microsecs)
    else:
        __old_TimeDelta_init__(self, *args)


def __new_TimeDelta_add__(self, td):
    """可和 TimeDelta, datetime.timedelta, Datetime执行相加操作"""
    if isinstance(td, TimeDelta):
        return __old_TimeDelta_add__(self, td)
    elif isinstance(td, timedelta):
        return __old_TimeDelta_add__(self, TimeDelta(td))
    elif isinstance(td, Datetime):
        return td + self
    elif isinstance(td, datetime):
        return td + Datetime(datetime)
    else:
        raise TypeError("unsupported operand type(s) for +: 'TimeDelta' and '{}'".format(type(td)))


def __new_TimeDelta_sub__(self, td):
    """可减去TimeDelta, datetime.timedelta"""
    return __old_TimeDelta_sub__(self, td) if isinstance(td, TimeDelta) else __old_TimeDelta_sub__(
        self, TimeDelta(td)
    )


def TimeDelta_timedelta(self):
    """ 转化为 datetime.timedelta """
    return timedelta(
        days=self.days,
        hours=self.hours,
        minutes=self.minutes,
        seconds=self.seconds,
        milliseconds=self.milliseconds,
        microseconds=self.microseconds
    )


TimeDelta.__init__ = __new_TimeDelta_init__
TimeDelta.__add__ = __new_TimeDelta_add__
TimeDelta.__sub__ = __new_TimeDelta_sub__
TimeDelta.timedelta = TimeDelta_timedelta

# ------------------------------------------------------------------
# 增强 KData 的遍历
# ------------------------------------------------------------------


def KData_getitem(kdata, i):
    """
    :param i: int | Datetime | slice | str 类型
    """
    if isinstance(i, int):
        length = len(kdata)
        index = length + i if i < 0 else i
        if index < 0 or index >= length:
            raise IndexError("index out of range: %d" % i)
        return kdata.getKRecord(index)

    elif isinstance(i, Datetime):
        return kdata.getKRecordByDate(i)

    elif isinstance(i, str):
        return kdata.getKRecordByDate(Datetime(i))

    elif isinstance(i, slice):
        return [kdata.getKRecord(x) for x in range(*i.indices(len(kdata)))]

    else:
        raise IndexError("Error index type")


def KData_iter(kdata):
    for i in range(len(kdata)):
        yield kdata[i]


def KData_getPos(kdata, datetime):
    """
    获取指定时间对应的索引位置

    :param Datetime datetime: 指定的时间
    :return: 对应的索引位置，如果不在数据范围内，则返回 None
    """
    pos = kdata._getPos(datetime)
    return pos if pos != constant.null_size else None


KData.__getitem__ = KData_getitem
KData.__iter__ = KData_iter
KData.getPos = KData_getPos

# ------------------------------------------------------------------
# 增加转化为 np.array、pandas.DataFrame 的功能
# ------------------------------------------------------------------

try:
    import numpy as np
    import pandas as pd

    def KData_to_np(kdata):
        """转化为numpy结构数组"""
        if kdata.getQuery().kType in ('DAY', 'WEEK', 'MONTH', 'QUARTER', 'HALFYEAR', 'YEAR'):
            k_type = np.dtype(
                {
                    'names': ['datetime', 'open', 'high', 'low', 'close', 'amount', 'volume'],
                    'formats': ['datetime64[D]', 'd', 'd', 'd', 'd', 'd', 'd']
                }
            )
        else:
            k_type = np.dtype(
                {
                    'names': ['datetime', 'open', 'high', 'low', 'close', 'amount', 'volume'],
                    'formats': ['datetime64[ms]', 'd', 'd', 'd', 'd', 'd', 'd']
                }
            )
        return np.array(
            [
                (
                    k.datetime.datetime(), k.openPrice, k.highPrice, k.lowPrice, k.closePrice,
                    k.transAmount, k.transCount
                ) for k in kdata
            ],
            dtype=k_type
        )

    def KData_to_df(kdata):
        """转化为pandas的DataFrame"""
        return pd.DataFrame.from_records(KData_to_np(kdata), index='datetime')

    KData.to_np = KData_to_np
    KData.to_df = KData_to_df

except:
    pass
