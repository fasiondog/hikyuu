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

import time
import logging
import sqlite3
import mysql.connector
from hikyuu.util.check import hku_catch

from hikyuu.util.mylog import class_logger
from hikyuu.util import capture_multiprocess_all_logger, get_default_logger
from hikyuu.data.common import g_market_list
from hikyuu.data.common_sqlite3 import get_stock_list as sqlite_get_stock_list
from hikyuu.data.common_mysql import get_stock_list as mysql_get_stock_list
from hikyuu import KDataToHdf5Importer, Query, Datetime


def ktype_to_qmt_period(ktype):
    if ktype == 'DAY':
        return '1d'
    elif ktype == '5MIN':
        return '5m'
    elif ktype == '1MIN':
        return '1m'
    else:
        raise ValueError('ktype must be DAY, 1MIN or 5MIN')


class ProgressBar:
    def __init__(self, src, market, ktype):
        self.src = src
        self.market = market
        self.ktype = ktype
        self.finished = False
        self.total = 0
        self.cur = 0

    def __call__(self, data):
        print(data)
        self.total = data['total']
        self.cur = data['finished']
        if self.src.queue:
            self.src.queue.put([self.src.task_name, self.market, self.ktype, (self.cur + 1) * 100 // self.total, 0])
        if self.cur == self.total:
            self.finished = True


class ImportQmtToH5Task:
    def __init__(self, log_queue, queue, config, ktype_list, quotations, dest_dir):
        super(ImportQmtToH5Task, self).__init__()
        self.logger = logging.getLogger(self.__class__.__name__)
        self.task_name = 'IMPORT_KDATA'
        self.log_queue = log_queue
        self.queue = queue
        self.config = config
        self.ktype_list = [ktype.upper() for ktype in ktype_list]
        self.quotations = quotations
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
            get_stock_list = sqlite_get_stock_list
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
            get_stock_list = mysql_get_stock_list
            self.logger.debug('use mysql import kdata')

        from xtquant import xtdata
        total = 0
        for ktype in self.ktype_list:
            for market in g_market_list:
                start_time = time.time()
                process = ProgressBar(self, market, ktype)
                try:
                    code_list = []
                    stock_list = get_stock_list(connect, market, self.quotations)
                    tmp_list = [f'{stock[2]}.{market}' for stock in stock_list]
                    code_list.extend(tmp_list)
                    xtdata.download_history_data2(code_list, period=ktype_to_qmt_period(ktype), start_time='', end_time='',
                                                  callback=process, incrementally=None)
                except Exception as e:
                    self.logger.error(e)

                while not process.finished:
                    time.sleep(0.1)
                    end_time = time.time()
                    if (end_time - start_time) > 30 and process.total == 0:
                        self.logger.error(f'QMT 超时没反应 {market} {ktype}')
                        break

                total += process.total

        if self.queue:
            self.queue.put([self.task_name, 'ALL', self.ktype, None, total])

        self.status = "finished"

        @hku_catch(trace=True)
        def import_qmt_to_h5(market, ktype, code_list, dest_dir):
            today = Datetime.today()
            im = KDataToHdf5Importer()
            if not im.set_config(dest_dir, market):
                self.logger.error('KDataToHdf5Importer set config error! Maybe no license!')
                return

            count_per_day = 0
            if ktype == '1MIN':
                nktype = Query.MIN
                count_per_day = 240
            elif ktype == '5MIN':
                nktype = Query.MIN5
                count_per_day = 48
            elif ktype == 'DAY':
                nktype = Query.DAY
                count_per_day = 1
            else:
                self.logger.error(f'Error ktype! {ktype}')
                return

            from xtquant import xtdata
            for code in code_list:
                last_date = im.get_last_datetime(market, code, nktype)
                delta = today - last_date.start_of_day()
                if delta.days <= 0:
                    continue
                count = delta.days * count_per_day
                # get_market_data(field_list=[], stock_list=[], period='1d', start_time='', end_time='', count=-1, dividend_type='none', fill_data=True)
                xtdata.get_market_data(field_list=['time', 'open', 'high', 'low', 'close', 'amount', 'volume'], stock_list=[f'{code}.{market}'], period='1d', start_time='',
                                       end_time='', count=count, dividend_type='none', fill_data=False)


if __name__ == '__main__':
    from configparser import ConfigParser
    import os

    filename = os.path.expanduser('~') + '/.hikyuu' + '/importdata-gui.ini'
    import_config = ConfigParser()
    import_config.read(filename, encoding='utf-8')

    from xtquant import xtdata
    # get_default_logger()

    # self, log_queue, queue, config, ktype, quotations, dest_dir
    task = ImportQmtToH5Task(None, None, import_config, 'day', ['stock', 'fund'], 'd:\\tmp\\stock')
    task()
