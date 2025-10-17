#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *
from hikyuu.data.download_block import *


def em_import_block_to_clickhouse(connect):
    download_block_info()

    blocks_info = read_block_from_path()
    blks = blocks_info['block']
    blks_info = blocks_info['block_info']

    if not blks:
        return 0

    hku_info("更新数据库")
    sql = f"delete from hku_base.block where category in {tuple(blks.keys())}"
    connect.command(sql)
    sql = f"delete from hku_base.blockindex where category in {tuple(blks.keys())}"
    connect.command(sql)

    insert_records = []

    for category, blocks in blks.items():
        for name, codes in blocks.items():
            for code in codes:
                insert_records.append((category, name, code))

    if insert_records:
        ic = connect.create_insert_context(table='block', database='hku_base',
                                           column_names=['category', 'name', 'market_code'],
                                           data=insert_records)
        connect.insert(context=ic)

    index_records = []
    for category, blocks in blks_info.items():
        for name, index_code in blocks.items():
            if len(index_code) == 8:
                index_records.append((category, name, index_code))
    if index_records:
        ic = connect.create_insert_context(table='blockindex', database='hku_base',
                                           column_names=['category', 'name', 'index_code'],
                                           data=index_records)
        connect.insert(context=ic)

    return len(insert_records)


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

    em_import_block_to_clickhouse(client)

    client.close()
