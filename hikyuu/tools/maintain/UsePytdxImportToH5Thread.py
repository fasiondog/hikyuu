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
import datetime
from multiprocessing import Queue, Process
from PyQt5.QtCore import QThread, pyqtSignal
from hikyuu.tools.maintain.ImportWeightToSqliteTask import ImportWeightToSqliteTask
from hikyuu.tools.maintain.ImportPytdxToH5Task import ImportPytdxToH5
from hikyuu.tools.maintain.ImportPytdxTransToH5Task import ImportPytdxTransToH5
from hikyuu.tools.maintain.ImportPytdxTimeToH5Task import ImportPytdxTimeToH5
from pytdx.hq import TdxHq_API
from hikyuu.data.common_sqlite3 import create_database
from hikyuu.data.pytdx_to_h5 import import_stock_name
from hikyuu.data.common_pytdx import search_best_tdx

class UsePytdxImportToH5Thread(QThread):
    message = pyqtSignal(list)

    def __init__(self, config):
        super(self.__class__, self).__init__()
        self.config = config
        self.msg_name = 'HDF5_IMPORT'

        self.process_list = []
        self.hosts = []
        self.tasks = []

        self.quotations = []
        if self.config['quotation']['stock']:
            self.quotations.append('stock')
        if self.config['quotation']['fund']:
            self.quotations.append('fund')

        self.queue = Queue()

    def __del__(self):
        for p in self.process_list:
            if p.is_alive():
                p.terminate()

    def send_message(self, msg):
        self.message.emit([self.msg_name] + msg)

    def init_task(self):
        config = self.config
        dest_dir = config['hdf5']['dir']
        sqlite_file_name = dest_dir + "/stock.db"

        self.tasks = []
        if self.config.getboolean('weight', 'enable', fallback=False):
            self.tasks.append(ImportWeightToSqliteTask(self.queue, sqlite_file_name, dest_dir))

        task_count = 0
        if self.config.getboolean('ktype', 'day', fallback=False):
            task_count += 2
        if self.config.getboolean('ktype', 'min5', fallback=False):
            task_count += 2
        if self.config.getboolean('ktype', 'min', fallback=False):
            task_count += 2
        if self.config.getboolean('ktype', 'trans', fallback=False):
            task_count += 2
        if self.config.getboolean('ktype', 'time', fallback=False):
            task_count += 2

        self.send_message(['INFO', '搜索通达信服务器'])
        self.hosts = search_best_tdx()
        if not self.hosts:
            self.send_message(['INFO', '无法连接通达信行情服务器！请检查网络设置！'])
            return

        if task_count == 0:
            return

        use_tdx_number = min(task_count, len(self.hosts), self.config.getint('pytdx', 'use_tdx_number', fallback=10))
        split = task_count // use_tdx_number
        use_hosts = []
        for i in range(use_tdx_number):
            for j in range(split):
                use_hosts.append((self.hosts[i][2], self.hosts[i][3]))
        if len(use_hosts) < task_count:
            for i in range(task_count - len(use_hosts)):
                use_hosts.insert(0, (self.hosts[0][2], self.hosts[0][3]))
        # for i in range(len(use_hosts)):
        #     print(i, use_hosts[i])

        cur_host = 0

        # 以下按数据量从大到小依次使用速度从高到低的TDX服务器
        if self.config.getboolean('ktype', 'trans', fallback=False):
            today = datetime.date.today()
            trans_start_date = datetime.datetime.strptime(config['ktype']['trans_start_date'], '%Y-%m-%d').date()
            trans_max_days = (today - trans_start_date).days + 1
            self.tasks.append(
                ImportPytdxTransToH5(
                    self.queue, sqlite_file_name, 'SH', self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir, trans_max_days
                )
            )
            cur_host += 1
            self.tasks.append(
                ImportPytdxTransToH5(
                    self.queue, sqlite_file_name, 'SZ', self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir, trans_max_days
                )
            )
            cur_host += 1

        if self.config.getboolean('ktype', 'min', fallback=False):
            start_date = datetime.datetime.strptime(config['ktype']['min_start_date'], '%Y-%m-%d').date()
            self.tasks.append(
                ImportPytdxToH5(
                    self.queue, sqlite_file_name, 'SH', '1MIN', self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir, start_date.year * 100000000 + start_date.month * 1000000 + start_date.day * 10000
                )
            )
            cur_host += 1
            self.tasks.append(
                ImportPytdxToH5(
                    self.queue, sqlite_file_name,
                    'SZ', '1MIN', self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir, start_date.year * 100000000 + start_date.month * 1000000 + start_date.day * 10000
                )
            )
            cur_host += 1

        if self.config.getboolean('ktype', 'time', fallback=False):
            today = datetime.date.today()
            time_start_date = datetime.datetime.strptime(config['ktype']['time_start_date'], '%Y-%m-%d').date()
            time_max_days = (today - time_start_date).days + 1
            self.tasks.append(ImportPytdxTimeToH5(self.queue, sqlite_file_name, 'SH',
                                                  self.quotations,
                                                  use_hosts[cur_host][0], use_hosts[cur_host][1],
                                                  dest_dir,
                                                  time_max_days))
            cur_host += 1
            self.tasks.append(ImportPytdxTimeToH5(self.queue, sqlite_file_name, 'SZ',
                                                  self.quotations,
                                                  use_hosts[cur_host][0], use_hosts[cur_host][1],
                                                  dest_dir,
                                                  time_max_days))
            cur_host += 1

        if self.config.getboolean('ktype', 'min5', fallback=False):
            start_date = datetime.datetime.strptime(config['ktype']['min5_start_date'], '%Y-%m-%d').date()
            self.tasks.append(
                ImportPytdxToH5(
                    self.queue, sqlite_file_name, 'SH', '5MIN',
                    self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir,
                    start_date.year * 100000000 + start_date.month * 1000000 + start_date.day * 10000
                )
            )
            cur_host += 1
            self.tasks.append(
                ImportPytdxToH5(
                    self.queue, sqlite_file_name, 'SZ', '5MIN',
                    self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir,
                    start_date.year * 100000000 + start_date.month * 1000000 + start_date.day * 10000
                )
            )
            cur_host += 1

        if self.config.getboolean('ktype', 'day', fallback=False):
            start_date = datetime.datetime.strptime(config['ktype']['day_start_date'], '%Y-%m-%d').date()
            self.tasks.append(
                ImportPytdxToH5(
                    self.queue, sqlite_file_name, 'SH', 'DAY',
                    self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir,
                    start_date.year * 100000000 + start_date.month * 1000000 + start_date.day * 10000
                )
            )
            cur_host += 1
            self.tasks.append(
                ImportPytdxToH5(
                    self.queue, sqlite_file_name, 'SZ', 'DAY',
                    self.quotations,
                    use_hosts[cur_host][0], use_hosts[cur_host][1],
                    dest_dir,
                    start_date.year * 100000000 + start_date.month * 1000000 + start_date.day * 10000
                )
            )
            cur_host += 1

    def run(self):
        try:
            self.init_task()
            self._run()
        except Exception as e:
            self.send_message(['THREAD', 'FAILURE', str(e)])
        else:
            self.send_message(['THREAD', 'FINISHED'])

    def _run(self):
        src_dir = self.config['tdx']['dir']
        dest_dir = self.config['hdf5']['dir']
        hdf5_import_progress = {'SH': {'DAY': 0, '1MIN': 0, '5MIN': 0},
                                'SZ': {'DAY': 0, '1MIN': 0, '5MIN': 0}}
        trans_progress = {'SH': 0, 'SZ': 0}
        time_progress = {'SH': 0, 'SZ': 0}

        #正在导入代码表
        self.send_message(['INFO', '导入股票代码表'])

        connect = sqlite3.connect(dest_dir + "/stock.db")
        create_database(connect)

        pytdx_api = TdxHq_API()
        pytdx_api.connect(self.hosts[0][2], self.hosts[0][3])

        count = import_stock_name(connect, pytdx_api, 'SH', self.quotations)
        if count > 0:
            self.send_message(['INFO', '上证新增股票数：%s' % count])
        count = import_stock_name(connect, pytdx_api, 'SZ', self.quotations)
        if count > 0:
            self.send_message(['INFO', '深证新增股票数：%s' % count])

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
                if taskname in ('IMPORT_KDATA', 'IMPORT_TRANS', 'IMPORT_TIME'):
                    self.send_message([taskname, 'FINISHED', market, ktype, total])
                else:
                    self.send_message([taskname, 'FINISHED'])
                continue

            if taskname == 'IMPORT_WEIGHT':
                if market == 'INFO':
                    self.send_message(['INFO', ktype])
                self.send_message([taskname, market, total])
            elif taskname == 'IMPORT_KDATA':
                hdf5_import_progress[market][ktype] = progress
                current_progress = (hdf5_import_progress['SH'][ktype] + hdf5_import_progress['SZ'][ktype]) // 2
                self.send_message([taskname, ktype, current_progress])
            elif taskname == 'IMPORT_TRANS':
                trans_progress[market] = progress
                current_progress = (trans_progress['SH'] + trans_progress['SZ']) // 2
                self.send_message([taskname, ktype, current_progress])
            elif taskname == 'IMPORT_TIME':
                time_progress[market] = progress
                current_progress = (time_progress['SH'] + time_progress['SZ']) // 2
                self.send_message([taskname, ktype, current_progress])
            else:
                print("Unknow task: ", taskname)