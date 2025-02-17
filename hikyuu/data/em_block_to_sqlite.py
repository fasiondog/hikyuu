#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

from concurrent.futures import ThreadPoolExecutor
from hikyuu.data.common import MARKET, get_stk_code_name_list
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *


@spend_time
def em_import_block_to_sqlite(connect, code_market_dict, categorys=('行业板块', '概念板块', '地域板块', '指数板块')):
    all_block_info = {}

    with ThreadPoolExecutor(4) as executor:
        if '行业板块' in categorys:
            t1 = executor.submit(get_all_hybk_info, code_market_dict)

        if '概念板块' in categorys:
            t2 = executor.submit(get_all_gnbk_info, code_market_dict)

        if '地域板块' in categorys:
            t3 = executor.submit(get_all_dybk_info, code_market_dict)

        if '指数板块' in categorys:
            t4 = executor.submit(get_all_zsbk_info, code_market_dict)

        success_fetch_hy = False
        if '行业板块' in categorys:
            x = t1.result()
            hku_info("获取行业板块信息完毕")
            if x:
                all_block_info["行业板块"] = x
                success_fetch_hy = True

        success_fetch_gn = False
        if '概念板块' in categorys:
            x = t2.result()
            hku_info("获取概念板块信息完毕")
            if x:
                all_block_info["概念板块"] = x
                success_fetch_gn = True

        success_fetch_dy = False
        if '地域板块' in categorys:
            x = t3.result()
            hku_info("获取地域板块信息完毕")
            if x:
                all_block_info["地域板块"] = x
                success_fetch_dy = True

        success_fetch_zs = False
        if '指数板块' in categorys:
            x = t4.result()
            hku_info("获取指数板块信息完毕")
            if x:
                all_block_info["指数板块"] = x
                success_fetch_zs = True

    blks = []
    if success_fetch_hy:
        blks.append('行业板块')
    if success_fetch_gn:
        blks.append('概念板块')
    if success_fetch_dy:
        blks.append('地域板块')
    if success_fetch_zs:
        blks.append('指数板块')

    print(blks)

    if not blks:
        return

    hku_info("更新数据库")
    cur = connect.cursor()
    if len(blks) == 1:
        sql = f"delete from block where category in ('{blks[0]}')"
    else:
        sql = f"delete from block where category in {tuple(blks)}"
    hku_info(sql)
    cur.execute(sql)

    insert_records = []

    for category in all_block_info:
        for name in all_block_info[category]:
            for code in all_block_info[category][name]:
                insert_records.append((category, name, code))

    if insert_records:
        sql = "insert into block (category, name, market_code) values (?,?,?)"
        hku_info(f"insert block records: {len(insert_records)}")
        cur.executemany(sql, insert_records)

    connect.commit()
    cur.close()
    return len(insert_records)


if __name__ == "__main__":
    import sqlite3
    from hikyuu.data.common_sqlite3 import create_database

    # dest_dir = "/home/fasiondog/stock"
    dest_dir = "/Users/fasiondog/stock"
    # dest_dir = "d:\\stock"

    connect = sqlite3.connect(dest_dir + "/stock.db")
    create_database(connect)

    x = get_stk_code_name_list(MARKET.SH)
    code_market_dict = {}
    for v in x:
        code_market_dict[v["code"]] = MARKET.SH
    # print(code_market_dict)

    em_import_block_to_sqlite(connect, code_market_dict, categorys=('地域板块',))

    connect.close()
