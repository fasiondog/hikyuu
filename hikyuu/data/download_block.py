#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-10-05
#    Author: fasiondog

from concurrent.futures import ThreadPoolExecutor
from hikyuu.data.common import modifiy_code
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_block_em import *
import csv
import os
from datetime import datetime

_BLOCK_SAVE_PATH = os.path.expanduser('~') + '/.hikyuu/downloads/block'
if not os.path.exists(_BLOCK_SAVE_PATH):
    os.makedirs(_BLOCK_SAVE_PATH)


def is_file_can_download(filepath, sec_limit=5 * 24 * 60 * 60):
    """判断文件创建时间是否超过时长限制可以下载"""
    if not os.path.exists(filepath):
        return True

    file_mtime = os.path.getmtime(filepath)
    file_datetime = datetime.fromtimestamp(file_mtime)
    now = datetime.now()

    # 计算时间差是否超过时间限制
    return (now - file_datetime).total_seconds() > sec_limit


def save_block(stkcodes: list,  filename: str):
    """将板块数据保存为CSV文件，格式为板块名称/code"""
    with open(filename, 'w', encoding='utf-8') as f:
        for code in stkcodes:
            f.write(f"{code}\n")
    hku_info(f"已保存至 {filename}")


@hku_catch(ret={}, trace=True)
def down_em_all_hybk_info():
    """下载东财所有行业板块列表"""
    save_path = f'{_BLOCK_SAVE_PATH}/东财行业板块'
    if not os.path.exists(save_path):
        os.makedirs(save_path)
    blk_list = get_hybk_names()
    time.sleep(random.uniform(1, 3))
    total = len(blk_list)
    for i, blk in enumerate(blk_list):
        filename = f"{save_path}/{blk[0]}_{blk[1]}.txt"
        if is_file_can_download(filename, 5 * 24 * 60 * 60):
            stk_codes = get_hybk_cons_code(blk[0])
            hku_info(f"{i+1}|{total} 获取行业板块{blk[1]}成分: {len(stk_codes)}")
            stk_codes = [modifiy_code(code) for code in stk_codes]
            stk_codes = [code for code in stk_codes if code is not None]
            save_block(stk_codes, filename)
            time.sleep(random.uniform(1, 3))


@hku_catch(ret={}, trace=True)
def down_em_all_gnbk_info():
    """获取所有概念版本列表"""
    save_path = f'{_BLOCK_SAVE_PATH}/东财概念板块'
    if not os.path.exists(save_path):
        os.makedirs(save_path)
    blk_names = stock_board_concept_name_em()['板块名称']
    total = len(blk_names)
    for i, blk_name in enumerate(blk_names):
        filename = f"{save_path}/{blk_name}.txt"
        if is_file_can_download(filename, 5 * 24 * 60 * 60):
            stk_codes = stock_board_concept_cons_em(blk_name)
            stk_codes = stk_codes['代码'].to_list()
            stk_codes = [modifiy_code(code) for code in stk_codes]
            stk_codes = [code for code in stk_codes if code is not None]
            hku_info(f"{i+1}|{total} 获取概念板块{blk_name}成分: {len(stk_codes)}")
            save_block(stk_codes, filename)
            time.sleep(random.uniform(1, 3))


@hku_catch(ret={}, trace=True)
def down_em_all_dybk_info():
    """获取所有地域板块列表"""
    save_path = f'{_BLOCK_SAVE_PATH}/东财地域板块'
    if not os.path.exists(save_path):
        os.makedirs(save_path)
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

    total = len(blk_list)
    for i, v in enumerate(blk_list):
        blk_code, blk_name = v[0], v[1]
        filename = f"{save_path}/{blk_name}.txt"
        if not is_file_can_download(filename, 10 * 24 * 60 * 60):
            continue

        params["fs"] = f"b:{blk_code} f:!50"
        params["pn"] = 1
        time.sleep(random.uniform(1, 3))
        r = requests.get(url, params=params, timeout=15)
        data = r.json()
        if data["data"] is None:
            continue

        stk_json = r.json()
        stk_json = stk_json["data"]["diff"]
        stk_codes = []

        total_page = math.ceil(data["data"]["total"] / em_num_per_page)
        for page in range(2, total_page + 1):
            params["pn"] = page
            r = requests.get(url, params=params, timeout=15)
            stk_json = r.json()
            stk_json = stk_json["data"]["diff"]
            stk_codes.extend([f"{v['f12']}" for v in stk_json])
            time.sleep(random.uniform(1, 3))

        stk_codes = [modifiy_code(code) for code in stk_codes]
        stk_codes = [code for code in stk_codes if code is not None]
        save_block(stk_codes, filename)
        hku_info(f'{i+1}|{total} 获取地域板块{blk_name}成分: {len(stk_codes)}')


@hku_catch(ret={}, trace=True)
def download_all_zsbk_info():
    """获取所有指数成分股列表"""
    save_path = f'{_BLOCK_SAVE_PATH}/指数板块'
    if not os.path.exists(save_path):
        os.makedirs(save_path)

    blk_info = ak.index_stock_info()
    time.sleep(random.uniform(1, 3))

    blk_info['index_code'] = blk_info['index_code'].astype(str)  # 确保是字符串类型
    df_000 = blk_info[blk_info['index_code'].str.startswith('000')].reset_index(drop=True)  # 000前缀
    df_399 = blk_info[blk_info['index_code'].str.startswith('399')].reset_index(drop=True)  # 399前缀

    # 2. 交替合并两个DataFrame
    merged_rows = []
    max_length = max(len(df_000), len(df_399))  # 取两个DataFrame的最大长度

    for i in range(max_length):
        # 先加000前缀的行（如果存在）
        if i < len(df_000):
            merged_rows.append(df_000.iloc[i])
        # 再加399前缀的行（如果存在）
        if i < len(df_399):
            merged_rows.append(df_399.iloc[i])

    # 3. 转换为DataFrame
    blk_info = pd.DataFrame(merged_rows).reset_index(drop=True)

    not_need_blks = set(["000012", "000013", "000022", "000061", "000101", "000188",
                         "000817", "000847", "000849", "000850", "000851", "000853",
                         "000854", "000856", "000857", "000858", "000923", "000973",
                         "000974", "000996", "000997", "000999"])

    failed_sina = 0
    blk_set = {}
    blk_codes = blk_info["index_code"]
    blk_names = blk_info["display_name"]
    total = len(blk_codes)
    for i in range(total):
        blk_name = blk_names[i]
        blk_code = blk_codes[i]
        # 沪深指数有重复，避免深指覆盖
        if blk_name in blk_set or blk_code in not_need_blks:
            continue

        market_block_code = f'SH{blk_code}' if blk_code.startswith('0') else f'SZ{blk_code}'
        filename = f"{save_path}/{market_block_code}_{blk_name}.txt"
        if not is_file_can_download(filename, 10 * 24 * 60 * 60):
            continue

        try:
            if blk_code[:3] == "399":
                if failed_sina >= 5:
                    continue
                stk_codes = ak.index_stock_cons(symbol=blk_code)
                stk_codes = stk_codes['品种代码'].to_list()
            else:
                stk_codes = ak.index_stock_cons_csindex(symbol=blk_code)
                stk_codes = stk_codes['成分券代码'].to_list()

            stk_codes = [modifiy_code(code) for code in stk_codes]
            stk_codes = [code for code in stk_codes if code is not None]
            save_block(stk_codes, filename)
            hku_info("{}|{} 获取指数板块 {}|{} 成分: {}", i, total, blk_code, blk_name, len(stk_codes))
            blk_set[blk_name] = 1
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"Failed! {i}, {blk_code}, {blk_name} {str(e)}")
            if blk_code.startswith("399"):
                failed_sina += 1
            # raise e
        time.sleep(random.uniform(1, 3))


if __name__ == "__main__":
    from hikyuu.interactive import *
    code_market_dict = {}
    for s in sm:
        if s.valid and s.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ):
            code_market_dict[s.code] = s.market

    # print(code_market_dict)

    # download_em_block(code_market_dict, categorys=('指数板块', ))

    down_em_all_hybk_info()
    down_em_all_dybk_info()
    down_em_all_gnbk_info()
    download_all_zsbk_info()
