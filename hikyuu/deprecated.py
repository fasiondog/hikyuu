# 已废弃接口

from .extend import *
from .util.mylog import escapetime


def deprecated_func(new_func, old_func_name, new_func_name):
    def wrap_deprecated_func(func):
        def wrapfunc(*args, **kwarg):
            print(
                'Deprecated warning: "{}" will be deprecated, please use: "{}"'.format(
                    old_func_name, new_func_name
                )
            )
            return new_func(*args, **kwarg)

        return wrapfunc

    return wrap_deprecated_func


def deprecated_attr(name_dict):
    """ name_dict: {'old_property_name': 'new_property_name'}"""
    def wrap_deprecated_attr(func):
        def wrapfunc(self, name):
            clzname = self.__class__.__name__
            if name in name_dict:
                if name_dict[name] is None:
                    print('Removed warning: the {}.{} will be removed!'.format(clzname, name))
                else:
                    print(
                        'Deprecated warning: the "{}.{}" will be deprecated, please use: "{}.{}"'.
                        format(clzname, name, clzname, name_dict[name])
                    )
                return func(self, name_dict[name])
            return func(self, name)

        return wrapfunc

    return wrap_deprecated_attr


#--------------------------------------------------------------------
#
# 待废弃函数
#
#--------------------------------------------------------------------


@deprecated_func(get_version, 'getVersion', 'get_version')
def getVersion():
    pass


@deprecated_func(get_stock, 'getStock', 'get_stock')
def getStock(querystr):
    pass


@deprecated_func(Query, 'KQuery', 'Query')
def KQuery(*args, **kwargs):
    pass


@deprecated_func(Query, 'QueryByIndex', 'Query')
def QueryByIndex(*args, **kwargs):
    pass


@deprecated_func(Query, 'QueryByDate', 'Query')
def QueryByDate(*args, **kwargs):
    pass


@deprecated_func(get_date_range, 'getDateRange', 'get_date_range')
def getDateRange(*args, **kwargs):
    pass


#--------------------------------------------------------------------
#
# 待废弃属性
#
#--------------------------------------------------------------------


@deprecated_attr(
    {
        'isNull': 'is_null',
        'dayOfWeek': 'day_of_week',
        'dayOfYear': 'day_of_year',
        'startOfDay': 'start_of_day',
        'endOfDay': 'end_of_day',
        'nextDay': 'next_day',
        'nextWeek': 'next_week',
        'nextMonth': 'next_month',
        'nextQuarter': 'next_quarter',
        'nextHalfyear': 'next_halfyear',
        'nextYear': 'next_year',
        'preDay': 'pre_day',
        'preWeek': 'pre_week',
        'preMonth': 'pre_month',
        'preQuarter': 'pre_quarter',
        'preHalfyear': 'pre_halfyear',
        'preYear': 'pre_year',
        'dateOfWeek': 'date_of_week',
        'startOfWeek': 'start_of_week',
        'endOfWeek': 'end_of_week',
        'startOfMonth': 'start_of_month',
        'endOfMonth': 'end_of_month',
        'startOfQuarter': 'start_of_quarter',
        'endOfQuarter': 'end_of_quarter',
        'startOfHalfyear': 'start_of_halfyear',
        'endOfHalfyear': 'end_of_halfyear',
        'startOfYear': 'start_of_year',
        'endOfYear': 'end_of_year'
    }
)
def Datetime_getattr(self, name):
    return getattr(self, name)


Datetime.__getattr__ = Datetime_getattr


@deprecated_attr({'size': '__len__', 'get': '__getitem__'})
def Block_getattr(self, name):
    return getattr(self, name)


Block.__getattr__ = Block_getattr


@deprecated_attr(
    {
        'startPos': 'start_pos',
        'endPos': 'end_pos',
        'lastPos': 'last_pos',
        'getPos': 'get_pos',
        'getDatetimeList': 'get_date_list',
        'getByDate': 'get_by_date',
        'getQuery': 'get_query',
        'getStock': 'get_stock',
    }
)
def KData_getattr(self, name):
    return getattr(self, name)


KData.__getattr__ = KData_getattr


@deprecated_attr(
    {
        'openPrice': 'open',
        'closePrice': 'close',
        'highPrice': 'high',
        'lowPrice': 'low',
        'transAmount': 'amount',
        'transCount': 'volume'
    }
)
def KRecord_getattr(self, name):
    return getattr(self, name)


KRecord.__getattr__ = KRecord_getattr


@deprecated_attr({'lastDate': 'last_date'})
def MarketInfo_getattr(self, name):
    return getattr(self, name)


MarketInfo.__getattr__ = MarketInfo_getattr


@deprecated_attr(
    {
        'startDatetime': 'start_date',
        'endDatetime': 'end_date',
        'queryType': 'query_type',
        'kType': 'ktype',
        'recoverType': 'recover_type'
    }
)
def Query_getattr(self, name):
    return getattr(self, name)


Query.__getattr__ = Query_getattr


@deprecated_attr({'getKData': 'get_kdata'})
def Stock_getattr(self, name):
    return getattr(self, name)


Stock.__getattr__ = Stock_getattr
