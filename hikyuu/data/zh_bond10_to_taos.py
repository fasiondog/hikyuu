#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2024-03-01
#    Author: fasiondog

import taos
import datetime
from hikyuu import Datetime, Days, UTCOffset
from hikyuu.data.common import get_china_bond10_rate
from hikyuu.util import *


@hku_catch(trace=True)
def import_zh_bond10_to_taos(connect):
    sql = "select LAST_ROW(date) from hku_base.z_zh_bond10"
    cur = connect.cursor()
    cur.execute(sql)
    a = [v for v in cur]
    if not a:
        start_date = "19901219"
    else:
        last_date = Datetime(a[0][0])
        start_date = str((last_date + Days(1)).ymd)
    rates = get_china_bond10_rate(start_date)
    if rates:
        raw_sql = "INSERT INTO hku_base.z_zh_bond10(date, value) VALUES "
        sql = raw_sql
        for i, rate in enumerate(rates):
            sql += f",({(Datetime(rate[0])-UTCOffset()).timestamp()}, {rate[1]})"
            if i > 1 and i % 18000 == 0:
                cur.execute(sql)
                sql = raw_sql
        if sql != raw_sql:
            cur.execute(sql)
    cur.close()


if __name__ == '__main__':
    import os
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    db = 'taos54'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    connect = taos.connect(
        user=user, password=password, host=host, port=port)

    import_zh_bond10_to_taos(connect)
    # x = get_china_bond10_rate("19901219")
    # print(len(x))

    connect.close()
