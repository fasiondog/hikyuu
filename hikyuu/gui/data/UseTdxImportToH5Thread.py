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
from multiprocessing import Queue, Process
from PyQt5.QtCore import QThread, pyqtSignal
from hikyuu.gui.data.ImportTdxToH5Task import ImportTdxToH5Task
from hikyuu.gui.data.ImportWeightToSqliteTask import ImportWeightToSqliteTask
from hikyuu.gui.data.ImportHistoryFinanceTask import ImportHistoryFinanceTask

from hikyuu.data.common_sqlite3 import create_database
from hikyuu.data.tdx_to_h5 import tdx_import_stock_name_from_file
from hikyuu.util import *


class UseTdxImportToH5Thread(QThread):
    message = pyqtSignal(list)

    def __init__(self, parent, config):
        super(UseTdxImportToH5Thread, self).__init__()
        self.logger = logging.getLogger(self.__class__.__name__)
        self.parent = parent
        self.log_queue = parent.mp_log_q if parent is not None else None
        self.config = config
        self.msg_name = 'HDF5_IMPORT'

        self.process_list = []

        src_dir = config['tdx']['dir']
        dest_dir = config['hdf5']['dir']
        sqlite_file_name = dest_dir + "/stock.db"

        self.quotations = []
        if self.config['quotation']['stock']:
            self.quotations.append('stock')
        if self.config['quotation']['fund']:
            self.quotations.append('fund')
        #if self.config['quotation']['future']:
        #    self.quotations.append('future')

        #通达信盘后没有债券数据。另外，如果用Pytdx下载债券数据，
        #每个债券本身的数据很少但债券种类太多占用空间和时间太多，用途较少不再考虑导入
        #if self.config['quotation']['bond']:
        #    self.quotations.append('bond')

        self.queue = Queue()
        self.tasks = []
        if self.config.getboolean('weight', 'enable', fallback=False):
            self.tasks.append(ImportWeightToSqliteTask(self.log_queue, self.queue, self.config, dest_dir))
        if self.config.getboolean('finance', 'enable', fallback=True):
            self.tasks.append(ImportHistoryFinanceTask(self.log_queue, self.queue, self.config, dest_dir))
        if self.config.getboolean('ktype', 'day', fallback=False):
            self.tasks.append(
                ImportTdxToH5Task(self.log_queue, self.queue, config, 'SH', 'DAY', self.quotations, src_dir, dest_dir)
            )
            self.tasks.append(
                ImportTdxToH5Task(self.log_queue, self.queue, config, 'SZ', 'DAY', self.quotations, src_dir, dest_dir)
            )
        if self.config.getboolean('ktype', 'min5', fallback=False):
            self.tasks.append(
                ImportTdxToH5Task(self.log_queue, self.queue, config, 'SH', '5MIN', self.quotations, src_dir, dest_dir)
            )
            self.tasks.append(
                ImportTdxToH5Task(self.log_queue, self.queue, config, 'SZ', '5MIN', self.quotations, src_dir, dest_dir)
            )
        if self.config.getboolean('ktype', 'min', fallback=False):
            self.tasks.append(
                ImportTdxToH5Task(self.log_queue, self.queue, config, 'SH', '1MIN', self.quotations, src_dir, dest_dir)
            )
            self.tasks.append(
                ImportTdxToH5Task(self.log_queue, self.queue, config, 'SZ', '1MIN', self.quotations, src_dir, dest_dir)
            )

    def __del__(self):
        for p in self.process_list:
            if p.is_alive():
                p.terminate()

    def send_message(self, msg):
        self.message.emit([self.msg_name] + msg)

    def run(self):
        try:
            self._run()
        except Exception as e:
            self.send_message(['THREAD', 'FAILURE', str(e)])
        else:
            self.send_message(['THREAD', 'FINISHED'])

    @hku_catch(trace=True, re_raise=True)
    def _run(self):
        src_dir = self.config['tdx']['dir']
        dest_dir = self.config['hdf5']['dir']
        hdf5_import_progress = {'SH': {'DAY': 0, '1MIN': 0, '5MIN': 0}, 'SZ': {'DAY': 0, '1MIN': 0, '5MIN': 0}}

        #正在导入代码表
        self.send_message(['START_IMPORT_CODE'])

        connect = sqlite3.connect(dest_dir + "/stock.db")
        create_database(connect)

        tdx_import_stock_name_from_file(connect, src_dir + "\\T0002\\hq_cache\\shm.tnf", 'SH', self.quotations)
        tdx_import_stock_name_from_file(connect, src_dir + "\\T0002\\hq_cache\\szm.tnf", 'SZ', self.quotations)

        self.send_message(['FINISHED_IMPORT_CODE'])

        self.process_list.clear()
        for task in self.tasks:
            p = Process(target=task)
            self.process_list.append(p)
            p.start()

        finished_count = len(self.tasks)
        while finished_count > 0:
            message = self.queue.get()
            taskname, market, ktype, progress, total = message
            if progress is None:
                finished_count -= 1
                if taskname == 'IMPORT_KDATA':
                    self.send_message(['IMPORT_KDATA', 'FINISHED', market, ktype, total])
                else:
                    self.send_message([taskname, 'FINISHED'])
                continue

            if taskname == 'IMPORT_WEIGHT':
                if market == 'INFO':
                    self.send_message(['INFO', ktype])
                self.send_message(['IMPORT_WEIGHT', market, total])
            elif taskname == 'IMPORT_KDATA':
                hdf5_import_progress[market][ktype] = progress
                current_progress = (hdf5_import_progress['SH'][ktype] + hdf5_import_progress['SZ'][ktype]) // 2
                self.send_message(['IMPORT_KDATA', ktype, current_progress])
            elif taskname == 'IMPORT_FINANCE':
                self.send_message([taskname, progress])
            else:
                self.logger.error("Unknow task: {}".format(taskname))
