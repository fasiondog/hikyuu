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
from pytdx.hq import TdxHq_API
from hikyuu.data.pytdx_to_h5 import import_data as h5_import_data
from hikyuu.data.pytdx_to_mysql import import_data as mysql_import_data
from hikyuu.util import *


class ProgressBar:
    def __init__(self, src):
        self.src = src

    def __call__(self, cur, total):
        progress = (cur + 1) * 100 // total
        # hku_info(f"{self.src.market} {self.src.ktype} 数据: {progress}%")
        self.src.queue.put([self.src.task_name, self.src.market, self.src.ktype, progress, 0])


class ImportPytdxToH5:
    def __init__(self, log_queue, queue, config, market, ktype, quotations, ip, port, dest_dir, start_datetime):
        self.logger = logging.getLogger(self.__class__.__name__)
        self.task_name = 'IMPORT_KDATA'
        self.queue = queue
        self.log_queue = log_queue
        self.config = config
        self.market = market
        self.ktype = ktype
        self.quotations = quotations
        self.ip = ip
        self.port = port
        self.dest_dir = dest_dir
        self.startDatetime = start_datetime
        self.status = "no run"

    @hku_catch(trace=True)
    def __call__(self):
        self.status = "running"
        capture_multiprocess_all_logger(self.log_queue)
        if self.config.getboolean('hdf5', 'enable', fallback=True):
            sqlite_file = "{}/stock.db".format(self.config['hdf5']['dir'])
            connect = sqlite3.connect(sqlite_file, timeout=1800)
            import_data = h5_import_data
            self.logger.debug('use hdf5 import kdata')
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
            api = TdxHq_API()
            hku_info("导入 {} {} 数据 from {}", self.market, self.ktype, self.ip)
            hku_check(api.connect(self.ip, self.port), "failed connect pytdx: {}:{}", self.ip, self.port)
            count = import_data(
                connect, self.market, self.ktype, self.quotations, api, self.dest_dir, self.startDatetime, progress
            )
            self.logger.info("导入 {} {} 记录数: {}".format(self.market, self.ktype, count))
        except Exception as e:
            self.logger.error("ImportPytdxToH5Task failed! {}".format(e))
            # self.queue.put([self.task_name, self.market, self.ktype, str(e), count])
        finally:
            connect.commit()
            connect.close()

        self.queue.put([self.task_name, self.market, self.ktype, None, count])
        self.status = "finished"
