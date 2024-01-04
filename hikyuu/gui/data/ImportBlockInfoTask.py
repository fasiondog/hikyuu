#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240105
#    Author: fasiondog

import sqlite3
import mysql.connector
from hikyuu.data.common import MARKET, get_stk_code_name_list
from hikyuu.data.em_block_to_mysql import em_import_block_to_mysql
from hikyuu.data.em_block_to_sqlite import em_import_block_to_sqlite
from hikyuu.util import *


class ImportBlockInfoTask:
    def __init__(self, log_queue, queue, config, categorys):
        self.log_queue = log_queue
        self.queue = queue
        self.config = config
        self.categorys = categorys
        self.task_name = 'IMPORT_BLOCKINFO'
        self.status = "no run"

    @hku_catch(trace=True)
    def __call__(self):
        self.status = "running"
        capture_multiprocess_all_logger(self.log_queue)
        if self.config.getboolean('hdf5', 'enable', fallback=True):
            sqlite_file = "{}/stock.db".format(self.config['hdf5']['dir'])
            connect = sqlite3.connect(sqlite_file, timeout=1800)
            import_block = em_import_block_to_sqlite
        else:
            db_config = {
                'user': self.config['mysql']['usr'],
                'password': self.config['mysql']['pwd'],
                'host': self.config['mysql']['host'],
                'port': self.config['mysql']['port']
            }
            connect = mysql.connector.connect(**db_config)
            import_block = em_import_block_to_mysql

        try:
            code_market_dict = {}
            for market in (MARKET.SH, MARKET.SZ, MARKET.BJ):
                x = get_stk_code_name_list(market)
                for v in x:
                    code_market_dict[v["code"]] = MARKET.SH
            import_block(connect, code_market_dict, self.categorys)
        except Exception as e:
            hku_error(str(e))

        connect.close()

        self.queue.put([self.task_name, "BLOCKINFO", f"{self.categorys}板块信息下载完毕", None, None])
        self.status = "finished"
