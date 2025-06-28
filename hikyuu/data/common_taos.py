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

from hikyuu import Datetime, UTCOffset
from hikyuu.data.common import get_stktype_list, get_new_holidays
from hikyuu.util import hku_debug, hku_catch, hku_info

import importlib

_g_taos = None


def get_taos():
    try:
        global _g_taos
        if _g_taos is not None:
            return _g_taos
        _g_taos = importlib.import_module('taos')
        return _g_taos
    except:
        hku_info("Failed import taos.")
        return None


def reload_taos():
    global _g_taos
    if _g_taos is not None:
        importlib.reload(_g_taos)


def is_exist_db(connect):
    """数据库是否已存在"""
    cur = connect.cursor()
    cur.execute("SELECT name FROM information_schema.ins_databases WHERE name='hku_base'")
    a = [x for x in cur]
    cur.close()
    return True if a else False


def get_db_version(connect):
    try:
        cur = connect.cursor()
        cur.execute("select `version` from `hku_base`.`version`")
        a = [x for x in cur]
        cur.close()
        return a[0][0] if a else 0
    except:
        return 0


def create_database(connect):
    """创建数据库"""
    sql_dir = os.path.dirname(__file__) + "/taos_upgrade"
    cur = connect.cursor()
    if not is_exist_db(connect):
        filename = sql_dir + "/createdb.sql"
        with open(filename, 'r', encoding='utf8') as f:
            sql = f.read()
        x = sql.split(';')
        for v in x:
            sql = v.strip()
            if sql and sql[0] != '-':
                cur.execute(v)

    db_version = get_db_version(connect)
    files = [x for x in Path(sql_dir).iterdir()
             if x.is_file()
             and x.name != 'createdb.sql'
             and x.name != '__init__.py'
             and int(x.stem) > db_version and not x.is_dir()]
    files.sort()
    for file in files:
        sql = file.read_text(encoding='utf8')
        sqls = sql.split(';')
        for v in sqls:
            sql = v.strip()
            if sql and sql[0] != '-':
                cur.execute(sql)

    connect.commit()
    cur.close()


def get_marketid(connect, market):
    cur = connect.cursor()
    cur.execute("select marketid, market from hku_base.n_market where market='{}'".format(market.upper()))
    marketid = [v for v in cur]
    marketid = marketid[0][0]
    cur.close()
    return marketid


def get_codepre_list(connect, marketid, quotations):
    """获取前缀代码表"""
    stktype_list = get_stktype_list(quotations)
    sql = "select codepre, type from `hku_base`.`n_coderuletype` " \
          "where marketid={marketid} and type in {type_list} ORDER by length(codepre) DESC"\
        .format(marketid=marketid, type_list=stktype_list)
    cur = connect.cursor()
    cur.execute(sql)
    a = [v for v in cur]
    cur.close()
    return sorted(a, key=lambda k: len(k[0]), reverse=True)


def get_stock_list(connect, market, quotations):
    stktype_list = get_stktype_list(quotations)
    sql = "select cast(stockid as bigint), market, code, name, type, valid, startDate, endDate from hku_base.n_stock where market='{}' and type in {}"\
        .format(market.upper(), stktype_list)
    cur = connect.cursor()
    cur.execute(sql)
    a = [v for v in cur]
    cur.close()
    return a


def import_new_holidays(connect):
    """导入新的交易所休假日历"""
    cur = connect.cursor()
    cur.execute("select date from `hku_base`.`z_zh_holiday` order by date desc limit 1")
    a = [v for v in cur]
    last_date = a[0][0] if a else 19901219
    holidays = get_new_holidays()
    new_holidays = [(int(v), ) for v in holidays if int(v) > last_date]
    now_id = int(datetime.datetime.now().timestamp()*1000)
    for i, v in enumerate(new_holidays):
        cur.execute(f"insert into `hku_base`.`z_zh_holiday` (id, date) values ({now_id+i}, {v[0]})")
    cur.close()


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
    return f"{ktype_dict[ktype.lower()]}_data.{market}{code}".lower()


def get_lastdatetime(connect, tablename):
    # print(tablename)
    try:
        tmp = connect.query("select LAST(date) from {}".format(tablename))
        a = tmp.fetch_all()
        return Datetime(a[0][0]) if a and len(a[0]) > 0 else None
    except:
        return None


def get_last_krecord(connect, tablename):
    """获取最后一条K线记录
    返回：(date, open, close, high, low, amount, volume)
    """
    try:
        tmp = connect.query("select LAST_ROW(*) from {}".format(tablename))
        a = tmp.fetch_all()
        if not a:
            return None
        a = a[0]
        return (Datetime(a[0]), a[1], a[2], a[3], a[4], a[5], a[6])
    except:
        return None


def update_extern_data(connect, market, code, data_type):
    """更新周线、月线、15分钟线等扩展数据索引"""
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

    if data_type.lower() == 'day':
        # index_list = ('week', 'month', 'year')
        index_list = ('week', 'month', 'quarter', 'halfyear', 'year')
        base_table = get_table(connect, market, code, 'day')
    else:
        index_list = ('min15', 'min30', 'min60')  # , 'hour2')
        # index_list = ('min15', )
        base_table = get_table(connect, market, code, 'min5')

    base_lastdate = get_lastdatetime(connect, base_table)
    if base_lastdate is None:
        return

    for index_type in index_list:
        # hku_debug("{}{} update {} index".format(market, code, index_type))
        index_table = get_table(connect, market, code, index_type)
        index_last_date = get_lastdatetime(connect, index_table)

        # 获取当前日期大于等于索引表最大日期的基础表日期列表
        cur = connect.cursor()
        if index_last_date is None:
            cur.execute(
                'select date, open, high, low, close, amount, volume from {} order by date asc '.format(base_table)
            )
        else:
            start_date, _ = getNewDate(index_type, index_last_date.ymdhm)
            cur.execute(
                'select date, open, high, low, close, amount, volume from {} where date>={}'.format(
                    base_table, (Datetime(start_date) - UTCOffset()).timestamp()
                )
            )
        base_list = [x for x in cur]
        cur.close()

        last_start_date = 199012010000
        last_end_date = 199012010000

        insert_buffer = []
        length_base_all = len(base_list)
        for x in range(length_base_all):
            current_date = Datetime(base_list[x][0]).ymdhm
            if current_date <= last_end_date:
                continue
            last_start_date, last_end_date = getNewDate(index_type, current_date)

            base_record_list = []
            start_ix = x
            ix_date = current_date
            while start_ix < length_base_all and \
                    ix_date >= last_start_date and ix_date <= last_end_date:
                base_record_list.append(base_list[start_ix])
                ix_date = Datetime(base_list[start_ix][0]).ymdhm
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
            insert_buffer.append((last_end_date, open_price, high_price, low_price, close_price, amount, count))

        if insert_buffer:
            # hku_info(f"update index: {market.lower()}{code}")
            rawsql = f"insert into {index_table} using {index_type}_data.kdata TAGS('{market}', '{code}') VALUES "
            sql = rawsql
            for i, v in enumerate(insert_buffer):
                sql += f"({(Datetime(v[0])-UTCOffset()).timestamp()}, {v[1]}, {v[2]}, {v[3]}, {v[4]}, {v[5]}, {v[6]})"
                if i > 0 and i % 8000 == 0:
                    connect.execute(sql)
                    sql = rawsql
            if sql != rawsql:
                connect.execute(sql)


if __name__ == '__main__':
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    db = 'taos54'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    connect = get_taos().connect(
        user=user, password=password, host=host, port=port)

    # sql = "select LAST_ROW(date) from bj_data.day_430017"
    # result = connect.query(sql)
    # data = result.fetch_all()
    # from hikyuu import Datetime
    # for row in data:
    #     print(Datetime(row[0]))

    connect.execute("drop database if exists hku_base")

    # import pandas as pd
    # df = pd.read_sql("SELECT id FROM hku_data.sh_day_000001", connect)
    # df["ts_utc"] = df["id"].dt.tz_localize(None)  # 移除时区信息
    # df["ts_str"] = df["ts_utc"].dt.strftime("%Y-%m-%d %H:%M:%S")
    # print(df)

    create_database(connect)
    print(is_exist_db(connect))

    # import_new_holidays(connect)

    x = get_db_version(connect)
    print(x)

    # get_table(connect, "SH", "000001", "DAY")

    marketid = get_marketid(connect, "SH")
    print(marketid)

    x = get_stock_list(connect, "SH", ["stock"])
    print(x)

    d = get_lastdatetime(connect, "day_data.sh000001")
    print(d)

    d = get_last_krecord(connect, "day_data.sh000001")
    print(d)

    connect.close()
