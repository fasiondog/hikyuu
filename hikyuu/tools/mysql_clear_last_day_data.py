#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2023-12-06
#    Author: fasiondog

#
# 用于清除 mysql 最后一日的数据。
# 防止在开市时间段内，不小心导入了数据，导致当天数据错误。
#

import datetime
import mysql.connector
from hikyuu.util import *


@hku_catch(trace=True)
def get_all_tables(conn, market, typ):
    cur = conn.cursor()
    sql = f"SELECT table_schema, table_name FROM information_schema.tables WHERE table_schema='{market}_{typ}'"
    cur.execute(sql)
    x = cur.fetchall()
    cur.close()
    return [f'`{v[0]}`.`{v[1]}`' for v in x]


def clear_today_day_data(conn, market, typ):
    tables = get_all_tables(conn, market, typ)
    today = datetime.date.today()

    # yyyy mm dd hh mm
    t = today.year * 100000000 + today.month * 1000000 + today.day * 10000
    if typ == 'trans':
        # yyyy mm dd hh mm ss
        t = t * 100
    cur = conn.cursor()
    for table in tables:
        hku_info(f"clear {table}")
        sql = f"delete from {table} where date>={t}"
        cur.execute(sql)
    cur.close()
    conn.commit()


if __name__ == '__main__':
    host = '127.0.0.1'
    port = 3306
    usr = 'root'
    pwd = ''

    conn = mysql.connector.connect(user=usr, password=pwd, host=host, port=port)

    x = get_all_tables(conn, 'bj', 'day')
    market_list = ['sh', 'sz', 'bj']
    typ_list = ['time', 'trans', 'day', 'week', 'month', 'quarter',
                'halfyear', 'year', 'min', 'min5', 'min15', 'min30', 'min60']
    for market in market_list:
        for typ in typ_list:
            clear_today_day_data(conn, market, typ)

    conn.close()
