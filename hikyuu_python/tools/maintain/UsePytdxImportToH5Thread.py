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

import sqlite3
from multiprocessing import Queue, Process
from PyQt5.QtCore import QThread, pyqtSignal
from ImportWeightToSqliteTask import ImportWeightToSqliteTask
from ImportPytdxToH5Task import ImportPytdxToH5
from pytdx.hq import TdxHq_API
from sqlite3_common import create_database
from pytdx_to_h5 import import_stock_name

class UsePytdxImportToH5Thread(QThread):
    message = pyqtSignal(list)

    def __init__(self, config):
        super(self.__class__, self).__init__()
        self.config = config
        self.msg_name = 'HDF5_IMPORT'

        if not self.check():
            return

        self.process_list = []

        dest_dir = config['hdf5']['dir']
        sqlite_file_name = dest_dir + "/stock.db"

        self.quotations = []
        if self.config['quotation']['stock']:
            self.quotations.append('stock')
        if self.config['quotation']['fund']:
            self.quotations.append('fund')
        #if self.config['quotation']['future']:
        #    self.quotations.append('future')

        self.ip = self.config['pytdx']['ip']
        self.port = int(self.config['pytdx']['port'])

        self.queue = Queue()
        self.tasks = []
        if self.config.getboolean('weight', 'enable', fallback=False):
            self.tasks.append(ImportWeightToSqliteTask(self.queue, sqlite_file_name, dest_dir))
        if self.config.getboolean('ktype', 'day', fallback=False):
            self.tasks.append(ImportPytdxToH5(self.queue, sqlite_file_name, 'SH', 'DAY',
                                              self.quotations, self.ip, self.port, dest_dir))
            self.tasks.append(ImportPytdxToH5(self.queue, sqlite_file_name, 'SZ', 'DAY',
                                              self.quotations, self.ip, self.port, dest_dir))
        if self.config.getboolean('ktype', 'min5', fallback=False):
            self.tasks.append(ImportPytdxToH5(self.queue, sqlite_file_name, 'SH', '5MIN',
                                              self.quotations, self.ip, self.port, dest_dir))
            self.tasks.append(ImportPytdxToH5(self.queue, sqlite_file_name, 'SZ', '5MIN',
                                              self.quotations, self.ip, self.port, dest_dir))
        if self.config.getboolean('ktype', 'min', fallback=False):
            self.tasks.append(ImportPytdxToH5(self.queue, sqlite_file_name, 'SH', '1MIN',
                                              self.quotations, self.ip, self.port, dest_dir))
            self.tasks.append(ImportPytdxToH5(self.queue, sqlite_file_name, 'SZ', '1MIN',
                                              self.quotations, self.ip, self.port, dest_dir))


    def check(self):
        return True


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

    def _run(self):
        if not self.check():
            print("存在错误！")
            return

        src_dir = self.config['tdx']['dir']
        dest_dir = self.config['hdf5']['dir']
        hdf5_import_progress = {'SH': {'DAY': 0, '1MIN': 0, '5MIN': 0},
                                'SZ': {'DAY': 0, '1MIN': 0, '5MIN': 0}}

        #正在导入代码表
        self.send_message(['START_IMPORT_CODE'])

        connect = sqlite3.connect(dest_dir + "/stock.db")
        create_database(connect)

        pytdx_api = TdxHq_API()
        pytdx_api.connect(self.ip, self.port)

        import_stock_name(connect, pytdx_api, 'SH', self.quotations)
        import_stock_name(connect, pytdx_api, 'SZ', self.quotations)

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
                self.send_message(['IMPORT_WEIGHT', market, total])
            elif taskname == 'IMPORT_KDATA':
                hdf5_import_progress[market][ktype] = progress
                current_progress = (hdf5_import_progress['SH'][ktype] + hdf5_import_progress['SZ'][ktype]) // 2
                self.send_message(['IMPORT_KDATA', ktype, current_progress])
            else:
                print("Unknow task: ", taskname)