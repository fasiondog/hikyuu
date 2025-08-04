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

import os
import datetime
from pathlib import Path

import clickhouse_connect

from hikyuu import Datetime, UTCOffset
from hikyuu.data.common import get_stktype_list, get_new_holidays
from hikyuu.util import hku_catch, hku_debug, hku_info


def is_exist_db(connect):
    """数据库是否已存在"""
    name = connect.command("SELECT name FROM system.databases WHERE name = 'hku_base'")
    return name == 'hku_base'


@hku_catch(ret=0)
def get_db_version(connect):
    version = connect.command("select `version` from `hku_base`.`version`")
    return version if type(version) == int else 0


def create_database(connect):
    """创建数据库"""
    sql_dir = os.path.dirname(__file__) + '/clickhouse_upgrade'
    if not is_exist_db(connect):
        filename = sql_dir + "/createdb.sql"
        with open(filename, 'r', encoding='utf8') as f:
            sql_string = f.read()
        statements = sql_string.split(';')
        for stmt in statements:
            stmt = stmt.strip()
            if stmt:  # 跳过空语句
                try:
                    connect.command(stmt)
                except Exception as e:
                    print(f"执行失败: {stmt[:30]}... 错误: {e}")

    db_version = get_db_version(connect)
    files = [x for x in Path(sql_dir).iterdir()
             if x.is_file()
             and x.name != 'createdb.sql'
             and x.name != '__init__.py'
             and int(x.stem) > db_version and not x.is_dir()]
    files.sort()
    for file in files:
        print(file)
        sql = file.read_text(encoding='utf8')
        statements = sql.split(';')
        for stmt in statements:
            print(stmt)
            stmt = stmt.strip()
            if stmt:  # 跳过空语句
                try:
                    connect.command(stmt)
                except Exception as e:
                    print(f"执行失败: {stmt[:30]}... 错误: {e}")


def get_codepre_list(connect, market, quotations):
    """获取前缀代码表"""
    stktype_list = get_stktype_list(quotations)
    sql = "select codepre, type from `hku_base`.`coderuletype` " \
          "where market='{market}' and type in {type_list} ORDER by length(codepre) DESC"\
        .format(market=market, type_list=stktype_list)
    ret = connect.query(sql)
    return ret.result_rows


def get_stock_list(connect, market, quotations):
    stktype_list = get_stktype_list(quotations)
    sql = "select market, code, valid, type from `hku_base`.`stock` where market='{}' and type in {}"\
        .format(market, stktype_list)
    a = connect.query(sql)
    return a.result_rows


def import_new_holidays(connect):
    """导入新的交易所休假日历"""
    last_date = connect.command("select date from `hku_base`.`holiday` order by date desc limit 1")
    last_date = last_date if type(last_date) == int else 19901219
    holidays = get_new_holidays()
    new_holidays = [(int(v),) for v in holidays if int(v) > last_date]
    if new_holidays:
        ic = connect.create_insert_context(table='holiday', database='hku_base',
                                           data=new_holidays)
        connect.insert(context=ic)


def get_table(connect, market, code, ktype):
    ktype_dict = {
        'day': 'day',
        'week': 'week',
        'month': 'month',
        'quarter': 'quarter',
        'halfyear': 'halfyear',
        'year': 'year',
        'min': 'min',
        '1min': 'min',
        '5min': 'min5',
        '15min': 'min15',
        '30min': 'min30',
        '60min': 'min60',
        'hour2': 'hour2',
        'min1': 'min',
        'min5': 'min5',
        'min15': 'min15',
        'min30': 'min30',
        'min60': 'min60',
        'timeline': 'timeline',
        'transdata': 'transdata',
    }
    nktype = ktype_dict[ktype.lower()]
    if nktype == 'timeline':
        return (f'hku_data.timeline', market.upper(), code.upper())
    elif nktype == 'transdata':
        return (f'hku_data.transdata', market.upper(), code.upper())
    else:
        return (f'hku_data.{nktype}_k', market.upper(), code.upper())


@hku_catch(ret=Datetime(1970, 1, 1))
def get_lastdatetime(connect, tablename):
    tmp = connect.command("select max(date) from {} where market='{}' and code='{}'".format(
        tablename[0], tablename[1], tablename[2]))
    tmp = Datetime(tmp)
    return None if tmp == Datetime(1970, 1, 1) else tmp + UTCOffset()


def get_last_krecord(connect, tablename):
    """获取最后一条K线记录
    返回：(date, open, close, high, low, amount, volume)
    """
    try:
        a = connect.query(
            "select toUInt32(date), open, high, low, close, amount, volume from {} where market='{}' and code='{}' order by date desc limit 1".format(tablename[0], tablename[1], tablename[2]), settings={'optimize_skip_merged_partitions': 1})
        a = a.result_rows
        # hku_info(f"{tablename} {a}")
        if not a:
            return None
        return (Datetime.from_timestamp_utc(a[0][0]*1000000).ymdhm, a[0][1], a[0][2], a[0][3], a[0][4], a[0][5], a[0][6])
    except:
        return None


def getWeekDate(olddate):
    y = olddate // 100000000
    m = olddate // 1000000 - y * 100
    d = olddate // 10000 - (y * 10000 + m * 100)
    tempdate = datetime.date(y, m, d)
    # python中周一是第0天，周五的第4天
    startdate = tempdate + datetime.timedelta(0 - tempdate.weekday())
    enddate = tempdate + datetime.timedelta(4 - tempdate.weekday())
    return (
        startdate.year * 100000000 + startdate.month * 1000000 + startdate.day * 10000,
        enddate.year * 100000000 + enddate.month * 1000000 + enddate.day * 10000
    )


def getMonthDate(olddate):
    y = olddate // 100000000
    m = olddate // 1000000 - y * 100
    import calendar
    _, d = calendar.monthrange(y, m)
    return (y * 100000000 + m * 1000000 + 10000, y * 100000000 + m * 1000000 + d * 10000)


def getQuarterDate(olddate):
    startDict = {1: 1, 2: 1, 3: 1, 4: 4, 5: 4, 6: 4, 7: 7, 8: 7, 9: 7, 10: 10, 11: 10, 12: 10}
    endDict = {1: 3, 2: 3, 3: 3, 4: 6, 5: 6, 6: 6, 7: 9, 8: 9, 9: 9, 10: 12, 11: 12, 12: 12}
    d_dict = {3: 310000, 6: 300000, 9: 300000, 12: 310000}
    y = olddate // 100000000
    m = olddate // 1000000 - y * 100
    start_m = startDict[m]
    end_m = endDict[m]
    return (y * 100000000 + start_m * 1000000 + 10000, y * 100000000 + end_m * 1000000 + d_dict[end_m])


def getHalfyearDate(olddate):
    y = olddate // 100000000
    m = olddate // 1000000 - y * 100
    return (y * 100000000 + (1010000 if m < 7 else 7010000), y * 100000000 + (6300000 if m < 7 else 12310000))


def getYearDate(olddate):
    y = olddate // 100000000
    return (y * 100000000 + 1010000, y * 100000000 + 12310000)


def getMin60Date(olddate):
    mint = olddate - olddate // 10000 * 10000
    newdate = olddate // 10000 * 10000
    if mint <= 1030:
        startdate = newdate + 931
        enddate = newdate + 1030
    elif mint <= 1130:
        startdate = newdate + 1031
        enddate = newdate + 1130
    elif mint <= 1400:
        startdate = newdate + 1301
        enddate = newdate + 1400
    else:
        startdate = newdate + 1401
        enddate = newdate + 1500
    return (startdate, enddate)


def getHour2Date(olddate):
    mint = olddate - olddate // 10000 * 10000
    newdate = olddate // 10000 * 10000
    if mint <= 1130:
        startdate = newdate + 931
        enddate = newdate + 1130
    else:
        startdate = newdate + 1301
        enddate = newdate + 1500
    return (startdate, enddate)


def getMin15Date(olddate):
    mint = olddate - olddate // 10000 * 10000
    newdate = olddate // 10000 * 10000
    if mint <= 945:
        startdate = newdate + 931
        enddate = newdate + 945
    elif mint <= 1000:
        startdate = newdate + 946
        enddate = newdate + 1000
    elif mint <= 1015:
        startdate = newdate + 1001
        enddate = newdate + 1015
    elif mint <= 1030:
        startdate = newdate + 1016
        enddate = newdate + 1030
    elif mint <= 1045:
        startdate = newdate + 1031
        enddate = newdate + 1045
    elif mint <= 1100:
        startdate = newdate + 1046
        enddate = newdate + 1100
    elif mint <= 1115:
        startdate = newdate + 1101
        enddate = newdate + 1115
    elif mint <= 1130:
        startdate = newdate + 1116
        enddate = newdate + 1130
    elif mint <= 1315:
        startdate = newdate + 1301
        enddate = newdate + 1315
    elif mint <= 1330:
        startdate = newdate + 1316
        enddate = newdate + 1330
    elif mint <= 1345:
        startdate = newdate + 1331
        enddate = newdate + 1345
    elif mint <= 1400:
        startdate = newdate + 1346
        enddate = newdate + 1400
    elif mint <= 1415:
        startdate = newdate + 1401
        enddate = newdate + 1415
    elif mint <= 1430:
        startdate = newdate + 1416
        enddate = newdate + 1430
    elif mint <= 1445:
        startdate = newdate + 1431
        enddate = newdate + 1445
    else:
        startdate = newdate + 1446
        enddate = newdate + 1500
    return (startdate, enddate)


def getMin30Date(olddate):
    mint = olddate - olddate // 10000 * 10000
    newdate = olddate // 10000 * 10000
    if mint <= 1000:
        startdate = newdate + 931
        enddate = newdate + 1000
    elif mint <= 1030:
        startdate = newdate + 1001
        enddate = newdate + 1030
    elif mint <= 1100:
        startdate = newdate + 1031
        enddate = newdate + 1100
    elif mint <= 1130:
        startdate = newdate + 1101
        enddate = newdate + 1130
    elif mint <= 1330:
        startdate = newdate + 1301
        enddate = newdate + 1330
    elif mint <= 1400:
        startdate = newdate + 1331
        enddate = newdate + 1400
    elif mint <= 1430:
        startdate = newdate + 1401
        enddate = newdate + 1430
    else:
        startdate = newdate + 1431
        enddate = newdate + 1500
    return (startdate, enddate)


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


@hku_catch(ret={'week': [], 'month': [], 'quarter': [], 'halfyear': [], 'year': [], 'min15': [], 'min30': [], 'min60': [], 'hour2': []}, trace=True)
def update_extern_data(connect, market, code, data_type):
    """更新周线、月线、15分钟线等扩展数据索引"""
    if data_type.lower() == 'day':
        index_list = ('week', 'month', 'quarter', 'halfyear', 'year')
        base_table = get_table(connect, market, code, 'day')
    else:
        index_list = ('min15', 'min30', 'min60', 'hour2')
        base_table = get_table(connect, market, code, 'min5')

    index_data = {}
    for index_type in index_list:
        index_data[index_type] = []

    base_lastdate = get_lastdatetime(connect, base_table)
    if base_lastdate is None:
        return index_data

    for index_type in index_list:
        hku_debug("{}{} update {} index".format(market, code, index_type))
        index_table = get_table(connect, market, code, index_type)
        index_last_date = get_lastdatetime(connect, index_table)

        # 获取当前日期大于等于索引表最大日期的基础表日期列表
        if index_last_date is None:
            sql = f"select toUInt32(date), open, high, low, close, amount, volume from {base_table[0]} where market='{base_table[1]}' and code='{base_table[2]}' order by date asc"
        else:
            index_last_date = index_last_date.ymdhm
            start_date, _ = getNewDate(index_type, index_last_date)
            start_date = Datetime(start_date).timestamp_utc() // 1000000
            sql = f"select toUInt32(date), open, high, low, close, amount, volume from {base_table[0]} where market='{base_table[1]}' and code='{base_table[2]}' and date>={start_date} order by date asc"
        a = connect.query(sql, settings={'optimize_skip_merged_partitions': 1})
        base_list = a.result_rows

        last_start_date = 199012010000
        last_end_date = 199012010000

        insert_buffer = index_data[index_type]
        length_base_all = len(base_list)
        for x in range(length_base_all):
            current_date = Datetime.from_timestamp_utc(base_list[x][0] * 1000000).ymdhm
            if current_date <= last_end_date:
                continue
            last_start_date, last_end_date = getNewDate(index_type, current_date)

            base_record_list = []
            start_ix = x
            ix_date = current_date
            while start_ix < length_base_all and \
                    ix_date >= last_start_date and ix_date <= last_end_date:
                base_record_list.append(base_list[start_ix])
                ix_date = Datetime.from_timestamp_utc(base_list[start_ix][0]*1000000).ymdhm
                start_ix += 1

            if not base_record_list:
                continue

            length = len(base_record_list)
            open_price = base_record_list[0][1]
            high_price = base_record_list[0][2]
            low_price = base_record_list[0][3]
            close_price = base_record_list[length - 1][4]
            amount = base_record_list[0][5]
            count = base_record_list[0][6]
            for i in range(1, length):
                if base_record_list[i][2] > high_price:
                    high_price = base_record_list[i][2]
                if base_record_list[i][3] < low_price:
                    low_price = base_record_list[i][3]
                amount += base_record_list[i][5]
                count += base_record_list[i][6]

            last_timestamp = Datetime(last_end_date).timestamp_utc()//1000000
            if last_end_date != index_last_date:
                insert_buffer.append((index_table[1], index_table[2], last_timestamp, open_price,
                                      high_price, low_price, close_price, amount, count))

        if len(insert_buffer) > 200000:
            hku_info(f"写入 {market} {index_table[0]} 扩展数据: {len(insert_buffer)} ...")
            ic = connect.create_insert_context(table=index_table[0],
                                               data=insert_buffer)
            connect.insert(context=ic, settings={"prefer_warmed_unmerged_parts_seconds": 86400})
            insert_buffer.clear()
    return index_data


if __name__ == '__main__':
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    # dev_config.read('d:/workspace/dev.ini')
    db = 'clickhouse54-http'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    import clickhouse_connect
    client = clickhouse_connect.get_client(
        host=host, username=user, password=password)

    print(is_exist_db(client))
    create_database(client)
    print(is_exist_db(client))

    # x = get_codepre_list(client, 'SH', ['stock',  'fund'])
    # print(x)

    # import_new_holidays(client)
    x = get_last_krecord(client, ('hku_data.min_k', 'SH', '000001'))
    print(x)

    update_extern_data(client, 'BJ', '430017', 'DAY')
    client.close()
