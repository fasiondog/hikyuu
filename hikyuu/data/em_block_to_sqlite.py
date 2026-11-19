#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Create on: 20240102
#    Author: fasiondog


from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *
from hikyuu.data import download_block
import asyncio
from pytdxdata import TdxData

async def pytdxdata_block_download() -> None:
    # TdxData 为异步上下文管理器：进入时建连，退出时释放
    async with TdxData() as td: 
        try:
            await download_block.down_tdx_hybk_info(td)
        except Exception as e:
            hku_warn(f"执行 行业导入 失败 {type(e).__name__}: {str(e)}")
        
        try:
            await download_block.down_tdx_gnbk_info(td)
        except Exception as e:
            hku_warn(f"执行 概念导入 失败 {type(e).__name__}: {str(e)}")

@spend_time
def em_import_block_to_sqlite(connect):
    download_block.download_block_info()
    asyncio.run(pytdxdata_block_download())

    blocks_info = download_block.read_block_from_path()
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
        placeholders = '?'
        params = categories
    else:
        placeholders = ','.join(['?'] * len(categories))  # 创建相应数量的占位符
        params = categories
    
    sql = f"delete from block where category in ({placeholders})"
    cur.execute(sql, params)
    
    sql = f"delete from BlockIndex where category in ({placeholders})"
    cur.execute(sql, params)

    insert_records = []

    for category, blocks in blks.items():
        for name, codes in blocks.items():
            for code in codes:
                insert_records.append((category, name, code))

    if insert_records:
        sql = "insert into block (category, name, market_code) values (?,?,?)"
        cur.executemany(sql, insert_records)

    index_records = []
    for category, blocks in blks_info.items():
        for name, index_code in blocks.items():
            if len(index_code) == 8:
                index_records.append((category, name, index_code))
    if index_records:
        sql = "insert into BlockIndex (category, name, market_code) values (?,?,?)"
        cur.executemany(sql, index_records)
        hku_info(f"insert block index records: {len(index_records)}")

    connect.commit()
    cur.close()
    return len(insert_records)


if __name__ == "__main__":
    import sqlite3

    # dest_dir = "/home/fasiondog/stock"
    # dest_dir = "/Users/fasiondog/stock"
    dest_dir = "/Users/Zhuanz1/data/hikyuu_stock"

    connect = sqlite3.connect(dest_dir + "/stock.db")
    # create_database(connect)

    em_import_block_to_sqlite(connect)

    connect.close()
