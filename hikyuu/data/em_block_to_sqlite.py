#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

import json
import akshare  # 这里必须再导入 akshare 下，否则get_all_zsbk_info会报错
from hikyuu.data.common import MARKET, STOCKTYPE, get_stk_code_name_list
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *


def em_import_block_to_sqlite(connect, code_market_dict):
    all_block_info = {}
    hku_info("获取行业板块信息")

    x = get_all_hybk_info(code_market_dict)
    if x:
        all_block_info["行业板块"] = x

    hku_info("获取概念板块信息")
    x = get_all_gnbk_info(code_market_dict)
    if x:
        all_block_info["概念板块"] = x

    hku_info("获取地域板块信息")
    x = get_all_dybk_info(code_market_dict)
    if x:
        all_block_info["地域板块"] = x

    hku_info("获取指数板块信息")
    x = get_all_zsbk_info(code_market_dict)
    if x:
        all_block_info["指数板块"] = x

    hku_info("更新数据库")
    cur = connect.cursor()
    sql = "select category from block"
    all_block_in_db = cur.execute(sql).fetchall()
    all_block_in_db = [v[0] for v in all_block_in_db]

    update_records = []
    insert_records = []

    if all_block_in_db:
        for category in all_block_info:
            if category in all_block_in_db:
                update_records.append((json.dumps(all_block_info[category], ensure_ascii=False), category))
            else:
                insert_records.append((category, json.dumps(all_block_info[category], ensure_ascii=False)))
    else:
        for category in all_block_info:
            insert_records.append((category, json.dumps(all_block_info[category], ensure_ascii=False)))

    if update_records:
        sql = "update block set content=? where category=?"
        cur.executemany(sql, update_records)
    if insert_records:
        sql = "insert into block (category, content) values (?,?)"
        cur.executemany(sql, insert_records)

    connect.commit()
    cur.close()
    pass


if __name__ == "__main__":
    import sqlite3
    from hikyuu.data.common_sqlite3 import create_database

    # dest_dir = "/home/fasiondog/stock"
    dest_dir = "d:\\stock"

    connect = sqlite3.connect(dest_dir + "/stock.db")
    create_database(connect)

    x = get_stk_code_name_list(MARKET.SH)
    code_market_dict = {}
    for v in x:
        code_market_dict[v["code"]] = MARKET.SH
    # print(code_market_dict)

    em_import_block_to_sqlite(connect, code_market_dict)
