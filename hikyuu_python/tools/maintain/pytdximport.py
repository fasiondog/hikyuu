#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import datetime
from pytdx.hq import TDXParams

def to_pytdx_market(market):
    """转换为pytdx的market"""
    pytdx_market = {'SH': TDXParams.MARKET_SH, 'SZ': TDXParams.MARKET_SZ}
    return pytdx_market[market.upper()]

pytdx_market = {'SH': TDXParams.MARKET_SH, 'SZ': TDXParams.MARKET_SZ}

def pytdx_get_weights(connect, market, code, lastdatetime=None):
    xdxr_list = connect.get_xdxr_info(pytdx_market[market.upper()], code)
    weight_dict = {}
    for xdxr in xdxr_list:
        this_date = xdxr['year'] * 100000000 + xdxr['month'] * 1000000 + xdxr['day'] * 10000
        if lastdatetime is not None and this_date <= lastdatetime:
            continue

        if this_date in weight_dict:
            pass
        else:
            weight = {'countAsGift' : 0, #每10股送X股
                      'countForSell': 0, #每10股配X股
                      'priceForSell': 0, #配股价
                      'bonus'       : 0, #每10股红利
                      'increasement': 0, #每10股转增X股
                      'totalCount'  : 0, #总股本（万股）
                      'freeCount'   : 0 #流通股（万股）
                      }
            weight['bonus'] = 0 if xdxr['fenhong'] is None or xdxr['fenhong'] == 0 else int(round(xdxr['fenhong'], 3) * 1000)

            weight['freeCount'] = 0 if xdxr['panhouliutong'] is None or xdxr['panhouliutong'] == 0 else int(xdxr['panhouliutong'])
            weight['totalCount'] = 0 if xdxr['houzongguben'] is None or xdxr['houzongguben'] == 0 else int(xdxr['houzongguben'])



if __name__ == '__main__':
    from pytdx.hq import TdxHq_API, TDXParams
    api = TdxHq_API()
    api.connect('119.147.212.81', 7709)

    market = to_pytdx_market('SZ')
    stk_count = api.get_security_count(market)
    print(stk_count, int(stk_count/1000))
    x = []
    for i in range(int(stk_count/1000)+1):
        x += api.get_security_list(market, i * 1000)

    print(len(x))
    api.disconnect()