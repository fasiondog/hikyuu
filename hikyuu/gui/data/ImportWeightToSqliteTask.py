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
import logging
import hashlib
import sqlite3
import urllib.request
import mysql.connector

from pytdx.hq import TdxHq_API
from hikyuu.data.common import MARKET, g_market_list
from hikyuu.data.common_pytdx import search_best_tdx
from hikyuu.data.weight_to_sqlite import qianlong_import_weight
from hikyuu.data.pytdx_weight_to_sqlite import pytdx_import_weight_to_sqlite
from hikyuu.data.pytdx_weight_to_mysql import pytdx_import_weight_to_mysql
from hikyuu.data.pytdx_finance_to_sqlite import pytdx_import_finance_to_sqlite
from hikyuu.data.pytdx_finance_to_mysql import pytdx_import_finance_to_mysql
from hikyuu.util import capture_multiprocess_all_logger, get_default_logger
from hikyuu.util.check import hku_catch, hku_check


class ImportWeightToSqliteTask:
    def __init__(self, log_queue, queue, config, dest_dir, market, cmd, host, port):
        self.logger = logging.getLogger(self.__class__.__name__)
        self.log_queue = log_queue
        self.queue = queue
        self.config = config
        self.dest_dir = dest_dir
        self.msg_name = 'IMPORT_WEIGHT'
        self.status = "no run"
        self.market = market
        self.cmd = cmd  # "weight" | "finance"
        self.host = host
        self.port = port

    @hku_catch(trace=True)
    def __call__(self):
        self.status = "running"
        capture_multiprocess_all_logger(self.log_queue)
        total_count = 0
        try:
            if self.config.getboolean('hdf5', 'enable', fallback=True):
                sqlite_file = "{}/stock.db".format(self.config['hdf5']['dir'])
                connect = sqlite3.connect(sqlite_file, timeout=1800)
                pytdx_import_weight = pytdx_import_weight_to_sqlite
                pytdx_import_finance = pytdx_import_finance_to_sqlite
                self.logger.debug('use sqlite import weight')
            else:
                db_config = {
                    'user': self.config['mysql']['usr'],
                    'password': self.config['mysql']['pwd'],
                    'host': self.config['mysql']['host'],
                    'port': self.config['mysql']['port']
                }
                connect = mysql.connector.connect(**db_config)
                pytdx_import_weight = pytdx_import_weight_to_mysql
                pytdx_import_finance = pytdx_import_finance_to_mysql
                self.logger.debug('use mysql import weight')

        except Exception as e:
            # self.queue.put([self.msg_name, str(e), -1, 0, total_count])
            self.queue.put([self.msg_name, 'INFO', str(e), 0, 0])
            self.queue.put([self.msg_name, '', 0, None, total_count])
            self.status = "failure"
            return

        try:
            api = TdxHq_API()
            hku_check(api.connect(self.host, self.port), "failed connect pytdx {}:{}!", self.host, self.port)

            if self.cmd == 'weight':
                total_count = pytdx_import_weight(api, connect, self.market)
                self.logger.info("导入 {} 权息记录数: {}".format(self.market, total_count))
                self.queue.put([self.msg_name, '导入权息数据完毕!', 0, 0, f'{self.market} {total_count}'])
            elif self.cmd == 'finance':
                self.queue.put([self.msg_name, f'下载通达信当前财务信息({self.market})...', 0, 0, 0])
                x = pytdx_import_finance(connect, api, self.market)
                self.logger.info(f'导入 {self.market} 通达信当前财务信息数: {x}')
                self.queue.put([self.msg_name, '导入通达信财务信息完毕!', 0, 0, f'{self.market} {x}'])

            api.disconnect()

        except Exception as e:
            self.logger.error(e)
            # self.queue.put([self.msg_name, str(e), -1, 0, total_count])
            self.queue.put([self.msg_name, 'INFO', str(e), 0, 0])
        finally:
            connect.commit()
            connect.close()

        self.queue.put([self.msg_name, '', 0, None, total_count])
        self.status = "finished"
