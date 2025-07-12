#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

from concurrent.futures import ThreadPoolExecutor
from hikyuu.data.common import MARKET, get_stk_code_name_list
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *


def em_import_block_to_clickhouse(connect, code_market_dict, categorys=('行业板块', '指数板块')):
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

    if not blks:
        return

    hku_info("更新数据库")
    if len(blks) == 1:
        sql = f"delete from hku_base.block where category in ('{blks[0]}')"
    else:
        sql = f"delete from hku_base.block where category in {tuple(blks)}"
    connect.command(sql)

    insert_records = []

    for category in all_block_info:
        for name in all_block_info[category]:
            for code in all_block_info[category][name]:
                insert_records.append((category, name, code))

    if insert_records:
        ic = connect.create_insert_context(table='block', database='hku_base',
                                           column_names=['category', 'name', 'market_code'],
                                           data=insert_records)
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

    x = get_stk_code_name_list(MARKET.SH)
    code_market_dict = {}
    for v in x:
        code_market_dict[v["code"]] = MARKET.SH
    # print(code_market_dict)

    em_import_block_to_clickhouse(client, code_market_dict, categorys=('行业板块', ))

    client.close()
