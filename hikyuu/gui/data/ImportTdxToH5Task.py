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

import logging
import sqlite3
import mysql.connector
from hikyuu.util.check import hku_catch

from hikyuu.util.mylog import class_logger
from hikyuu.data.tdx_to_h5 import tdx_import_data as h5_import_data
from hikyuu.data.tdx_to_mysql import tdx_import_data as mysql_import_data
from hikyuu.util import capture_multiprocess_all_logger, get_default_logger


class ProgressBar:
    def __init__(self, src):
        self.src = src

    def __call__(self, cur, total):
        self.src.queue.put([self.src.task_name, self.src.market, self.src.ktype, (cur + 1) * 100 // total, 0])


class ImportTdxToH5Task:
    def __init__(self, log_queue, queue, config, market, ktype, quotations, src_dir, dest_dir):
        super(ImportTdxToH5Task, self).__init__()
        self.logger = logging.getLogger(self.__class__.__name__)
        self.task_name = 'IMPORT_KDATA'
        self.log_queue = log_queue
        self.queue = queue
        self.config = config
        self.market = market.upper()
        self.ktype = ktype.upper()
        self.quotations = quotations
        if self.market == 'SH':
            if self.ktype == 'DAY':
                self.src_dir = src_dir + "/vipdoc/sh/lday"
            elif self.ktype == '1MIN':
                self.src_dir = src_dir + "/vipdoc/sh/minline"
            elif self.ktype == '5MIN':
                self.src_dir = src_dir + "/vipdoc/sh/fzline"
        elif self.market == 'SZ':
            if self.ktype == 'DAY':
                self.src_dir = src_dir + "/vipdoc/sz/lday"
            elif self.ktype == '1MIN':
                self.src_dir = src_dir + "/vipdoc/sz/minline"
            elif self.ktype == '5MIN':
                self.src_dir = src_dir + "/vipdoc/sz/fzline"
        elif self.market == 'BJ':
            if self.ktype == 'DAY':
                self.src_dir = src_dir + "/vipdoc/bj/lday"
            elif self.ktype == '1MIN':
                self.src_dir = src_dir + "/vipdoc/bj/minline"
            elif self.ktype == '5MIN':
                self.src_dir = src_dir + "/vipdoc/bj/fzline"
        self.dest_dir = dest_dir
        self.status = "no run"

    def __del__(self):
        # print(self.__class__.__name__, self.market, self.ktype, "__del__")
        pass

    @hku_catch(trace=True)
    def __call__(self):
        self.status = "no run"
        capture_multiprocess_all_logger(self.log_queue)
        use_hdf = False
        if self.config.getboolean('hdf5', 'enable', fallback=True):
            sqlite_file = "{}/stock.db".format(self.config['hdf5']['dir'])
            connect = sqlite3.connect(sqlite_file, timeout=1800)
            import_data = h5_import_data
            self.logger.debug('use hdf5 import kdata')
            use_hdf = True
        else:
            db_config = {
                'user': self.config['mysql']['usr'],
                'password': self.config['mysql']['pwd'],
                'host': self.config['mysql']['host'],
                'port': self.config['mysql']['port']
            }
            connect = mysql.connector.connect(**db_config)
            import_data = mysql_import_data
            self.logger.debug('use mysql import kdata')

        count = 0
        try:
            progress = ProgressBar(self)
            if use_hdf:
                count = import_data(
                    connect, self.market, self.ktype, self.quotations, self.src_dir, self.dest_dir, progress
                )
            else:
                count = import_data(connect, self.market, self.ktype, self.quotations, self.src_dir, progress)

        except Exception as e:
            self.logger.error(e)
        self.queue.put([self.task_name, self.market, self.ktype, None, count])
        self.status = "finished"
