#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240102
#    Author: fasiondog

import requests
import math
import time
import random
import akshare as ak
import pandas as pd
import re
from functools import lru_cache
from hikyuu.util import *

em_num_per_page = 100


@hku_catch(ret=[], trace=True)
def get_hybk_names():
    """获取所有行业(板块代码,板块名称)列表"""
    url = "https://19.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": str(em_num_per_page),
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f3",
        "fs": "m:90 t:2 f:!50",
        "fields": "f12,f14",
        "_": "1626075887768",
    }
    r = requests.get(url, params=params, timeout=15)
    data_json = r.json()
    ret = [(v['f12'], v['f14']) for v in data_json["data"]["diff"]]
    total_page = math.ceil(data_json["data"]["total"] / em_num_per_page)
    for page in range(2, total_page + 1):
        params["pn"] = page
        r = requests.get(url, params=params, timeout=15)
        data_json = r.json()
        if data_json["data"] is None:
            continue
        tmp = [(v['f12'], v['f14']) for v in data_json["data"]["diff"]]
        ret.extend(tmp)
        time.sleep(random.uniform(1, 3))
    return ret


@hku_catch(ret=[], trace=True)
def get_hybk_cons_code(blk_code):
    "获取指定行业板块成分代码列表"
    url = "http://30.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": str(em_num_per_page),
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f3",
        "fs": f"b:{blk_code} f:!50",
        "fields": "f12",
        "_": "1626081702127",
    }
    r = requests.get(url, params=params, timeout=15)
    data_json = r.json()
    ret = [v['f12'] for v in data_json["data"]["diff"]]
    total_page = math.ceil(data_json["data"]["total"] / em_num_per_page)
    for page in range(2, total_page + 1):
        params["pn"] = page
        r = requests.get(url, params=params, timeout=15)
        data_json = r.json()
        if data_json["data"] is None:
            continue
        tmp = [v['f12'] for v in data_json["data"]['diff']]
        time.sleep(random.uniform(1, 3))
        ret.extend(tmp)
    return ret


@hku_catch(ret={}, trace=True)
def get_all_hybk_info(code_market_dict, sep=""):
    """获取所有行业板块列表"""
    blk_list = get_hybk_names()
    ret = {}
    for blk in blk_list:
        stk_codes = get_hybk_cons_code(blk[0])
        hku_info(f"获取行业板块{blk[1]}成分: {len(stk_codes)}")
        ret[blk[1]] = [
            f"{code_market_dict[stk_code]}{sep}{stk_code}" for stk_code in stk_codes if stk_code in code_market_dict]
    return ret


def get_tqdm(enable: bool = True):
    """
    返回适用于当前环境的 tqdm 对象。

    Args:
        enable (bool): 是否启用进度条。默认为 True。

    Returns:
        tqdm 对象。
    """
    if not enable:
        # 如果进度条被禁用，返回一个不显示进度条的 tqdm 对象
        return lambda iterable, *args, **kwargs: iterable

    try:
        # 尝试检查是否在 jupyter notebook 环境中，有利于退出进度条
        # noinspection PyUnresolvedReferences
        shell = get_ipython().__class__.__name__
        if shell == "ZMQInteractiveShell":
            from tqdm.notebook import tqdm
        else:
            from tqdm import tqdm
    except (NameError, ImportError):
        # 如果不在 Jupyter 环境中，就使用标准 tqdm
        from tqdm import tqdm

    return tqdm


def fetch_paginated_data(url: str, base_params: dict, timeout: int = 15):
    """
    东方财富-分页获取数据并合并结果
    https://quote.eastmoney.com/f1.html?newcode=0.000001
    :param url: 股票代码
    :type url: str
    :param base_params: 基础请求参数
    :type base_params: dict
    :param timeout: 请求超时时间
    :type timeout: str
    :return: 合并后的数据
    :rtype: pandas.DataFrame
    """
    # 复制参数以避免修改原始参数
    params = base_params.copy()
    # 获取第一页数据，用于确定分页信息
    r = requests.get(url, params=params, timeout=timeout)
    data_json = r.json()
    # 计算分页信息
    per_page_num = len(data_json["data"]["diff"])
    total_page = math.ceil(data_json["data"]["total"] / per_page_num)
    # 存储所有页面数据
    temp_list = []
    # 添加第一页数据
    temp_list.append(pd.DataFrame(data_json["data"]["diff"]))
    # 获取进度条
    tqdm = get_tqdm()
    # 获取剩余页面数据
    for page in tqdm(range(2, total_page + 1), leave=False):
        params.update({"pn": page})
        r = requests.get(url, params=params, timeout=timeout)
        data_json = r.json()
        inner_temp_df = pd.DataFrame(data_json["data"]["diff"])
        temp_list.append(inner_temp_df)
        time.sleep(random.uniform(1, 3))
    # 合并所有数据
    temp_df = pd.concat(temp_list, ignore_index=True)
    temp_df["f3"] = pd.to_numeric(temp_df["f3"], errors="coerce")
    temp_df.sort_values(by=["f3"], ascending=False, inplace=True, ignore_index=True)
    temp_df.reset_index(inplace=True)
    temp_df["index"] = temp_df["index"].astype(int) + 1
    return temp_df


def stock_board_concept_name_em() -> pd.DataFrame:
    """
    from akshare
    东方财富网-行情中心-沪深京板块-概念板块-名称
    https://quote.eastmoney.com/center/boardlist.html#concept_board
    :return: 概念板块-名称
    :rtype: pandas.DataFrame
    """
    url = "https://59.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": "100",
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f12",
        "fs": "m:90 t:3 f:!50",
        "fields": "f2,f3,f4,f8,f12,f14,f15,f16,f17,f18,f20,f21,f24,f25,f22,f33,f11,f62,f128,f124,f107,f104,f105,f136",
    }
    temp_df = fetch_paginated_data(url, params)
    temp_df.columns = [
        "排名",
        "最新价",
        "涨跌幅",
        "涨跌额",
        "换手率",
        "_",
        "板块代码",
        "板块名称",
        "_",
        "_",
        "_",
        "_",
        "总市值",
        "_",
        "_",
        "_",
        "_",
        "_",
        "_",
        "上涨家数",
        "下跌家数",
        "_",
        "_",
        "领涨股票",
        "_",
        "_",
        "领涨股票-涨跌幅",
    ]
    temp_df = temp_df[
        [
            "排名",
            "板块名称",
            "板块代码",
            "最新价",
            "涨跌额",
            "涨跌幅",
            "总市值",
            "换手率",
            "上涨家数",
            "下跌家数",
            "领涨股票",
            "领涨股票-涨跌幅",
        ]
    ]
    temp_df["最新价"] = pd.to_numeric(temp_df["最新价"], errors="coerce")
    temp_df["涨跌额"] = pd.to_numeric(temp_df["涨跌额"], errors="coerce")
    temp_df["涨跌幅"] = pd.to_numeric(temp_df["涨跌幅"], errors="coerce")
    temp_df["总市值"] = pd.to_numeric(temp_df["总市值"], errors="coerce")
    temp_df["换手率"] = pd.to_numeric(temp_df["换手率"], errors="coerce")
    temp_df["上涨家数"] = pd.to_numeric(temp_df["上涨家数"], errors="coerce")
    temp_df["下跌家数"] = pd.to_numeric(temp_df["下跌家数"], errors="coerce")
    temp_df["领涨股票-涨跌幅"] = pd.to_numeric(
        temp_df["领涨股票-涨跌幅"], errors="coerce"
    )
    return temp_df


@lru_cache()
def __stock_board_concept_name_em() -> pd.DataFrame:
    """
    东方财富网-行情中心-沪深京板块-概念板块-名称
    https://quote.eastmoney.com/center/boardlist.html#concept_board
    :return: 概念板块-名称
    :rtype: pandas.DataFrame
    """
    url = "https://79.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": "100",
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f12",
        "fs": "m:90 t:3 f:!50",
        "fields": "f2,f3,f4,f8,f12,f14,f15,f16,f17,f18,f20,f21,f24,f25,f22,f33,f11,f62,f128,f124,f107,f104,f105,f136",
    }
    temp_df = fetch_paginated_data(url, params)
    temp_df.columns = [
        "排名",
        "最新价",
        "涨跌幅",
        "涨跌额",
        "换手率",
        "_",
        "板块代码",
        "板块名称",
        "_",
        "_",
        "_",
        "_",
        "总市值",
        "_",
        "_",
        "_",
        "_",
        "_",
        "_",
        "上涨家数",
        "下跌家数",
        "_",
        "_",
        "领涨股票",
        "_",
        "_",
        "领涨股票-涨跌幅",
    ]
    temp_df = temp_df[
        [
            "排名",
            "板块名称",
            "板块代码",
            "最新价",
            "涨跌额",
            "涨跌幅",
            "总市值",
            "换手率",
            "上涨家数",
            "下跌家数",
            "领涨股票",
            "领涨股票-涨跌幅",
        ]
    ]
    temp_df["最新价"] = pd.to_numeric(temp_df["最新价"], errors="coerce")
    temp_df["涨跌额"] = pd.to_numeric(temp_df["涨跌额"], errors="coerce")
    temp_df["涨跌幅"] = pd.to_numeric(temp_df["涨跌幅"], errors="coerce")
    temp_df["总市值"] = pd.to_numeric(temp_df["总市值"], errors="coerce")
    temp_df["换手率"] = pd.to_numeric(temp_df["换手率"], errors="coerce")
    temp_df["上涨家数"] = pd.to_numeric(temp_df["上涨家数"], errors="coerce")
    temp_df["下跌家数"] = pd.to_numeric(temp_df["下跌家数"], errors="coerce")
    temp_df["领涨股票-涨跌幅"] = pd.to_numeric(
        temp_df["领涨股票-涨跌幅"], errors="coerce"
    )
    return temp_df


def stock_board_concept_cons_em(symbol: str = "融资融券") -> pd.DataFrame:
    """
    东方财富-沪深板块-概念板块-板块成份
    https://quote.eastmoney.com/center/boardlist.html#boards-BK06551
    :param symbol: 板块名称或者板块代码
    :type symbol: str
    :return: 板块成份
    :rtype: pandas.DataFrame
    """
    if re.match(pattern=r"^BK\d+", string=symbol):
        stock_board_code = symbol
    else:
        stock_board_concept_em_map = __stock_board_concept_name_em()
        stock_board_code = stock_board_concept_em_map[
            stock_board_concept_em_map["板块名称"] == symbol
        ]["板块代码"].values[0]
    url = "https://29.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": "100",
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f12",
        "fs": f"b:{stock_board_code} f:!50",
        "fields": "f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f12,f13,f14,f15,f16,f17,f18,f20,f21,f23,"
        "f24,f25,f22,f11,f62,f128,f136,f115,f152,f45",
    }
    temp_df = fetch_paginated_data(url, params)
    temp_df.columns = [
        "序号",
        "_",
        "最新价",
        "涨跌幅",
        "涨跌额",
        "成交量",
        "成交额",
        "振幅",
        "换手率",
        "市盈率-动态",
        "_",
        "_",
        "代码",
        "_",
        "名称",
        "最高",
        "最低",
        "今开",
        "昨收",
        "_",
        "_",
        "_",
        "市净率",
        "_",
        "_",
        "_",
        "_",
        "_",
        "_",
        "_",
        "_",
        "_",
        "_",
    ]
    temp_df = temp_df[
        [
            "序号",
            "代码",
            "名称",
            "最新价",
            "涨跌幅",
            "涨跌额",
            "成交量",
            "成交额",
            "振幅",
            "最高",
            "最低",
            "今开",
            "昨收",
            "换手率",
            "市盈率-动态",
            "市净率",
        ]
    ]
    temp_df["最新价"] = pd.to_numeric(temp_df["最新价"], errors="coerce")
    temp_df["涨跌幅"] = pd.to_numeric(temp_df["涨跌幅"], errors="coerce")
    temp_df["涨跌额"] = pd.to_numeric(temp_df["涨跌额"], errors="coerce")
    temp_df["成交量"] = pd.to_numeric(temp_df["成交量"], errors="coerce")
    temp_df["成交额"] = pd.to_numeric(temp_df["成交额"], errors="coerce")
    temp_df["振幅"] = pd.to_numeric(temp_df["振幅"], errors="coerce")
    temp_df["最高"] = pd.to_numeric(temp_df["最高"], errors="coerce")
    temp_df["最低"] = pd.to_numeric(temp_df["最低"], errors="coerce")
    temp_df["今开"] = pd.to_numeric(temp_df["今开"], errors="coerce")
    temp_df["昨收"] = pd.to_numeric(temp_df["昨收"], errors="coerce")
    temp_df["换手率"] = pd.to_numeric(temp_df["换手率"], errors="coerce")
    temp_df["市盈率-动态"] = pd.to_numeric(temp_df["市盈率-动态"], errors="coerce")
    temp_df["市净率"] = pd.to_numeric(temp_df["市净率"], errors="coerce")
    return temp_df


@hku_catch(ret={}, trace=True)
def get_all_gnbk_info(code_market_dict, sep=""):
    """获取所有概念版本列表"""
    blk_names = stock_board_concept_name_em()['板块名称']
    ret = {}
    for i, blk_name in enumerate(blk_names):
        stk_codes = stock_board_concept_cons_em(blk_name)
        stk_codes = stk_codes['代码'].to_list()
        hku_info(f"{i} 获取概念板块{blk_name}成分: {len(stk_codes)}")
        ret[blk_name] = [
            f"{code_market_dict[stk_code]}{sep}{stk_code}" for stk_code in stk_codes if stk_code in code_market_dict]
    return ret


@hku_catch(ret=[], trace=True)
def get_dybk_names():
    """获取所有地域板块名称列表"""
    url = "http://13.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": str(em_num_per_page),
        "po": "1",
        "np": "1",
        "ut": "bd1d9ddb04089700cf9c27f6f7426281",
        "fltt": "2",
        "invt": "2",
        "fid": "f3",
        "fs": "m:90+t:1+f:!50",
        "fields": "f12,f14",
        "_": "1623833739532",
    }
    r = requests.get(url, params=params, timeout=15)
    data_json = r.json()
    hku_check(data_json['data'] is not None, "获取地域板块名称列表失败!")
    ret = [(v["f12"], v["f14"]) for v in data_json["data"]["diff"]]

    total_page = math.ceil(data_json["data"]["total"] / em_num_per_page)
    for page in range(2, total_page + 1):
        params["pn"] = page
        r = requests.get(url, params=params, timeout=15)
        data_json = r.json()
        if data_json["data"] is not None:
            ret.extend([(v["f12"], v["f14"]) for v in data_json["data"]["diff"]])
        time.sleep(random.uniform(1, 3))
    return ret


@hku_catch(ret={}, trace=True)
def get_all_dybk_info(code_market_dict, sep=""):
    """获取所有地域板块列表"""
    blk_list = get_dybk_names()

    url = "http://13.push2.eastmoney.com/api/qt/clist/get"
    params = {
        "pn": "1",
        "pz": str(em_num_per_page),
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
    for v in blk_list:
        blk_code, blk_name = v[0], v[1]
        params["fs"] = f"b:{blk_code} f:!50"
        params["pn"] = 1
        r = requests.get(url, params=params, timeout=15)
        data = r.json()
        if data["data"] is None:
            continue

        stk_json = r.json()
        stk_json = stk_json["data"]["diff"]
        ret[blk_name] = [
            f"{code_market_dict[v['f12']]}{sep}{v['f12']}" for v in stk_json if v['f12'] in code_market_dict]

        total_page = math.ceil(data["data"]["total"] / em_num_per_page)
        for page in range(2, total_page + 1):
            params["pn"] = page
            r = requests.get(url, params=params, timeout=15)
            stk_json = r.json()
            stk_json = stk_json["data"]["diff"]
            ret[blk_name].extend(
                [f"{code_market_dict[v['f12']]}{sep}{v['f12']}" for v in stk_json if v["f12"] in code_market_dict])
            time.sleep(random.uniform(1, 3))
        hku_info(f'获取地域板块{blk_name}成分: {len(ret[blk_name])}')

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
        # print(i, blk_name)
        # 沪深指数有重复，避免深指覆盖
        if blk_name in ret:
            continue
        try:
            stk_codes = ak.index_stock_cons_csindex(symbol=blk_code)
            stk_codes = stk_codes['成分券代码'].to_list()
            hku_info("{} 获取指数板块{}成分: {}", i, blk_name, len(stk_codes))
            ret[blk_name] = [
                f"{code_market_dict[stk_code]}{sep}{stk_code}" for stk_code in stk_codes if stk_code in code_market_dict]
        except KeyboardInterrupt:
            break
        except:
            # print("Failed!", blk_code, blk_name)
            pass
    return ret


if __name__ == "__main__":
    blks = get_hybk_names()
    # blks = get_hybk_cons_code('BK0480')
    print(blks)
