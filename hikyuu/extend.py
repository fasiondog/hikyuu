#
# Extend the classes and functions exported from C++; the pybind11 export efficiency for small functions is not as good as executing them directly in python
#

# Load the hikyuu library first, to prevent the DLL initialization failure caused by different common dependency libraries on windows
from .core import *

# Filter out the numpy warnings
import os
os.environ["NUMEXPR_MAX_THREADS"] = str(os.cpu_count())

from datetime import datetime, timedelta, date  # NOQA: E402
import numpy as np  # NOQA: E402
import pandas as pd  # NOQA: E402

# Solve the Chinese alignment problem
pd.set_option('display.unicode.ambiguous_as_wide', True)
pd.set_option('display.unicode.east_asian_width', True)

# ------------------------------------------------------------------
# Add hash support for Datetime and Stock, so that they can be used as the keys of a dict
# ------------------------------------------------------------------

Datetime.__hash__ = lambda self: self.ticks
TimeDelta.__hash__ = lambda self: self.ticks
Stock.__hash__ = lambda self: self.id

# ------------------------------------------------------------------
# Enhance Datetime
# ------------------------------------------------------------------

__old_Datetime_init__ = Datetime.__init__
__old_Datetime_add__ = Datetime.__add__
__old_Datetime_sub__ = Datetime.__sub__


def __new_Datetime_add__(self, td):
    """Add the specified time duration; the duration object can be a TimeDelta or a datetime.timedelta type

    :param TimeDelta td: the time duration
    :rtype: Datetime
    """
    if isinstance(td, TimeDelta):
        return __old_Datetime_add__(self, td)
    elif isinstance(td, timedelta):
        return __old_Datetime_add__(self, TimeDelta(td))
    else:
        raise TypeError("unsupported operand type(s) for +: 'TimeDelta' and '{}'".format(type(td)))


def __new_Datetime_sub__(self, td):
    """Subtract the specified time duration; the duration object can be a TimeDelta or a datetime.timedelta type

    :param TimeDelta td: the specified time duration
    :rtype: Datetime
    """
    if isinstance(td, TimeDelta):
        return __old_Datetime_sub__(self, td)
    elif isinstance(td, timedelta):
        return __old_Datetime_sub__(self, TimeDelta(td))
    elif isinstance(td, Datetime):
        return __old_Datetime_sub__(self, td)
    else:
        raise TypeError("unsupported operand type(s) for +: 'TimeDelta' and '{}'".format(type(td)))


def Datetime_date(self):
    """Convert to a python date"""
    return date(self.year, self.month, self.day)


def Datetime_datetime(self):
    """Convert to a python datetime"""
    return datetime(self.year, self.month, self.day, self.hour, self.minute, self.second, self.microsecond)


Datetime.__add__ = __new_Datetime_add__
Datetime.__radd__ = __new_Datetime_add__
Datetime.__sub__ = __new_Datetime_sub__
Datetime.date = Datetime_date
Datetime.datetime = Datetime_datetime

# ------------------------------------------------------------------
# Enhance TimeDelta
# ------------------------------------------------------------------

__old_TimeDelta_init__ = TimeDelta.__init__
__old_TimeDelta_add__ = TimeDelta.__add__
__old_TimeDelta_sub__ = TimeDelta.__sub__


def __new_TimeDelta_init__(self, *args, **kwargs):
    """
    It can be built in the following ways:

    - Built from a datetime.timedelta. TimeDelta(timedelta instance)
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
    """Can be added with TimeDelta, datetime.timedelta and Datetime"""
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
    """Can subtract TimeDelta and datetime.timedelta"""
    return __old_TimeDelta_sub__(self, td) if isinstance(td, TimeDelta) else __old_TimeDelta_sub__(self, TimeDelta(td))


def TimeDelta_timedelta(self):
    """ Convert to a datetime.timedelta """
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
# Redefine Query
# ------------------------------------------------------------------

Query.INDEX = Query.QueryType.INDEX
Query.DATE = Query.QueryType.DATE
Query.DAY = "DAY"
Query.WEEK = "WEEK"
Query.MONTH = "MONTH"
Query.QUARTER = "QUARTER"
Query.HALFYEAR = "HALFYEAR"
Query.YEAR = "YEAR"
Query.MIN = "MIN"
Query.MIN3 = "MIN3"
Query.MIN5 = "MIN5"
Query.MIN15 = "MIN15"
Query.MIN30 = "MIN30"
Query.MIN60 = "MIN60"
Query.HOUR2 = "HOUR2"
Query.HOUR4 = "HOUR4"
Query.HOUR6 = "HOUR6"
Query.HOUR12 = "HOUR12"
Query.NO_RECOVER = Query.RecoverType.NO_RECOVER
Query.FORWARD = Query.RecoverType.FORWARD
Query.BACKWARD = Query.RecoverType.BACKWARD
Query.EQUAL_FORWARD = Query.RecoverType.EQUAL_FORWARD
Query.EQUAL_BACKWARD = Query.RecoverType.EQUAL_BACKWARD

old_Query_init = Query.__init__


def new_Query_init(self, start=0, end=None, ktype=Query.DAY, recover_type=Query.NO_RECOVER):
    """
        Build the condition to get the K-line data by index in the [start, end) way. start and end should both be int or both be Datetime.

        :param int|Datetime start: the start index position or the start date
        :param int|Datetime end: the end index position or the end date
        :param Query.KType ktype: the K-line data type (such as daily, minute, etc.)
        :param Query.RecoverType recover_type: the recovery type
        :return: the query condition
        :rtype: KQuery
        """
    if isinstance(start, int):
        end_pos = constant.null_int64 if end is None else end
    elif isinstance(start, Datetime):
        end_pos = constant.null_datetime if end is None else end
    else:
        raise TypeError('Incorrect parameter type error!')
    old_Query_init(self, start, end_pos, ktype, recover_type)


Query.__init__ = new_Query_init


# ------------------------------------------------------------------
# Add the conversion to np.array and pandas.DataFrame
# ------------------------------------------------------------------
def DatetimeList_to_np(data: DatetimeList):
    return dates_to_np(data)


def DatetimeList_to_df(data: DatetimeList):
    return pd.DataFrame.from_records(dates_to_np(data))


DatetimeList.to_np = DatetimeList_to_np
DatetimeList.to_df = DatetimeList_to_df
DatetimeList.to_numpy = DatetimeList.to_np
DatetimeList.to_pandas = DatetimeList.to_df


TimeLineList.to_np = lambda data: timeline_to_np(data)
TimeLineList.to_df = lambda data: timeline_to_df(data)
TimeLineList.to_numpy = TimeLineList.to_np
TimeLineList.to_pandas = TimeLineList.to_df


TransList.to_np = lambda data: translist_to_np(data)
TransList.to_df = lambda data: translist_to_df(data)
TransList.to_numpy = TransList.to_np
TransList.to_pandas = TransList.to_df

StockWeightList.to_np = lambda data: weights_to_np(data)
StockWeightList.to_df = lambda data: weights_to_df(data)
StockWeightList.to_numpy = StockWeightList.to_np
StockWeightList.to_pandas = StockWeightList.to_df

KRecordList.to_np = lambda data: krecords_to_np(data)
KRecordList.to_df = lambda data: krecords_to_df(data)
KRecordList.to_numpy = KRecordList.to_np
KRecordList.to_pandas = KRecordList.to_df

KData.to_numpy = KData.to_np
KData.to_pandas = KData.to_df


# ------------------------------------------------------------------
# Enhance Parameter
# ------------------------------------------------------------------


def Parameter_iter(self):
    name_list = self.get_name_list()
    for key in name_list:
        yield self[key]


def Parameter_keys(self):
    return list(self.get_name_list())


def Parameter_items(self):
    return [(key, self[key]) for key in self.get_name_list()]


def Parameter_to_dict(self):
    """Convert to a Python dict object"""
    return dict(self.items())


Parameter.__iter__ = Parameter_iter
Parameter.keys = Parameter_keys
Parameter.items = Parameter_items
Parameter.to_dict = Parameter_to_dict
