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

import mysql.connector

from hikyuu.data.common import get_stktype_list, get_new_holidays
from hikyuu.util import hku_debug, hku_info


def get_mysql_connect_version():
    m, n, _ = mysql.connector.__version__.split('.')
    return int(m) + float(n) * 0.1


def is_exist_db(connect):
    """数据库是否已存在"""
    cur = connect.cursor()
    cur.execute("SELECT 1 FROM information_schema.SCHEMATA where SCHEMA_NAME='hku_base'")
    a = cur.fetchone()
    cur.close()
    return True if a else False


def get_db_version(connect):
    try:
        cur = connect.cursor()
        cur.execute("select `version` from `hku_base`.`version`")
        a = cur.fetchone()
        cur.close()
        return a[0] if a else 0
    except:
        return 0


def create_database(connect):
    """创建数据库"""
    sql_dir = os.path.dirname(__file__) + "/mysql_upgrade"
    cur = connect.cursor()
    mysql_version = get_mysql_connect_version()
    if not is_exist_db(connect):
        filename = sql_dir + "/createdb.sql"
        with open(filename, 'r', encoding='utf8') as f:
            sql = f.read()
        if mysql_version >= 9.2:
            cur.execute(sql)
            _ = cur.fetchall()
            while cur.nextset():
                _ = cur.fetchall()
        else:
            for x in cur.execute(sql, multi=True):
                pass

    db_version = get_db_version(connect)
    files = [x for x in Path(sql_dir).iterdir()
             if x.is_file()
             and x.name != 'createdb.sql'
             and x.name != '__init__.py'
             and int(x.stem) > db_version and not x.is_dir()]
    files.sort()
    for file in files:
        sql = file.read_text(encoding='utf8')
        if mysql_version >= 9.2:
            cur.execute(sql, map_results=False)
            _ = cur.fetchall()
            while cur.nextset():
                _ = cur.fetchall()
        else:
            for x in cur.execute(sql, multi=True):
                # print(x.statement)
                pass

    connect.commit()
    cur.close()


def get_marketid(connect, market):
    cur = connect.cursor()
    cur.execute("select marketid, market from `hku_base`.`market` where market='{}'".format(market.upper()))
    marketid = cur.fetchone()
    marketid = marketid[0]
    cur.close()
    return marketid


def get_codepre_list(connect, marketid, quotations):
    """获取前缀代码表"""
    stktype_list = get_stktype_list(quotations)
    sql = "select codepre, type from `hku_base`.`coderuletype` " \
          "where marketid={marketid} and type in {type_list} ORDER by length(codepre) DESC"\
        .format(marketid=marketid, type_list=stktype_list)
    cur = connect.cursor()
    cur.execute(sql)
    a = cur.fetchall()
    cur.close()
    return sorted(a, key=lambda k: len(k[0]), reverse=True)


def get_stock_list(connect, market, quotations):
    marketid = get_marketid(connect, market)
    stktype_list = get_stktype_list(quotations)
    sql = "select stockid, marketid, code, valid, type from `hku_base`.`stock` where marketid={} and type in {}"\
        .format(marketid, stktype_list)
    cur = connect.cursor()
    cur.execute(sql)
    a = cur.fetchall()
    connect.commit()
    cur.close()
    return a


def import_new_holidays(connect):
    """导入新的交易所休假日历"""
    cur = connect.cursor()
    cur.execute("select date from `hku_base`.`holiday` order by date desc limit 1")
    a = cur.fetchall()
    last_date = a[0][0] if a else 19901219
    holidays = get_new_holidays()
    new_holidays = [(int(v), ) for v in holidays if int(v) > last_date]
    if new_holidays:
        cur.executemany("insert into `hku_base`.`holiday` (date) values (%s)", new_holidays)
        connect.commit()
        cur.close()


def get_table(connect, market, code, ktype):
    cur = connect.cursor()
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
    }
    schema = "{market}_{ktype}".format(market=market, ktype=ktype_dict[ktype.lower()]).lower()
    cur.execute("SELECT 1 FROM information_schema.SCHEMATA where SCHEMA_NAME='{}'".format(schema))
    a = cur.fetchone()
    if not a:
        cur.execute("CREATE SCHEMA `{}`".format(schema))
        connect.commit()

    tablename = code.lower()
    cur.execute(
        "SELECT 1 FROM information_schema.tables "
        "where table_schema='{schema}' and table_name='{name}'".format(schema=schema, name=tablename)
    )
    a = cur.fetchone()
    if not a:
        sql = """
                CREATE TABLE `{schema}`.`{name}` (
                    `date` BIGINT(20) UNSIGNED NOT NULL,
                    `open` DOUBLE UNSIGNED NOT NULL,
                    `high` DOUBLE UNSIGNED NOT NULL,
                    `low` DOUBLE UNSIGNED NOT NULL,
                    `close` DOUBLE UNSIGNED NOT NULL,
                    `amount` DOUBLE UNSIGNED NOT NULL,
                    `count` DOUBLE UNSIGNED NOT NULL,
                    PRIMARY KEY (`date`)
                )
                COLLATE='utf8mb4_general_ci'
                ENGINE=MyISAM
                ;
              """.format(schema=schema, name=tablename)
        cur.execute(sql)
        connect.commit()

    cur.close()
    return "`{schema}`.`{name}`".format(schema=schema, name=tablename)


def get_lastdatetime(connect, tablename):
    cur = connect.cursor()
    cur.execute("select max(date) from {}".format(tablename))
    a = cur.fetchone()
    return a[0]


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
        index_list = ('min15', 'min30', 'min60', 'hour2')
        # index_list = ('min15', )
        base_table = get_table(connect, market, code, 'min5')

    base_lastdate = get_lastdatetime(connect, base_table)
    if base_lastdate is None:
        return

    for index_type in index_list:
        hku_debug("{}{} update {} index".format(market, code, index_type))
        index_table = get_table(connect, market, code, index_type)
        index_last_date = get_lastdatetime(connect, index_table)

        # 获取当前日期大于等于索引表最大日期的基础表日期列表
        cur = connect.cursor()
        if index_last_date is None:
            cur.execute(
                'select date, open, high, low, close, amount, count from {} order by date asc '.format(base_table)
            )
        else:
            start_date, _ = getNewDate(index_type, index_last_date)
            cur.execute(
                'select date, open, high, low, close, amount, count from {} where date>={}'.format(
                    base_table, start_date
                )
            )
        base_list = [x for x in cur]
        cur.close()

        last_start_date = 199012010000
        last_end_date = 199012010000

        update_buffer = []
        insert_buffer = []
        # for current_base in base_list:
        length_base_all = len(base_list)
        for x in range(length_base_all):
            current_date = base_list[x][0]
            if current_date <= last_end_date:
                continue
            last_start_date, last_end_date = getNewDate(index_type, current_date)

            # cur = connect.cursor()
            # cur.execute(
            #    'select date, open, high, low, close, amount, count from {} \
            #    where date>={} and date<={} order by date asc'.format(
            #        base_table, last_start_date, last_end_date
            #    )
            # )
            # base_record_list = [r for r in cur]
            # cur.close()
            base_record_list = []
            start_ix = x
            ix_date = current_date
            while start_ix < length_base_all and \
                    ix_date >= last_start_date and ix_date <= last_end_date:
                base_record_list.append(base_list[start_ix])
                ix_date = base_list[start_ix][0]
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
            if last_end_date == index_last_date:
                update_buffer.append((open_price, high_price, low_price, close_price, amount, count, last_end_date))
            else:
                insert_buffer.append((last_end_date, open_price, high_price, low_price, close_price, amount, count))

        if update_buffer:
            cur = connect.cursor()
            cur.executemany(
                "update {} set open=%s, high=%s, low=%s, close=%s, amount=%s, count=%s \
                 where date=%s".format(index_table), update_buffer
            )
            connect.commit()
            cur.close()
        if insert_buffer:
            cur = connect.cursor()
            cur.executemany(
                "insert into {} (date, open, high, low, close, amount, count) \
                 values (%s, %s, %s, %s, %s, %s, %s)".format(index_table), insert_buffer
            )
            connect.commit()
            cur.close()


if __name__ == '__main__':
    host = '127.0.0.1'
    port = 3306
    usr = 'root'
    pwd = ''

    cnx = mysql.connector.connect(user=usr, password=pwd, host=host, port=port)

    update_extern_data(cnx, 'SH', '000001', 'day')
    cnx.close()
