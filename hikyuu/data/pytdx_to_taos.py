#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-06-13
#    Author: fasiondog

import sys
import math
import datetime
from pytdx.hq import TDXParams

from hikyuu.util.mylog import hku_error, hku_debug

import taos

from hikyuu.data.common import *
from hikyuu.data.common_pytdx import to_pytdx_market, pytdx_get_day_trans
from hikyuu.data.common_taos import (
    create_database,
    get_marketid,
    get_codepre_list,
    get_stock_list,
    get_table,
    get_lastdatetime,
    update_extern_data,
)


@hku_catch(ret=0, trace=True)
def import_index_name(connect):
    """
    导入所有指数代码表

    :param connect: taos 连接对象
    :return: 指数个数
    """
    index_list = get_index_code_name_list()
    if not index_list:
        return 0

    cur = connect.cursor()
    cur.execute(
        "select cast(stockid as bigint), marketid, code from hku_base.n_stock where type={}".format(
            STOCKTYPE.INDEX
        )
    )
    a = [v for v in cur]
    oldStockDict = {}
    for oldstock in a:
        oldstockid = oldstock[0]
        oldcode = "{}{}".format(
            MARKET.SH if oldstock[1] == MARKETID.SH else MARKET.SZ, oldstock[2]
        )
        oldStockDict[oldcode] = oldstockid

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    now_id = int(datetime.datetime.now().timestamp() * 1000)
    count = 0
    for index in index_list:
        stockid = oldStockDict[index["market_code"]] if index["market_code"] in oldStockDict else now_id + count
        # print(stockid)

        marketid = (
            MARKETID.SH if index["market_code"][:2] == MARKET.SH else MARKETID.SZ
        )
        sql = (
            "insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) \
                       values (%s, %s, '%s', '%s', %s, %s, %s, %s)"
            % (
                stockid,
                marketid,
                index["market_code"][2:],
                index["name"],  # .encode('utf8').decode('utf8', 'ignore'),
                STOCKTYPE.INDEX,
                1,
                today,
                99999999,
            )
        )
        count += 1
        cur.execute(sql)
    cur.close()
    return len(index_list)


@hku_catch(ret=0, trace=True)
def import_stock_name(connect, api, market, quotations=None):
    """更新每只股票的名称、当前是否有效性、起始日期及结束日期
    如果导入的代码表中不存在对应的代码，则认为该股已失效

    :param connect: sqlite3实例
    :param api: pytdx接口，必须在函数外进行连接
    :param market: 'SH' | 'SZ'
    :param quotations: 待导入的行情类别，空为导入全部 'stock' | 'fund' | 'bond' | None
    """
    cur = connect.cursor()

    deSet = set()  # 记录退市证券
    if market == MARKET.SH:
        df = ak.stock_info_sh_delist()
        l = df[['公司代码', '公司简称']].to_dict(orient='records') if not df .empty else []
        for stock in l:
            code = str(stock['公司代码'])
            deSet.add(code)
    elif market == MARKET.SZ:
        for t in ['暂停上市公司', '终止上市公司']:
            df = ak.stock_info_sz_delist(t)
            l = df[['证券代码', '证券简称']].to_dict(orient='records') if not df.empty else []
            for stock in l:
                code = str(stock['证券代码'])
                deSet.add(code)

    newStockDict = {}
    stk_list = get_stk_code_name_list(market)
    if not stk_list:
        hku_error("获取 {} 股票代码表失败", market)
        return 0

    if not quotations or "fund" in [v.lower() for v in quotations]:
        stk_list.extend(get_fund_code_name_list(market))
    for stock in stk_list:
        code = str(stock["code"])
        if code not in deSet:
            newStockDict[code] = stock["name"]

    marketid = get_marketid(connect, market)

    stktype_list = get_stktype_list(quotations)
    stktype_list = list(stktype_list)
    stktype_list.remove(STOCKTYPE.INDEX)  # 移除指数类型
    stktype_list = tuple(stktype_list)
    cur.execute(
        "select cast(stockid as bigint), marketid, code, name, type, valid, startDate, endDate from hku_base.n_stock where marketid={} and type in {}".format(
            marketid, stktype_list
        )
    )
    a = [v for v in cur]
    oldStockDict = {}
    for oldstock in a:
        oldstockid, oldmarketid, oldcode, oldname, oldtype, oldvalid, oldstartDate, oldendDate = oldstock
        oldStockDict[oldcode] = oldstockid

        # 新的代码表中无此股票，则置为无效
        if (oldvalid == 1) and ((oldcode not in newStockDict) or oldcode in deSet):
            cur.execute(
                f"insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) values ({oldstockid}, {oldmarketid}, '{oldcode}', '{oldname}', {oldtype}, 0, {oldstartDate}, {oldendDate})"
            )

        # 股票名称发生变化，更新股票名称;如果原无效，则置为有效
        if oldcode in newStockDict:
            if oldname != newStockDict[oldcode]:
                cur.execute(
                    f"insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) values ({oldstockid}, {oldmarketid}, '{oldcode}', '{newStockDict[oldcode]}', {oldtype}, {oldvalid}, {oldstartDate}, {oldendDate})")
            if oldvalid == 0:
                cur.execute(
                    f"insert into hku_base.n_stock (stockid, marketid, code, name, type, valid, startDate, endDate) values ({oldstockid}, {oldmarketid}, '{oldcode}', '{oldname}', {oldtype}, 1, {oldstartDate}, 99999999)")

    # 处理新出现的股票
    codepre_list = get_codepre_list(connect, marketid, quotations)

    today = datetime.date.today()
    today = today.year * 10000 + today.month * 100 + today.day
    now_id = int(datetime.datetime.now().timestamp() * 1000)
    count = 0
    for code in newStockDict:
        if code not in oldStockDict:
            for codepre in codepre_list:
                length = len(codepre[0])
                if code[:length] == codepre[0]:
                    # print(market, code, newStockDict[code], codepre)
                    sql = (
                        "insert into `hku_base`.`n_stock` (stockid, marketid, code, name, type, valid, startDate, endDate) \
                           values (%s, %s, '%s', '%s', %s, %s, %s, %s)"
                        % (
                            now_id + count,
                            marketid,
                            code,
                            newStockDict[code],
                            codepre[1],
                            1,
                            today,
                            99999999,
                        )
                    )
                    cur.execute(sql)
                    count += 1
                    break

    # print('%s新增股票数：%i' % (market.upper(), count))
    cur.close()
    return count


if __name__ == '__main__':
    import os
    from configparser import ConfigParser
    dev_config = ConfigParser()
    dev_config.read(os.path.expanduser("~") + '/workspace/dev.ini')
    db = 'taos54'
    user = dev_config.get(db, 'user')
    password = dev_config.get(db, 'pwd')
    host = dev_config.get(db, 'host')
    port = dev_config.getint(db, 'port')

    connect = taos.connect(
        user=user, password=password, host=host, port=port)

    import_index_name(connect)

    import_stock_name(connect, None, MARKET.SH, None)

    connect.close()
