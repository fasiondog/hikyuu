#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240105
#    Author: fasiondog

import sqlite3
import mysql.connector
from hikyuu.data.em_block_to_mysql import em_import_block_to_mysql
from hikyuu.data.em_block_to_sqlite import em_import_block_to_sqlite
from hikyuu.data.em_block_to_clickhouse import em_import_block_to_clickhouse
from hikyuu.util import *


class ImportBlockInfoTask:
    def __init__(self, log_queue, queue, config):
        self.log_queue = log_queue
        self.queue = queue
        self.config = config
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
        elif self.config.getboolean('mysql', 'enable', fallback=True):
            db_config = {
                'user': self.config['mysql']['usr'],
                'password': self.config['mysql']['pwd'],
                'host': self.config['mysql']['host'],
                'port': self.config['mysql']['port']
            }
            connect = mysql.connector.connect(**db_config)
            import_block = em_import_block_to_mysql
        elif self.config.getboolean('clickhouse', 'enable', fallback=True):
            db_config = {
                'username': self.config['clickhouse']['usr'],
                'password': self.config['clickhouse']['pwd'],
                'host': self.config['clickhouse']['host'],
                'port': self.config['clickhouse']['http_port']
            }
            import clickhouse_connect
            connect = clickhouse_connect.get_client(**db_config)
            import_block = em_import_block_to_clickhouse

        count = 0
        try:
            count = import_block(connect)
        except Exception as e:
            hku_error(str(e))

        connect.close()

        self.queue.put([self.task_name, "BLOCKINFO", f"板块信息更新完毕: {count}", None, None])
        self.status = "finished"
