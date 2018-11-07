# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog/hikyuu
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

import tables as tb

HDF5_COMPRESS_LEVEL = 9

class H5Record(tb.IsDescription):
    """HDF5基础K线数据格式（日线、分钟线、5分钟线"""
    datetime = tb.UInt64Col()        #IGNORE:E1101
    openPrice = tb.UInt32Col()       #IGNORE:E1101
    highPrice = tb.UInt32Col()       #IGNORE:E1101
    lowPrice = tb.UInt32Col()        #IGNORE:E1101
    closePrice = tb.UInt32Col()      #IGNORE:E1101
    transAmount = tb.UInt64Col()     #IGNORE:E1101
    transCount = tb.UInt64Col()      #IGNORE:E1101


class H5Index(tb.IsDescription):
    """HDF5扩展K线数据格式（周线、月线、季线、半年线、年线、15分钟线、30分钟线、60分钟线"""
    datetime = tb.UInt64Col()        #IGNORE:E1101
    start    = tb.UInt64Col()        #IGNORE:E1101


def open_h5file(dest_dir, market, ktype):
    filename = "{}/{}_{}.h5".format(dest_dir, market.lower(), ktype.lower())
    h5file = tb.open_file(filename, "a", filters=tb.Filters(complevel=HDF5_COMPRESS_LEVEL, complib='zlib', shuffle=True))
    return h5file

def get_h5table(h5file, market, code):
    try:
        group = h5file.get_node("/", "data")
    except:
        group = h5file.create_group("/", "data")

    tablename = market.upper() + code
    try:
        table = h5file.get_node(group, tablename)
    except:
        table = h5file.create_table(group, tablename, H5Record)

    return table