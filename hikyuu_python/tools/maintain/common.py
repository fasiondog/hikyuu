#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

MARKET_SH = 1
MARKET_SZ = 2

STOCKTYPE_A     = 1  #A股
STOCKTYPE_INDEX = 2  #指数
STOCKTYPE_B     = 3  #B股
STOCKTYPE_FUND  = 4  #基金（非ETF）
STOCKTYPE_ETF   = 5  #ETF
STOCKTYPE_ND    = 6  #国债
STOCKTYPE_BOND  = 7  #其他债券
STOCKTYPE_GEM   = 8  #创业板
STOCKTYPE_BTC   = 9  #数字币

def get_stktype_list(quotations=None):
    if not quotations:
        return (1, 2, 3, 4, 5, 6, 7, 8, 9)

    result = []
    for quotation in quotations:
        new_quotation = quotation.lower()
        if new_quotation == 'stock':
            result += [STOCKTYPE_A, STOCKTYPE_INDEX, STOCKTYPE_B, STOCKTYPE_GEM]
        elif new_quotation == 'fund':
            result += [STOCKTYPE_FUND, STOCKTYPE_ETF]
        elif new_quotation == 'bond':
            result += [STOCKTYPE_ND, STOCKTYPE_BOND]
        else:
            print('Unknow quotation: {}'.format(quotation))

    return tuple(result)