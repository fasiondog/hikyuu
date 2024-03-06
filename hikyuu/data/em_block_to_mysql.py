#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

import json
import akshare  # 这里必须再导入 akshare 下，否则get_all_zsbk_info会报错
from hikyuu.data.common import MARKET, get_stk_code_name_list
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *


def em_import_block_to_mysql(connect, code_market_dict, categorys=('行业板块', '概念板块', '地域板块', '指数板块')):
    all_block_info = {}

    if '行业板块' in categorys:
        hku_info("获取行业板块信息")
        x = get_all_hybk_info(code_market_dict)
        if x:
            all_block_info["行业板块"] = x

    if '行业板块' in categorys:
        hku_info("获取概念板块信息")
        x = get_all_gnbk_info(code_market_dict)
        if x:
            all_block_info["概念板块"] = x

    if '行业板块' in categorys:
        hku_info("获取地域板块信息")
        x = get_all_dybk_info(code_market_dict)
        if x:
            all_block_info["地域板块"] = x

    if '行业板块' in categorys:
        hku_info("获取指数板块信息")
        x = get_all_zsbk_info(code_market_dict)
        if x:
            all_block_info["指数板块"] = x

    hku_info("更新数据库")
    cur = connect.cursor()
    sql = "delete from hku_base.block where category in ('行业板块', '概念板块', '地域板块', '指数板块')"
    cur.execute(sql)

    insert_records = []

    for category in all_block_info:
        for name in all_block_info[category]:
            for code in all_block_info[category][name]:
                insert_records.append((category, name, code))

    if insert_records:
        sql = "insert into hku_base.block (category, name, market_code) values (%s,%s,%s)"
        cur.executemany(sql, insert_records)

    connect.commit()
    cur.close()
    pass


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

    x = get_stk_code_name_list(MARKET.SH)
    code_market_dict = {}
    for v in x:
        code_market_dict[v["code"]] = MARKET.SH
    # print(code_market_dict)

    em_import_block_to_mysql(connect, code_market_dict)
    connect.close()
