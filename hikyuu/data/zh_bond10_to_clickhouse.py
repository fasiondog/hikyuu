#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2024-03-01
#    Author: fasiondog

import datetime
from hikyuu.data.common import get_china_bond10_rate
from hikyuu.util import *


@hku_catch(trace=True)
def import_zh_bond10_to_clickhouse(connect):
    sql = "select max(date) from hku_base.zh_bond10"
    start_date = connect.command(sql)
    if type(start_date) == int:
        start_date = "19901219"
    else:
        last_date = start_date
        year = last_date // 10000
        month = last_date // 100 - year * 100
        day = last_date - year * 10000 - month * 100
        start_date = (datetime.date(year, month, day) + datetime.timedelta(1)).strftime('%Y%m%d')
    rates = get_china_bond10_rate(start_date)
    if rates:
        ic = connect.create_insert_context(table='zh_bond10', database='hku_base',
                                           column_names=['date', 'value'],
                                           data=rates)
        connect.insert(context=ic)


if __name__ == "__main__":
    import os
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    db = 'clickhouse54-http'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    import clickhouse_connect
    client = clickhouse_connect.get_client(
        host=host, username=user, password=password)

    import_zh_bond10_to_clickhouse(client)

    client.close()
