#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

import requests
import akshare as ak
from hikyuu.util import *


@hku_catch(ret={}, trace=True)
def get_all_hybk_info(code_market_dict, sep=""):
    """获取所有行业板块列表"""
    blk_names = ak.stock_board_industry_name_em()['板块名称']
    ret = {}
    for blk_name in blk_names:
        # print(blk_name)
        ret[blk_name] = []
        stk_codes = ak.stock_board_industry_cons_em(blk_name)
        stk_codes = stk_codes['代码'].to_list()
        for stk_code in stk_codes:
            try:
                ret[blk_name].append(f"{code_market_dict[stk_code]}{sep}{stk_code}")
            except:
                # print(stk_code)
                pass
    return ret


@hku_catch(ret={}, trace=True)
def get_all_gnbk_info(code_market_dict, sep=""):
    """获取所有概念版本列表"""
    blk_names = ak.stock_board_concept_name_em()['板块名称']
    ret = {}
    for blk_name in blk_names:
        # print(blk_name)
        ret[blk_name] = []
        stk_codes = ak.stock_board_concept_cons_em(blk_name)
        stk_codes = stk_codes['代码'].to_list()
        for stk_code in stk_codes:
            try:
                ret[blk_name].append(f"{code_market_dict[stk_code]}{sep}{stk_code}")
            except:
                # print(stk_code)
                pass
    return ret


@hku_catch(ret={}, trace=True)
def get_all_dybk_info(code_market_dict, sep=""):
    """获取所有地域板块列表"""
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
    for v in data_json:
        # print(v)
        blk_code = v["f12"]
        blk_name = v["f14"]
        # print(blk_name)
        ret[blk_name] = []
        params["fs"] = f"b:{blk_code} f:!50"
        r = requests.get(url, params=params)
        stk_json = r.json()
        stk_json = stk_json["data"]["diff"]

        for item in stk_json:
            stk_code = item["f12"]
            try:
                ret[blk_name].append(f"{code_market_dict[stk_code]}{sep}{stk_code}")
            except:
                # print(stk_code)
                pass
    return ret


@hku_catch(ret={}, trace=True)
def get_all_zsbk_info(code_market_dict, sep=""):
    """获取所有指数成分股列表"""
    blk_info = ak.index_stock_info()
    blk_codes = blk_info["index_code"]
    blk_names = blk_info["display_name"]
    ret = {}
    total = len(blk_codes)
    for i in range(total):
        blk_name = blk_names[i]
        blk_code = blk_codes[i]
        hku_info("{} 获取指数板块 {} 详情", i, blk_name)
        # print(i, blk_name)
        # 沪深指数有重复，避免深指覆盖
        if blk_name in ret:
            continue
        try:
            ret[blk_name] = []
            stk_codes = ak.index_stock_cons_csindex(symbol=blk_code)
            stk_codes = stk_codes['成分券代码'].to_list()
            for stk_code in stk_codes:
                try:
                    ret[blk_name].append(f"{code_market_dict[stk_code]}{sep}{stk_code}")
                except:
                    # print(stk_code)
                    pass
        except KeyboardInterrupt:
            break
        except:
            # print("Failed!", blk_code, blk_name)
            pass
    return ret
