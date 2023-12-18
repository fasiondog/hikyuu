#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2023-12-18
#    Author: fasiondog

import akshare as ak
from hikyuu.interactive import *


def get_code_market_dict():
    ret = {}
    for s in sm:
        if s.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_B, constant.STOCKTYPE_GEM, constant.STOCKTYPE_START):
            ret[s.code] = "0" if s.market == 'SH' else "1"
    return ret


def get_all_hybk_info():
    code_market = get_code_market_dict()
    blk_names = ak.stock_board_industry_name_em()['板块名称']
    ret = {}
    for blk_name in blk_names:
        print(blk_name)
        ret[blk_name] = []
        stk_codes = ak.stock_board_industry_cons_em(blk_name)
        stk_codes = stk_codes['代码'].to_list()
        for stk_code in stk_codes:
            try:
                ret[blk_name].append(f"{code_market[stk_code]},{stk_code}")
            except:
                print(stk_code)
    return ret


def get_all_gnbk_info():
    code_market = get_code_market_dict()
    blk_names = ak.stock_board_concept_name_em()['板块名称']
    ret = {}
    for blk_name in blk_names:
        print(blk_name)
        ret[blk_name] = []
        stk_codes = ak.stock_board_concept_cons_em(blk_name)
        stk_codes = stk_codes['代码'].to_list()
        for stk_code in stk_codes:
            try:
                ret[blk_name].append(f"{code_market[stk_code]},{stk_code}")
            except:
                print(stk_code)
    return ret


def write_blk_file(blk_info, filename):
    with open(filename, 'w', encoding='utf-8') as f:
        for k in blk_info:
            print(k)
            if blk_info[k]:
                f.write(f"[{k}]\n")
                for code in blk_info[k]:
                    f.write(f'{code}\n')


if __name__ == "__main__":
    # x = ak.stock_board_industry_name_em()
    # y = ak.stock_board_industry_cons_em(symbol=x['板块名称'][0])
    x = get_all_hybk_info()
    write_blk_file(x, 'hikyuu/config/block/hybk.ini')

    x = get_all_gnbk_info()
    write_blk_file(x, 'hikyuu/config/block/gnbk.ini')

    # print(x)
