#
# 对 C++ 引出类和函数进行扩展, pybind11 对小函数到导出效率不如 python 直接执行
#

# 优先加载 hikyuu 库，防止 windows 公共依赖库不同导致DLL初始化失败
from .core import *
import numpy as np
import pandas as pd
from datetime import *

# ------------------------------------------------------------------
# 增加Datetime、Stock的hash支持，以便可做为dict的key
# ------------------------------------------------------------------

Datetime.__hash__ = lambda self: self.ticks
TimeDelta.__hash__ = lambda self: self.ticks
Stock.__hash__ = lambda self: self.id

# ------------------------------------------------------------------
# 增强 Datetime
# ------------------------------------------------------------------

__old_Datetime_init__ = Datetime.__init__
__old_Datetime_add__ = Datetime.__add__
__old_Datetime_sub__ = Datetime.__sub__


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
    elif isinstance(td, Datetime):
        return __old_Datetime_sub__(self, td)
    else:
        raise TypeError("unsupported operand type(s) for +: 'TimeDelta' and '{}'".format(type(td)))


def Datetime_date(self):
    """转化生成 python 的 date"""
    return date(self.year, self.month, self.day)


def Datetime_datetime(self):
    """转化生成 python 的 datetime"""
    return datetime(self.year, self.month, self.day, self.hour, self.minute, self.second, self.microsecond)


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
    return __old_TimeDelta_sub__(self, td) if isinstance(td, TimeDelta) else __old_TimeDelta_sub__(self, TimeDelta(td))


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
        return kdata.get(index)

    elif isinstance(i, Datetime):
        return kdata.get_by_datetime(i)

    elif isinstance(i, str):
        return kdata.get_by_datetime(Datetime(i))

    elif isinstance(i, slice):
        return [kdata.get(x) for x in range(*i.indices(len(kdata)))]

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


def KData_getPosInStock(kdata, datetime):
    """
    获取指定时间对应的原始K线中的索引位置

    :param Datetime datetime: 指定的时间
    :return: 对应的索引位置，如果不在数据范围内，则返回 None
    """
    pos = kdata._getPosInStock(datetime)
    return pos if pos != constant.null_size else None


KData.__getitem__ = KData_getitem
KData.__iter__ = KData_iter
KData.get_pos = KData_getPos
KData.get_pos_in_stock = KData_getPosInStock


# ------------------------------------------------------------------
# 重定义Query
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
        构建按索引 [start, end) 方式获取K线数据条件。start，end应同为 int 或 同为 Datetime 类型。

        :param int|Datetime start: 起始索引位置或起始日期
        :param int|Datetime end: 结束索引位置或结束日期
        :param Query.KType ktype: K线数据类型（如日线、分钟线等）
        :param Query.RecoverType recover_type: 复权类型
        :return: 查询条件
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
# 增加转化为 np.array、pandas.DataFrame 的功能
# ------------------------------------------------------------------
def KData_to_np(kdata):
    """转化为numpy结构数组"""
    if kdata.get_query().ktype in ('DAY', 'WEEK', 'MONTH', 'QUARTER', 'HALFYEAR', 'YEAR'):
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
        [(k.datetime.datetime(), k.open, k.high, k.low, k.close, k.amount, k.volume) for k in kdata], dtype=k_type
    )


def KData_to_df(kdata):
    """转化为pandas的DataFrame"""
    return pd.DataFrame.from_records(KData_to_np(kdata), index='datetime')


KData.to_np = KData_to_np
KData.to_df = KData_to_df


def DatetimeList_to_np(data):
    """仅在安装了numpy模块时生效，转换为numpy.array"""
    return np.array(data, dtype='datetime64[D]')


def DatetimeList_to_df(data):
    """仅在安装了pandas模块时生效，转换为pandas.DataFrame"""
    return pd.DataFrame(data.to_np(), columns=('Datetime', ))


DatetimeList.to_np = DatetimeList_to_np
DatetimeList.to_df = DatetimeList_to_df


def TimeLine_to_np(data):
    """转化为numpy结构数组"""
    t_type = np.dtype({'names': ['datetime', 'price', 'vol'], 'formats': ['datetime64[ms]', 'd', 'd']})
    return np.array([(t.date.datetime(), t.price, t.vol) for t in data], dtype=t_type)


def TimeLine_to_df(kdata):
    """转化为pandas的DataFrame"""
    return pd.DataFrame.from_records(TimeLine_to_np(kdata), index='datetime')


TimeLineList.to_np = TimeLine_to_np
TimeLineList.to_df = TimeLine_to_df


def TransList_to_np(data):
    """转化为numpy结构数组"""
    t_type = np.dtype(
        {
            'names': ['datetime', 'price', 'vol', 'direct'],
            'formats': ['datetime64[ms]', 'd', 'd', 'd']
        }
    )
    return np.array([(t.date.datetime(), t.price, t.vol, t.direct) for t in data], dtype=t_type)


def TransList_to_df(kdata):
    """转化为pandas的DataFrame"""
    return pd.DataFrame.from_records(TransList_to_np(kdata), index='datetime')


TransList.to_np = TransList_to_np
TransList.to_df = TransList_to_df

# ------------------------------------------------------------------
# 增强 Parameter
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
    """转化为 Python dict 对象"""
    return dict(self.items())


Parameter.__iter__ = Parameter_iter
Parameter.keys = Parameter_keys
Parameter.items = Parameter_items
Parameter.to_dict = Parameter_to_dict

# ------------------------------------------------------------------
# 增强 StrategyContext
# ------------------------------------------------------------------
__old_StrategyContext_init__ = StrategyContext.__init__


def __new_StrategyContext_init__(self, stock_code_list=None):
    __old_StrategyContext_init__(self)
    if stock_code_list is not None:
        self.stock_list = stock_code_list


StrategyContext.__init__ = __new_StrategyContext_init__
