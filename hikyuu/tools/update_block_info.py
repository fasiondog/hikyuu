#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2023-12-18
#    Author: fasiondog

import requests
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


def get_all_dybk_info():
    url = "http://13.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": "50000",
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f3",
        "fs": "m:90+t:1+f:!50",
        "fields": "f12,f14",
        # "fields": "f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f12,f13,f14,f15,f16,f17,f18,f20,f21,f23,f24,f25,f22,f11,f62,f128,"
        #           "f136,f115,f152",
        "_": "1623833739532",
    }
    r = requests.get(url, params=params)
    data_json = r.json()
    data_json = data_json["data"]["diff"]

    params = {
        "pn": "1",
        "pz": "2000",
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f3",
        # "fs": f"b:{stock_board_code} f:!50",
        "fields": "f12",
        "_": "1626081702127",
    }

    ret = {}
    code_market = get_code_market_dict()
    for v in data_json:
        # print(v)
        blk_code = v["f12"]
        blk_name = v["f14"]
        print(blk_name)
        ret[blk_name] = []
        params["fs"] = f"b:{blk_code} f:!50"
        r = requests.get(url, params=params)
        stk_json = r.json()
        stk_json = stk_json["data"]["diff"]

        for item in stk_json:
            stk_code = item["f12"]
            try:
                ret[blk_name].append(f"{code_market[stk_code]},{stk_code}")
            except:
                print(stk_code)
    return ret


def get_all_zsbk_info():
    code_market = get_code_market_dict()
    blk_info = ak.index_stock_info()
    blk_codes = blk_info["index_code"]
    blk_names = blk_info["display_name"]
    ret = {}
    total = len(blk_codes)
    # for blk_name in blk_names:
    for i in range(total):
        blk_name = blk_names[i]
        blk_code = blk_codes[i]
        print(i, blk_name)
        # 沪深指数有重复，避免深指覆盖
        if blk_name in ret:
            continue
        try:
            ret[blk_name] = []
            stk_codes = ak.index_stock_cons_csindex(symbol=blk_code)
            stk_codes = stk_codes['成分券代码'].to_list()
            for stk_code in stk_codes:
                try:
                    ret[blk_name].append(f"{code_market[stk_code]},{stk_code}")
                except:
                    print(stk_code)
        except KeyboardInterrupt:
            exit(-1)
        except:
            print("Failed!", blk_code, blk_name)
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
    # # 更新行业板块
    # x = get_all_hybk_info()
    # write_blk_file(x, 'hikyuu/config/block/hybk.ini')

    # # 更新概念板块
    # x = get_all_gnbk_info()
    # write_blk_file(x, 'hikyuu/config/block/gnbk.ini')

    # # 更新地域板块
    # x = get_all_dybk_info()
    # write_blk_file(x, 'hikyuu/config/block/dybk.ini')

    # 更新指数成分
    x = get_all_zsbk_info()
    write_blk_file(x, 'hikyuu/config/block/zsbk.ini')
    # print(x)
