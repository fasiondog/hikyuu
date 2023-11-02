# -*- coding: utf8 -*-
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
#

import os
import sys
import sqlite3

from hikyuu import Datetime
from hikyuu.util import *


def dbcommit(func):
    def wrapfunc(*args, **kwargs):
        result = None
        x = args[0]
        try:
            hku_check(x.cursor is None, "You cannot nest using database cursors!")
            x.cursor = x._get_cursor()
            result = func(*args, **kwargs)
            x._cnx.commit()
        except Exception as e:
            hku_error("{} [{}.{}]".format(e, func.__module__, func.__name__))
            result = None

        if x.cursor is not None:
            x.cursor.close()
            x.cursor = None
        return result

    return wrapfunc


class SqliteMemDriver:
    def __init__(self):
        self._cnx = sqlite3.connect("file:hikyuu_mem?mode=memory&cache=shared")
        self.cursor = None
        self._create_table()

    def __del__(self):
        self.close()

    def close(self):
        if self._cnx:
            self._cnx.close()
            self._cnx = None

    def _get_cursor(self):
        return self._cnx.cursor()

    @dbcommit
    def _create_table(self):
        filename = os.path.dirname(__file__) + '/sqlite_mem_sql/createdb.sql'
        with open(filename, 'r', encoding='utf8') as f:
            self.cursor.executescript(f.read())

    @dbcommit
    def insert_bars(self, market, bars, ktype):
        """批量插入一批Bar, 如果 bar['market'] 和 指定的 market 不一致，将被忽略

        :param str market: 市场简称
        :param bar: 可通过键值方式取值，必须包含以下key：market, code, datetime, open, high
                    low, close, amount, volume, bid1,bid1_amount .. bid5, ask1, ask1_amount...
        """
        kline = [
            (
                bar['code'], Datetime(bar['datetime']).start_of_day().number if ktype == 'day' else
                Datetime(bar['datetime']).number, bar['open'], bar['high'], bar['low'],
                bar['close'], bar['amount'], bar['volume'], bar['bid1'], bar['bid1_amount'],
                bar['bid2'], bar['bid2_amount'], bar['bid3'], bar['bid3_amount'], bar['bid4'],
                bar['bid4_amount'], bar['bid5'], bar['bid5_amount'], bar['ask1'],
                bar['ask1_amount'], bar['ask2'], bar['ask2_amount'], bar['ask3'],
                bar['ask3_amount'], bar['ask4'], bar['ask4_amount'], bar['ask5'], bar['ask5_amount']
            ) for bar in bars if market.lower() == bar['market'].lower()
        ]
        sql = 'INSERT OR REPLACE INTO {}_{}(code, datetime, open, high, low, close, amount, \
               volume, bid1, bid1_amount, bid2, bid2_amount, bid3, bid3_amount, bid4, bid4_amount, \
               bid5, bid5_amount, ask1, ask1_amount, ask2, ask2_amount, ask3, ask3_amount, \
               ask4, ask4_amount, ask5, ask5_amount) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,\
               ?,?,?,?,?,?,?,?,?,?)'.format(market, ktype)
        self.cursor.executemany(sql, kline)
