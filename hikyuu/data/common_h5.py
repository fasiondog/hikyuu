# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
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

import datetime
import tables as tb

from hikyuu.util import hku_catch
from hikyuu.util.mylog import hku_error, hku_debug

HDF5_COMPRESS_LEVEL = 9


class H5Record(tb.IsDescription):
    """HDF5基础K线数据格式（日线、分钟线、5分钟线"""
    datetime = tb.UInt64Col()  #IGNORE:E1101
    openPrice = tb.UInt32Col()  #IGNORE:E1101
    highPrice = tb.UInt32Col()  #IGNORE:E1101
    lowPrice = tb.UInt32Col()  #IGNORE:E1101
    closePrice = tb.UInt32Col()  #IGNORE:E1101
    transAmount = tb.UInt64Col()  #IGNORE:E1101
    transCount = tb.UInt64Col()  #IGNORE:E1101


class H5Index(tb.IsDescription):
    """HDF5扩展K线数据格式（周线、月线、季线、半年线、年线、15分钟线、30分钟线、60分钟线"""
    datetime = tb.UInt64Col()  #IGNORE:E1101
    start = tb.UInt64Col()  #IGNORE:E1101


class H5Transaction(tb.IsDescription):
    """分笔数据"""
    datetime = tb.UInt64Col()
    price = tb.UInt64Col()
    vol = tb.UInt64Col()
    buyorsell = tb.UInt8Col()


class H5TransactionIndex(tb.IsDescription):
    """分笔数据按天索引"""
    datetime = tb.UInt64Col()
    start = tb.UInt64Col()


class H5MinuteTime(tb.IsDescription):
    """分时线"""
    datetime = tb.UInt64Col()
    price = tb.UInt64Col()
    vol = tb.UInt64Col()


#------------------------------------------------------------------------------
# K线数据
#------------------------------------------------------------------------------


def open_h5file(dest_dir, market, ktype):
    filename = "{}/{}_{}.h5".format(dest_dir, market.lower(), ktype.lower())
    h5file = tb.open_file(
        filename, "a", filters=tb.Filters(complevel=HDF5_COMPRESS_LEVEL, complib='zlib', shuffle=True)
    )
    return h5file


@hku_catch(None, trace=True)
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


def update_hdf5_extern_data(h5file, tablename, data_type):
    """更新周线、月线、15分钟线等扩展数据索引"""
    def getWeekDate(olddate):
        y = olddate // 100000000
        m = olddate // 1000000 - y * 100
        d = olddate // 10000 - (y * 10000 + m * 100)
        tempdate = datetime.date(y, m, d)
        # python中周一是第0天，周五的第4天
        tempweekdate = tempdate + datetime.timedelta(4 - tempdate.weekday())
        newdate = tempweekdate.year * 100000000 + tempweekdate.month * 1000000 + tempweekdate.day * 10000
        return newdate

    def getMonthDate(olddate):
        y = olddate // 100000000
        m = olddate // 1000000 - y * 100
        import calendar
        _, d = calendar.monthrange(y, m)
        return (y * 100000000 + m * 1000000 + d * 10000)

    def getQuarterDate(olddate):
        quarterDict = {1: 3, 2: 3, 3: 3, 4: 6, 5: 6, 6: 6, 7: 9, 8: 9, 9: 9, 10: 12, 11: 12, 12: 12}
        d_dict = {3: 310000, 6: 300000, 9: 300000, 12: 310000}
        y = olddate // 100000000
        m = olddate // 1000000 - y * 100
        new_m = quarterDict[m]
        return (y * 100000000 + new_m * 1000000 + d_dict[new_m])

    def getHalfyearDate(olddate):
        y = olddate // 100000000
        m = olddate // 1000000 - y * 100
        return y * 100000000 + (6300000 if m < 7 else 12310000)

    def getYearDate(olddate):
        y = olddate // 100000000
        return (y * 100000000 + 12310000)

    def getMin60Date(olddate):
        mint = olddate - olddate // 10000 * 10000
        if mint <= 1030:
            newdate = olddate // 10000 * 10000 + 1030
        elif mint <= 1130:
            newdate = olddate // 10000 * 10000 + 1130
        elif mint <= 1400:
            newdate = olddate // 10000 * 10000 + 1400
        else:
            newdate = olddate // 10000 * 10000 + 1500
        return newdate
    def getHour2Date(olddate):
        mint = olddate - olddate // 10000 * 10000
        if mint <= 1130:
            newdate = olddate // 10000 * 10000 + 1130
        else:
            newdate = olddate // 10000 * 10000 + 1500

        return newdate
    def getMin15Date(olddate):
        mint = olddate - olddate // 10000 * 10000
        if mint <= 945:
            newdate = olddate // 10000 * 10000 + 945
        elif mint <= 1000:
            newdate = olddate // 10000 * 10000 + 1000
        elif mint <= 1015:
            newdate = olddate // 10000 * 10000 + 1015
        elif mint <= 1030:
            newdate = olddate // 10000 * 10000 + 1030
        elif mint <= 1045:
            newdate = olddate // 10000 * 10000 + 1045
        elif mint <= 1100:
            newdate = olddate // 10000 * 10000 + 1100
        elif mint <= 1115:
            newdate = olddate // 10000 * 10000 + 1115
        elif mint <= 1130:
            newdate = olddate // 10000 * 10000 + 1130
        elif mint <= 1315:
            newdate = olddate // 10000 * 10000 + 1315
        elif mint <= 1330:
            newdate = olddate // 10000 * 10000 + 1330
        elif mint <= 1345:
            newdate = olddate // 10000 * 10000 + 1345
        elif mint <= 1400:
            newdate = olddate // 10000 * 10000 + 1400
        elif mint <= 1415:
            newdate = olddate // 10000 * 10000 + 1415
        elif mint <= 1430:
            newdate = olddate // 10000 * 10000 + 1430
        elif mint <= 1445:
            newdate = olddate // 10000 * 10000 + 1445
        else:
            newdate = olddate // 10000 * 10000 + 1500
        return newdate

    def getMin30Date(olddate):
        mint = olddate - olddate // 10000 * 10000
        if mint <= 1000:
            newdate = olddate // 10000 * 10000 + 1000
        elif mint <= 1030:
            newdate = olddate // 10000 * 10000 + 1030
        elif mint <= 1100:
            newdate = olddate // 10000 * 10000 + 1100
        elif mint <= 1130:
            newdate = olddate // 10000 * 10000 + 1130
        elif mint <= 1330:
            newdate = olddate // 10000 * 10000 + 1330
        elif mint <= 1400:
            newdate = olddate // 10000 * 10000 + 1400
        elif mint <= 1430:
            newdate = olddate // 10000 * 10000 + 1430
        else:
            newdate = olddate // 10000 * 10000 + 1500
        return newdate

    def getNewDate(index_type, olddate):
        if index_type == 'week':
            return getWeekDate(olddate)
        elif index_type == 'month':
            return getMonthDate(olddate)
        elif index_type == 'quarter':
            return getQuarterDate(olddate)
        elif index_type == 'halfyear':
            return getHalfyearDate(olddate)
        elif index_type == 'year':
            return getYearDate(olddate)
        elif index_type == 'min15':
            return getMin15Date(olddate)
        elif index_type == 'min30':
            return getMin30Date(olddate)
        elif index_type == 'min60':
            return getMin60Date(olddate)
        elif index_type == 'hour2':
            return getHour2Date(olddate)
        else:
            return None

    if data_type == 'DAY':
        index_list = ('week', 'month', 'quarter', 'halfyear', 'year')
    else:
        index_list = ('min15', 'min30', 'min60', 'hour2')

    groupDict = {}
    for index_type in index_list:
        try:
            groupDict[index_type] = h5file.get_node("/", index_type)
        except:
            groupDict[index_type] = h5file.create_group("/", index_type)

    try:
        table = h5file.get_node("/data", tablename)
    except Exception as e:
        hku_error("{}".format(e))
        return

    for index_type in index_list:
        hku_debug("{} update {} index".format(tablename, index_type))
        try:
            index_table = h5file.get_node(groupDict[index_type], tablename)
        except:
            index_table = h5file.create_table(groupDict[index_type], tablename, H5Index)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(index_type, int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(index_type, date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()

        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(index_type, date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()


#------------------------------------------------------------------------------
# 分笔数据
#------------------------------------------------------------------------------


def open_trans_file(dest_dir, market):
    filename = "{}/{}_trans.h5".format(dest_dir, market.lower())
    h5file = tb.open_file(
        filename, "a", filters=tb.Filters(complevel=HDF5_COMPRESS_LEVEL, complib='zlib', shuffle=True)
    )
    return h5file


@hku_catch(trace=True)
def get_trans_table(h5file, market, code):
    try:
        group = h5file.get_node("/", "data")
    except:
        group = h5file.create_group("/", "data")

    tablename = market.upper() + code
    try:
        table = h5file.get_node(group, tablename)
    except:
        table = h5file.create_table(group, tablename, H5Transaction)

    return table


def update_hdf5_trans_index(h5file, tablename):
    """更新分笔数据按日索引"""
    hku_debug("{} update trans index".format(tablename))
    try:
        table = h5file.get_node("/data", tablename)
    except:
        return

    total = table.nrows
    if 0 == total:
        return

    try:
        group = h5file.get_node("/", 'index')
    except:
        group = h5file.create_group("/", 'index')

    try:
        index_table = h5file.get_node(group, tablename)
    except:
        index_table = h5file.create_table(group, tablename, H5Index)

    index_total = index_table.nrows
    index_row = index_table.row
    if index_total:
        index_last_date = int(index_table[-1]['datetime'])
        last_date = int(table[-1]['datetime'] // 1000000 * 10000)
        if index_last_date == last_date:
            return
        startix = int(index_table[-1]['start'])
        pre_index_date = index_last_date
    else:
        startix = 0
        date = int(table[0]['datetime'] // 1000000 * 10000)
        pre_index_date = date
        index_row['datetime'] = pre_index_date
        index_row['start'] = 0
        index_row.append()

    index = startix
    for row in table[startix:]:
        date = int(row['datetime'] // 1000000 * 10000)
        cur_index_date = date
        if cur_index_date != pre_index_date:
            index_row['datetime'] = cur_index_date
            index_row['start'] = index
            index_row.append()
            pre_index_date = cur_index_date
        index += 1
    index_table.flush()


#------------------------------------------------------------------------------
# 分时数据
#------------------------------------------------------------------------------
def open_time_file(dest_dir, market):
    filename = "{}/{}_time.h5".format(dest_dir, market.lower())
    h5file = tb.open_file(
        filename, "a", filters=tb.Filters(complevel=HDF5_COMPRESS_LEVEL, complib='zlib', shuffle=True)
    )
    return h5file


@hku_catch(None, trace=True)
def get_time_table(h5file, market, code):
    try:
        group = h5file.get_node("/", "data")
    except:
        group = h5file.create_group("/", "data")

    tablename = market.upper() + code
    try:
        table = h5file.get_node(group, tablename)
    except:
        table = h5file.create_table(group, tablename, H5MinuteTime)

    return table