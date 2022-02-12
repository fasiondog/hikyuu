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


class MARKET:
    SH = 'SH'
    SZ = 'SZ'


class MARKETID:
    SH = 1
    SZ = 2


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