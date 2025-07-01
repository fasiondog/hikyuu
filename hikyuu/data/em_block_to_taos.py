#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

import zlib
from concurrent.futures import ThreadPoolExecutor
from hikyuu import Datetime, UTCOffset
from hikyuu.data.common import MARKET, get_stk_code_name_list
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *


@spend_time
def em_import_block_to_taos(connect, code_market_dict, categorys=('行业板块', '概念板块', '地域板块', '指数板块')):
    all_block_info = {}
    if not categorys:
        return

    if '行业板块' in categorys:
        all_block_info["行业板块"] = get_all_hybk_info(code_market_dict)

    if '概念板块' in categorys:
        all_block_info["概念板块"] = get_all_gnbk_info(code_market_dict)

    if '地域板块' in categorys:
        all_block_info["地域板块"] = get_all_dybk_info(code_market_dict)

    if '指数板块' in categorys:
        all_block_info["指数板块"] = get_all_zsbk_info(code_market_dict)

    hku_info("更新数据库")

    cur = connect.cursor()
    insert_records = 0
    for category in all_block_info:
        for name in all_block_info[category]:
            x = f"{category}_{name}"
            hash_value = zlib.crc32(x.encode("utf-8"))
            tbname = f"z_blk_{hash_value}"
            cur.execute(
                f"create table if not exists hku_base.{tbname} using hku_base.s_block (category, name) TAGS ('{category}', '{name}')")
            cur.execute(f"delete from hku_base.{tbname}")
            codes = all_block_info[category][name]
            # print(hash_value)
            id = (Datetime.from_timestamp(hash_value*10000)-UTCOffset()).timestamp()
            rawsql = f"insert into hku_base.{tbname} (id, market_code) values "
            sql = rawsql
            for i, code in enumerate(codes):
                sql += f"({id+i}, '{code}')"
                if i > 0 and i % 3000 == 0:
                    cur.execute(sql)
                    sql = rawsql
            if sql != rawsql:
                cur.execute(sql)
            insert_records += len(codes)

    cur.close()
    return insert_records


if __name__ == "__main__":
    import os
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    db = 'taos54'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    tdx_server = '180.101.48.170'
    tdx_port = 7709

    from hikyuu.data.common_taos import get_taos
    connect = get_taos().connect(
        user=user, password=password, host=host, port=port)

    import time
    starttime = time.time()

    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect(tdx_server, tdx_port)

    x = get_stk_code_name_list(MARKET.SH)
    code_market_dict = {}
    for v in x:
        code_market_dict[v["code"]] = MARKET.SH

    print('数量:', em_import_block_to_taos(connect, code_market_dict, categorys=('行业板块', '指数板块')))
    # print('数量:', em_import_block_to_taos(connect, code_market_dict, categorys=('概念板块','地域板块', )))

    api.disconnect()
    connect.close()

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
