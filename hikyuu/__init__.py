#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

__copyright__ = """
MIT License

Copyright (c) 2010-2017 fasiondog

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import traceback
import sys
import pickle

from .util import *

try:
    from .extend import *
    # from .deprecated import *  # pybind getattr 会被递归，无法使用
    from .indicator import *
    from .trade_manage import *
    from .trade_sys import *
    from .analysis import *
    from .hub import *
except Exception as e:
    info = sys.exc_info()
    hku_fatal("{}: {}".format(info[0].__name__, info[1]))
    hku_fatal(traceback.format_exc())
    print("""请使用 pipdeptree -p hikyuu 检查是否存在缺失的依赖包。
如果没有问题可以在 https://gitee.com/fasiondog/hikyuu 或 https://github.com/fasiondog/hikyuu 
上提交 issue，同时附上 "用户目录/.hikyuu" 下的 hikyuu_py.log 和 hikyuu.log 日志文件 """)
    raise e

__version__ = get_version()

sm = StockManager.instance()


class iodog:
    # Only for compatibility with old code
    @staticmethod
    def open():
        open_ostream_to_python()

    @staticmethod
    def close():
        close_ostream_to_python()


if in_interactive_session():
    set_python_in_interactive(True)


# 如果是在 jupyter 环境中运行，重定向C++ stdout/stderr输出至python
if in_ipython_frontend():
    set_python_in_jupyter(True)
    hku_info("hikyuu version: {}", get_version_with_build())
    iodog.open()


def hku_save(var, filename):
    """
    序列化，将hikyuu内建类型的变量（如Stock、TradeManager等）保存在指定的文件中，格式为XML。

    :param var: hikyuu内建类型的变量
    :param str filename: 指定的文件名
    """
    with open(filename, 'wb') as f:
        pickle.dump(var, f)


def hku_load(filename):
    """
    将通过 hku_save 保存的变量，读取到var中。

    :param str filename: 待载入的序列化文件。
    :return: 之前被序列化保存的文件    
    """
    with open(filename, 'rb') as f:
        out = pickle.load(f)
    return out


# ==============================================================================
#
# 设置关键类型简称
#
# ==============================================================================
O = OPEN()
C = CLOSE()
H = HIGH()
L = LOW()
A = AMO()
V = VOL()
D = Datetime
K = None
Q = Query
