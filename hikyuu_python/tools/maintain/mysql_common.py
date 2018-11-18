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
from mysql.connector import errorcode

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
            print(x.statement)
            pass

    connect.commit()
    cur.close()


if __name__ == '__main__':
    host = '127.0.0.1'
    port = 3306
    usr = 'root'
    pwd = '*Btc_2018_?&'

    cnx = mysql.connector.connect(user=usr, password=pwd, host=host, port=port)
    print(get_db_version(cnx))

    create_database(cnx)

    from pathlib import Path
    #x = list(Path("./mysql_upgrade").glob("*.sql"))
    #print(x)

    cur = cnx.cursor()
    cur.close()
    cnx.commit()
