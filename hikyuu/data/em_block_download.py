#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-10-05
#    Author: fasiondog

from concurrent.futures import ThreadPoolExecutor
from hikyuu.data.common import MARKET, get_stk_code_name_list
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *
import csv
import os
from datetime import datetime


def is_file_created_today(filepath):
    """判断文件是否是今天创建的"""
    if not os.path.exists(filepath):
        return False

    file_mtime = os.path.getmtime(filepath)
    file_date = datetime.fromtimestamp(file_mtime).date()
    today = datetime.today().date()

    return file_date == today


def save_block_to_csv(block_data, block_category, save_path):
    """将板块数据保存为CSV文件，格式为板块名称/code"""
    filename = f"{save_path}/{block_category}.csv"

    with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['block_name', 'code'])
        for name in block_data:
            for code in block_data[name]:
                writer.writerow([name, code])
    hku_info(f"已保存{block_category}至 {filename}")


@spend_time
def download_em_block(code_market_dict, save_path='.', categorys=('行业板块', '概念板块', '地域板块', '指数板块')):
    all_block_info = {}

    new_categorys = []
    for catetory in categorys:
        if not is_file_created_today(f"{save_path}/{catetory}.csv"):
            new_categorys.append(catetory)

    with ThreadPoolExecutor(4) as executor:
        if '行业板块' in new_categorys:
            t1 = executor.submit(get_all_hybk_info, code_market_dict)

        if '概念板块' in new_categorys:
            t2 = executor.submit(get_all_gnbk_info, code_market_dict)

        if '地域板块' in new_categorys:
            t3 = executor.submit(get_all_dybk_info, code_market_dict)

        if '指数板块' in new_categorys:
            t4 = executor.submit(get_all_zsbk_info, code_market_dict)

        if '行业板块' in new_categorys:
            x = t1.result()
            hku_info("获取行业板块信息完毕")
            if x:
                all_block_info["行业板块"] = x
                save_block_to_csv(x, "行业板块", save_path)

        if '概念板块' in new_categorys:
            x = t2.result()
            hku_info("获取概念板块信息完毕")
            if x:
                all_block_info["概念板块"] = x
                save_block_to_csv(x, "概念板块", save_path)

        if '地域板块' in new_categorys:
            x = t3.result()
            hku_info("获取地域板块信息完毕")
            if x:
                all_block_info["地域板块"] = x
                save_block_to_csv(x, "地域板块", save_path)

        if '指数板块' in new_categorys:
            x = t4.result()
            hku_info("获取指数板块信息完毕")
            if x:
                all_block_info["指数板块"] = x
                save_block_to_csv(x, "指数板块", save_path)


if __name__ == "__main__":
    from hikyuu.interactive import *
    code_market_dict = {}
    for s in sm:
        if s.valid and s.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ):
            code_market_dict[s.code] = s.market

    # print(code_market_dict)

    download_em_block(code_market_dict, categorys=('行业板块', ))
