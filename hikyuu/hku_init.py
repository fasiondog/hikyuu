#
# 对 C++ 引出的类型进行补充定义
#

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