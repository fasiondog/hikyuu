# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog/hikyuu
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

import os
from pathlib import Path

import mysql.connector

from common import MARKETID, get_stktype_list


def is_exist_db(connect):
    """数据库是否已存在"""
    cur = connect.cursor()
    cur.execute("SELECT 1 FROM information_schema.SCHEMATA where SCHEMA_NAME='hku_base'")
    a = cur.fetchone()
    cur.close()
    return True if a else False


def get_db_version(connect):
    try:
        cur = connect.cursor()
        cur.execute("select `version` from `hku_base`.`version`")
        a = cur.fetchone()
        cur.close()
        return a[0] if a else 0
    except:
        return 0


def create_database(connect):
    """创建数据库"""
    sql_dir = os.path.dirname(__file__) + "/mysql_upgrade"
    cur = connect.cursor()
    if not is_exist_db(connect):
        filename = sql_dir + "/createdb.sql"
        with open(filename, 'r', encoding='utf8') as f:
            sql = f.read()
        for x in cur.execute(sql, multi=True):
            #print(x.statement)
            pass

    db_version = get_db_version(connect)
    files = [x for x in Path(sql_dir).iterdir() if x.name != 'createdb.sql' and int(x.stem) > db_version and not x.is_dir()]
    files.sort()
    for file in files:
        sql = file.read_text(encoding='utf8')
        for x in cur.execute(sql, multi=True):
            #print(x.statement)
            pass

    connect.commit()
    cur.close()


def get_marketid(connect, market):
    cur = connect.cursor()
    cur.execute("select marketid, market from `hku_base`.`market` where market='{}'".format(market.upper()))
    marketid = cur.fetchone()
    marketid = marketid[0]
    cur.close()
    return marketid


def get_codepre_list(connect, marketid, quotations):
    """获取前缀代码表"""
    stktype_list = get_stktype_list(quotations)
    sql = "select codepre, type from `hku_base`.`coderuletype` " \
          "where marketid={marketid} and type in {type_list}"\
        .format(marketid=marketid, type_list=stktype_list)
    cur = connect.cursor()
    cur.execute(sql)
    a = cur.fetchall()
    cur.close()
    return sorted(a, key=lambda k: len(k[0]), reverse=True)


def get_stock_list(connect, market, quotations):
    marketid = get_marketid(connect, market)
    stktype_list = get_stktype_list(quotations)
    sql = "select stockid, marketid, code, valid, type from `hku_base`.`stock` where marketid={} and type in {}"\
        .format(marketid, stktype_list)
    cur = connect.cursor()
    cur.execute(sql)
    a = cur.fetchall()
    connect.commit()
    cur.close()
    return a


def get_table(connect, market, code, ktype):
    """note: market: 'DAY' | 'MIN' | 'MIN5' """
    cur = connect.cursor()
    schema = "{market}_{ktype}".format(market=market, ktype=ktype).lower()
    cur.execute("SELECT 1 FROM information_schema.SCHEMATA where SCHEMA_NAME='{}'".format(schema))
    a = cur.fetchone()
    if not a:
        cur.execute("CREATE SCHEMA `{}`".format(schema))
        connect.commit()

    tablename = code.lower()
    cur.execute("SELECT 1 FROM information_schema.tables "
                "where table_schema='{schema}' and table_name='{name}'"
                .format(schema=schema, name=tablename))
    a = cur.fetchone()
    if not a:
        sql = """
                CREATE TABLE `{schema}`.`{name}` (
                    `date` BIGINT(20) UNSIGNED NOT NULL,
                    `open` DOUBLE UNSIGNED NOT NULL,
                    `high` DOUBLE UNSIGNED NOT NULL,
                    `low` DOUBLE UNSIGNED NOT NULL,
                    `close` DOUBLE UNSIGNED NOT NULL,
                    `amount` DOUBLE UNSIGNED NOT NULL,
                    `count` DOUBLE UNSIGNED NOT NULL,
                    PRIMARY KEY (`date`)
                )
                COLLATE='utf8_general_ci'
                ENGINE=MyISAM
                ;
              """.format(schema=schema, name=tablename)
        cur.execute(sql)
        connect.commit()

    cur.close()
    return "`{schema}`.`{name}`".format(schema=schema, name=tablename)


def get_lastdatetime(connect, tablename):
    cur = connect.cursor()
    cur.execute("select max(date) from {}".format(tablename))
    a = cur.fetchone()
    return a[0]


if __name__ == '__main__':
    host = '127.0.0.1'
    port = 3306
    usr = 'root'
    pwd = ''

    cnx = mysql.connector.connect(user=usr, password=pwd, host=host, port=port)

    create_database(cnx)
    #print(get_codepre_list(cnx, 2, ['stock']))
    #update_last_date(cnx, 1, 20180101)
    #print(get_last_date(cnx, 1))
    #print(get_stock_list(cnx, 'sh', ['stock']))
    #print(get_lastdatetime(cnx, "`hb__min`.`bch_usd`"))
    print(get_table(cnx, 'sh', '000001', 'MIN'))


    from pathlib import Path
    #x = list(Path("./mysql_upgrade").glob("*.sql"))
    #print(x)

    cur = cnx.cursor()
    cur.close()
    cnx.commit()
