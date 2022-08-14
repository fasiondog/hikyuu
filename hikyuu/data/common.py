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

import akshare as ak


class MARKET:
    SH = 'SH'
    SZ = 'SZ'
    BJ = 'BJ'


class MARKETID:
    SH = 1
    SZ = 2
    BJ = 3


class STOCKTYPE:
    BLOCK = 0  #板块
    A = 1  #A股
    INDEX = 2  #指数
    B = 3  #B股
    FUND = 4  #基金（非ETF）
    ETF = 5  #ETF
    ND = 6  #国债
    BOND = 7  #其他债券
    GEM = 8  #创业板
    START = 9  #科创板


def get_stktype_list(quotations=None):
    """
    根据行情类别获取股票类别元组

    :param quotations: 'stock'（股票） | 'fund'（基金） | 'bond'（债券）
    :rtype: tuple
    :return: 股票类别元组
    """
    if not quotations:
        return (1, 2, 3, 4, 5, 6, 7, 8, 9)

    result = []
    for quotation in quotations:
        new_quotation = quotation.lower()
        if new_quotation == 'stock':
            result += [STOCKTYPE.A, STOCKTYPE.INDEX, STOCKTYPE.B, STOCKTYPE.GEM, STOCKTYPE.START]
        elif new_quotation == 'fund':
            result += [STOCKTYPE.FUND, STOCKTYPE.ETF]
        elif new_quotation == 'bond':
            result += [STOCKTYPE.ND, STOCKTYPE.BOND]
        else:
            print('Unknow quotation: {}'.format(quotation))

    return tuple(result)


def get_stk_code_name_list(market: str) -> list:
    """
    获取指定证券交易所股票代码与名称列表
    :return: 代码名称对组成的列表：[{'code': 'code1': 'name': 'name1'}, ...]
    """
    # 获取深圳股票代码表
    if market == MARKET.SZ:
        df = ak.stock_info_sz_name_code()
        df.rename(columns={'A股代码': 'code', 'A股简称': 'name'}, inplace=True)
        return df[['code', 'name']].to_dict(orient='records')

    # 获取上证股票代码表
    if market == MARKET.SH:
        df = ak.stock_info_sh_name_code()
        df.rename(columns={'证券代码': 'code', '证券简称': 'name'}, inplace=True)
        return df[['code', 'name']].to_dict(orient='records')

    # 获取北京股票代码表
    if market == MARKET.BJ:
        df = ak.stock_info_bj_name_code()
        df.rename(columns={'证券代码': 'code', '证券简称': 'name'}, inplace=True)
        return df[['code', 'name']].to_dict(orient='records')


def get_index_code_name_list() -> list:
    """
    获取所有股票指数代码名称列表
    从新浪获取，多次频繁调用会被封禁IP，需10分钟后再试
    
    :return: [{'market_code': 'SHxxx'}, ...]
    """
    df = ak.stock_zh_index_spot()
    return [{'market_code': df.loc[i]['代码'].upper(), 'name': df.loc[i]['名称']} for i in range(len(df))]
