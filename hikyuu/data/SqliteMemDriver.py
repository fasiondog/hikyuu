# -*- coding: utf8 -*-
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
#

import os
import sys
import sqlite3

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

    def _get_cursor(self):
        return self._cnx.cursor()

    @dbcommit
    def create_table(self):
        filename = os.path.dirname(__file__) + '/sqlite_mem/createdb.sql'
        with open(filename, 'r', encoding='utf8') as f:
            self.cursor.executescript(f.read())

    @dbcommit
    def insert_bars(self, market, code, bars, ktype):
        kline = [
            (
                code, bar['datetime'], bar['open'], bar['high'], bar['low'], bar['close'],
                bar['amount'], bar['volumn']
            ) for bar in bars
        ]
        sql = 'INSERT OR REPLACE INTO {}_{}(code, datetime, open, high, low, close, amount, \
               volumn) VALUES (?,?,?,?,?,?,?,?)'.format(market, ktype)
        self.cursor.executemany(sql, kline)
