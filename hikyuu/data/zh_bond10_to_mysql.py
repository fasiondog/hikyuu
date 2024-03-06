#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2024-03-01
#    Author: fasiondog

import datetime
from hikyuu.data.common import get_china_bond10_rate
from hikyuu.util import *


@hku_catch(trace=True)
def import_zh_bond10_to_mysql(connect):
    sql = "select max(date) from hku_base.zh_bond10"
    cur = connect.cursor()
    cur.execute(sql)
    a = cur.fetchall()
    if a[0][0] is None:
        start_date = "19901219"
    else:
        last_date = a[0][0]
        year = last_date // 10000
        month = last_date // 100 - year * 100
        day = last_date - year * 10000 - month * 100
        start_date = (datetime.date(year, month, day) + datetime.timedelta(1)).strftime('%Y%m%d')
    rates = get_china_bond10_rate(start_date)
    if rates:
        cur.executemany("INSERT INTO hku_base.zh_bond10(date, value) VALUES (%s, %s)", rates)
        connect.commit()
        cur.close()
