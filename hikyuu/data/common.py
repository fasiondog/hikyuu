# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import requests
import re
import akshare as ak
import pandas as pd
import datetime
from hikyuu.core import cpp_bytes_to_vector_float_blob
from hikyuu.util import *


class MARKET:
    SH = 'SH'
    SZ = 'SZ'
    BJ = 'BJ'


g_market_list = [MARKET.SH, MARKET.SZ, MARKET.BJ]


class MARKETID:
    SH = 1
    SZ = 2
    BJ = 3


class STOCKTYPE:
    BLOCK = 0  # 板块
    A = 1  # A股
    INDEX = 2  # 指数
    B = 3  # B股
    FUND = 4  # 基金（非ETF）
    ETF = 5  # ETF
    ND = 6  # 国债
    BOND = 7  # 其他债券
    GEM = 8  # 创业板
    START = 9  # 科创板
    A_BJ = 11  # 北交所A股


def get_a_stktype_list():
    """获取A股市场证券类型元组，含B股"""
    return (STOCKTYPE.A, STOCKTYPE.INDEX, STOCKTYPE.B, STOCKTYPE.GEM, STOCKTYPE.START, STOCKTYPE.A_BJ)


def get_stktype_list(quotations=None):
    """
    根据行情类别获取股票类别元组

    :param quotations: 'stock'（股票） | 'fund'（基金） | 'bond'（债券）
    :rtype: tuple
    :return: 股票类别元组
    """
    if not quotations:
        return (1, 2, 3, 4, 5, 6, 7, 8, 9, 11)

    result = []
    for quotation in quotations:
        new_quotation = quotation.lower()
        if new_quotation == 'stock':
            result += list(get_a_stktype_list())
        elif new_quotation == 'fund':
            result += [STOCKTYPE.FUND, STOCKTYPE.ETF]
        elif new_quotation == 'bond':
            result += [STOCKTYPE.ND, STOCKTYPE.BOND]
        else:
            print('Unknow quotation: {}'.format(quotation))

    return tuple(result)


@hku_catch(ret=[], trace=True)
@timeout(120)
def get_stk_code_name_list(market: str) -> list:
    """
    获取指定证券交易所股票代码与名称列表
    :return: 代码名称对组成的列表：[{'code': 'code1': 'name': 'name1'}, ...]
    """
    # 获取深圳股票代码表
    if market == MARKET.SZ:
        ind_list = ["A股列表", "B股列表"]
        df = None
        for ind in ind_list:
            tmp_df = ak.stock_info_sz_name_code(ind)
            tmp_df.rename(columns={'A股代码': 'code', 'A股简称': 'name'}, inplace=True)
            df = pd.concat([df, tmp_df]) if df is not None else tmp_df
        hku_info("获取深圳证券交易所股票数量: {}", len(df) if df is not None else 0)
        return df[['code', 'name']].to_dict(orient='records') if df is not None else []

    # 获取上证股票代码表
    if market == MARKET.SH:
        ind_list = ["主板A股", "主板B股", "科创板"]
        df = None
        for ind in ind_list:
            tmp_df = ak.stock_info_sh_name_code(ind)
            tmp_df.rename(columns={'证券代码': 'code', '证券简称': 'name'}, inplace=True)
            df = pd.concat([df, tmp_df]) if df is not None else tmp_df
        hku_info("获取上海证券交易所股票数量: {}", len(df) if df is not None else 0)
        return df[['code', 'name']].to_dict(orient='records') if df is not None else []

    # 获取北京股票代码表
    if market == MARKET.BJ:
        df = ak.stock_info_bj_name_code()
        df.rename(columns={'证券代码': 'code', '证券简称': 'name'}, inplace=True)
        hku_info("获取北京证券交易所股票数量: {}", len(df) if df is not None else 0)
        return df[['code', 'name']].to_dict(orient='records') if df is not None else []


@hku_catch(ret=[], trace=True)
@timeout(120)
def get_index_code_name_list() -> list:
    """
    获取所有股票指数代码名称列表
    从新浪获取，多次频繁调用会被封禁IP，需10分钟后再试

    :return: [{'market_code': 'SHxxx'}, ...]
    """
    if hasattr(ak, 'stock_zh_index_spot_sina'):
        df = ak.stock_zh_index_spot_sina()
    elif hasattr(ak, 'stock_zh_index_spot_em'):
        df = ak.stock_zh_index_spot_em()
    else:
        df = ak.stock_zh_index_spot()
    res = [{'market_code': df.loc[i]['代码'].upper(), 'name': df.loc[i]['名称']} for i in range(len(df))]
    ret = [v for v in res if len(v['market_code']) == 8]
    return ret


g_fund_code_name_list = {}
for market in g_market_list:
    g_fund_code_name_list[market] = []
g_last_get_fund_code_name_list_date = datetime.date(1990, 12, 9)


@hku_catch(ret=[], trace=True)
@timeout(60)
def get_fund_code_name_list(market: str) -> list:
    """
    获取基金代码名称列表 (来源: sina)
    """
    # 保证一天只获取一次基金股票代码表，防止对 sina 的频繁访问
    global g_last_get_fund_code_name_list_date
    now = datetime.date.today()
    if now <= g_last_get_fund_code_name_list_date:
        return g_fund_code_name_list[market]

    ind_list = "封闭式基金", "ETF基金", "LOF基金"
    for ind in ind_list:
        df = ak.fund_etf_category_sina(ind)
        for i in range(len(df)):
            loc = df.loc[i]
            try:
                code, name = str(loc['代码']), str(loc['名称'])
                g_fund_code_name_list[code[:2].upper()].append(dict(code=code[2:], name=name))
            except Exception as e:
                hku_error("{}! {}", str(e), loc)
    hku_info("获取基金列表数量: {}", len(g_fund_code_name_list[market]))
    g_last_get_fund_code_name_list_date = now
    return g_fund_code_name_list[market]


@hku_catch(ret=[], trace=True)
def get_new_holidays():
    """获取新的交易所休假日历"""
    res = requests.get('https://www.tdx.com.cn/url/holiday/', timeout=60)
    res.encoding = res.apparent_encoding
    ret = re.findall(r'<textarea id="data" style="display:none;">([\s\w\d\W]+)</textarea>', res.text, re.M)[0].strip()
    day = [d.split('|')[:4] for d in ret.split('\n')]
    return [v[0] for v in day if len(v) >= 3 and v[2] == '中国']


@hku_catch(ret=[], trace=True)
@timeout(120)
def get_china_bond10_rate(start_date="19901219"):
    """获取中国国债收益率10年"""
    bond_zh_us_rate_df = ak.bond_zh_us_rate(start_date)
    df = bond_zh_us_rate_df[['中国国债收益率10年', '日期']].dropna()
    return [(v[1].strftime('%Y%m%d'), int(v[0]*10000)) for v in df.values]


def modifiy_code(code):
    if code.startswith(('0', '3')):
        return 'SZ' + code
    if code.startswith(('4', '8')):
        return 'BJ' + code
    if code.startswith('6'):
        return 'SH' + code
    else:
        hku_warn("Unknow code: {}", code)
        return None


def historyfinancialreader(filepath):
    """
    读取解析通达信目录的历史财务数据（来源: onefish, 公众号：一鱼策略）
    :param filepath: 字符串类型。传入文件路径
    :return: DataFrame格式。返回解析出的财务文件内容
    """
    import struct

    cw_file = open(filepath, 'rb')
    header_pack_format = '<1hI1H3L'
    header_size = struct.calcsize(header_pack_format)
    stock_item_size = struct.calcsize("<6s1c1L")
    data_header = cw_file.read(header_size)
    stock_header = struct.unpack(header_pack_format, data_header)
    max_count = stock_header[2]
    file_date = stock_header[1]
    print(file_date)
    report_size = stock_header[4]
    report_fields_count = int(report_size / 4)
    report_pack_format = '<{}f'.format(report_fields_count)
    results = []
    for stock_idx in range(0, max_count):
        cw_file.seek(header_size + stock_idx * struct.calcsize("<6s1c1L"))
        si = cw_file.read(stock_item_size)
        stock_item = struct.unpack("<6s1c1L", si)
        code = stock_item[0].decode("utf-8")
        foa = stock_item[2]
        cw_file.seek(foa)
        info_data = cw_file.read(struct.calcsize(report_pack_format))
        cw_info = list(struct.unpack(report_pack_format, info_data))
        report_date = int(cw_info[313])  # 财务公告日期
        report_date = 19000000 + report_date if report_date > 800000 else 20000000 + report_date
        # results.append((modifiy_code(code), report_date, cw_info))
        # results.append((file_date, modifiy_code(code), report_date, cpp_bytes_to_vector_float_blob(info_data)))
        results.append((file_date, modifiy_code(code), report_date, info_data))
    cw_file.close()
    return results
