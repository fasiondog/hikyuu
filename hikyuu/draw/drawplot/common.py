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

#===============================================================================
# history:
# 1. 20171122, Added by fasiondog
#===============================================================================

from hikyuu import Query


def get_draw_title(kdata):
    """根据typ值，返回相应的标题，如 上证指数（日线）
    参数：kdata: KData实例
    返回：一个包含stock名称的字符串，可用作绘图时的标题
    """
    if not kdata:
        return ""

    query = kdata.get_query()
    stock = kdata.get_stock()
    if stock.is_null():
        return ""

    s1 = ''
    if query.ktype == Query.DAY:
        s1 = u' （日线）'
    elif query.ktype == Query.WEEK:
        s1 = u' （周线）'
    elif query.ktype == Query.MONTH:
        s1 = u' （月线）'
    elif query.ktype == Query.QUARTER:
        s1 = u' （季线）'
    elif query.ktype == Query.HALFYEAR:
        s1 = u' （半年线）'
    elif query.ktype == Query.YEAR:
        s1 = u' （年线）'
    elif query.ktype == Query.MIN:
        s1 = u' （1分钟线）'
    elif query.ktype == Query.MIN3:
        s1 = u' （3分钟线）'
    elif query.ktype == Query.MIN5:
        s1 = u' （5分钟线）'
    elif query.ktype == Query.MIN15:
        s1 = u' （15分钟线）'
    elif query.ktype == Query.MIN30:
        s1 = u' （30分钟线）'
    elif query.ktype == Query.MIN60:
        s1 = u' （60分钟线）'
    elif query.ktype == Query.HOUR2:
        s1 = u' （2小时线）'
    elif query.ktype == Query.HOUR4:
        s1 = u' （4小时线）'
    elif query.ktype == Query.HOUR6:
        s1 = u' （6小时线）'
    elif query.ktype == Query.HOUR12:
        s1 = u' （12小时线）'

    name = stock.name

    if stock.code == "":
        stitle = "Block(%s) %s" % (stock.id, name) + s1
    else:
        stitle = stock.market + "/" + stock.code + ' ' + name + s1

    return stitle


def in_interactive_session() -> bool:
    """
    Check if we're running in an interactive shell.

    Returns
    -------
    bool
        True if running under python/ipython interactive shell.
    """
    def check_main():
        try:
            import __main__ as main
        except ModuleNotFoundError:
            return False
        return not hasattr(main, "__file__")

    try:
        # error: Name '__IPYTHON__' is not defined
        return __IPYTHON__ or check_main()  # type: ignore[name-defined]
    except NameError:
        return check_main()


def in_ipython_frontend() -> bool:
    """
    Check if we're inside an IPython zmq frontend.

    Returns
    -------
    bool
    """
    try:
        # error: Name 'get_ipython' is not defined
        ip = get_ipython()  # type: ignore[name-defined]
        return "zmq" in str(type(ip)).lower()
    except NameError:
        pass

    return False