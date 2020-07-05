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


#--------------------------------------------------------------------
#
# 待废弃属性
#
#--------------------------------------------------------------------


@deprecated_attr({'size': '__len__', 'get': '__getitem__'})
def Block_getattr(self, name):
    return getattr(self, name)


Block.__getattr__ = Block_getattr


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


@deprecated_attr({'getKData': 'get_kdata'})
def Stock_getattr(self, name):
    return getattr(self, name)


Stock.__getattr__ = Stock_getattr
