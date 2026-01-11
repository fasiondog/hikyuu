#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog


from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *
from hikyuu.data.download_block import *


def em_import_block_to_mysql(connect):
    download_block_info()

    blocks_info = read_block_from_path()
    blks = blocks_info['block']
    blks_info = blocks_info['block_info']

    if not blks:
        return 0

    hku_info("更新数据库")
    cur = connect.cursor()
    
    # 构建参数化的DELETE语句以防止SQL注入
    categories = tuple(blks.keys())
    if len(categories) == 1:
        # 处理只有一个类别的特殊情况
        placeholders = '%s'
        params = categories
    else:
        placeholders = ','.join(['%s'] * len(categories))  # 创建相应数量的占位符
        params = categories
    
    sql = f"delete from hku_base.block where category in ({placeholders})"
    cur.execute(sql, params)
    
    sql = f"delete from hku_base.BlockIndex where category in ({placeholders})"
    cur.execute(sql, params)

    insert_records = []

    for category, blocks in blks.items():
        for name, codes in blocks.items():
            for code in codes:
                insert_records.append((category, name, code))

    if insert_records:
        sql = "insert into hku_base.block (category, name, market_code) values (%s,%s,%s)"
        cur.executemany(sql, insert_records)

    index_records = []
    for category, blocks in blks_info.items():
        for name, index_code in blocks.items():
            if len(index_code) == 8:
                index_records.append((category, name, index_code))
    if index_records:
        sql = "insert into hku_base.BlockIndex (category, name, market_code) values (%s,%s,%s)"
        cur.executemany(sql, index_records)

    connect.commit()
    cur.close()
    return len(insert_records)


if __name__ == "__main__":
    import mysql
    from hikyuu.data.common_mysql import create_database

    host = '127.0.0.1'
    port = 3306
    usr = 'root'
    pwd = ''

    src_dir = "D:\\TdxW_HuaTai"
    quotations = ['stock', 'fund']  # 通达信盘后数据没有债券

    connect = mysql.connector.connect(user=usr, password=pwd, host=host, port=port)
    create_database(connect)

    em_import_block_to_mysql(connect)
    connect.close()
