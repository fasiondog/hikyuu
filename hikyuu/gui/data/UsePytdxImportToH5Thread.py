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
import datetime
import mysql.connector
import queue
from multiprocessing import Queue, Process
from PyQt5.QtCore import QThread, pyqtSignal
from hikyuu.gui.data.ImportWeightToSqliteTask import ImportWeightToSqliteTask
from hikyuu.gui.data.ImportPytdxToH5Task import ImportPytdxToH5
from hikyuu.gui.data.ImportPytdxTransToH5Task import ImportPytdxTransToH5
from hikyuu.gui.data.ImportPytdxTimeToH5Task import ImportPytdxTimeToH5
from hikyuu.gui.data.ImportHistoryFinanceTask import ImportHistoryFinanceTask
from hikyuu.gui.data.ImportBlockInfoTask import ImportBlockInfoTask
from hikyuu.gui.data.ImportZhBond10Task import ImportZhBond10Task
from pytdx.hq import TdxHq_API
from hikyuu.data.common_pytdx import search_best_tdx

from hikyuu.data.common import *
from hikyuu.data.common_sqlite3 import import_new_holidays as sqlite_import_new_holidays
from hikyuu.data.common_sqlite3 import create_database as sqlite_create_database
from hikyuu.data.pytdx_to_h5 import import_index_name as sqlite_import_index_name
from hikyuu.data.pytdx_to_h5 import import_stock_name as sqlite_import_stock_name
from hikyuu.data.common_mysql import create_database as mysql_create_database
from hikyuu.data.common_mysql import import_new_holidays as mysql_import_new_holidays
from hikyuu.data.pytdx_to_mysql import import_index_name as mysql_import_index_name
from hikyuu.data.pytdx_to_mysql import import_stock_name as mysql_import_stock_name
from hikyuu.util.mylog import class_logger


class UsePytdxImportToH5Thread(QThread):
    message = pyqtSignal(list)

    def __init__(self, parent, config):
        super(UsePytdxImportToH5Thread, self).__init__()
        self.parent = parent
        self.log_queue = parent.mp_log_q if parent is not None else None
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

        self.tasks = []
        if self.config.getboolean('finance', 'enable', fallback=True):
            self.tasks.append(
                ImportHistoryFinanceTask(self.log_queue, self.queue, self.config, dest_dir))

        self.tasks.append(ImportBlockInfoTask(self.log_queue, self.queue,
                          self.config, ('行业板块', '概念板块', '地域板块', '指数板块')))
        self.tasks.append(ImportZhBond10Task(self.log_queue, self.queue, self.config))

        task_count = 0
        market_count = len(g_market_list)
        if self.config.getboolean('ktype', 'day', fallback=False):
            task_count += market_count
        if self.config.getboolean('ktype', 'min5', fallback=False):
            task_count += market_count
        if self.config.getboolean('ktype', 'min', fallback=False):
            task_count += market_count
        if self.config.getboolean('ktype', 'trans', fallback=False):
            task_count += market_count
        if self.config.getboolean('ktype', 'time', fallback=False):
            task_count += market_count
        if self.config.getboolean('weight', 'enable', fallback=False):
            task_count += (market_count*2)

        self.logger.info('搜索通达信服务器')
        self.send_message(['INFO', '搜索通达信服务器'])
        self.hosts = search_best_tdx()
        if not self.hosts:
            self.logger.warn('无法连接通达信行情服务器！请检查网络设置！')
            self.send_message(['INFO', '无法连接通达信行情服务器！请检查网络设置！'])
            return

        if task_count == 0:
            self.send_message(['INFO', '未选择需要导入的行情数据！'])
            return

        use_tdx_number = min(
            task_count, len(self.hosts),
            self.config.getint('pytdx', 'use_tdx_number', fallback=10))
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
            trans_start_date = datetime.datetime.strptime(
                config['ktype']['trans_start_date'], '%Y-%m-%d').date()
            trans_max_days = (today - trans_start_date).days + 1
            for market in g_market_list:
                self.tasks.append(
                    ImportPytdxTransToH5(self.log_queue, self.queue,
                                         self.config, market, self.quotations,
                                         use_hosts[cur_host][0],
                                         use_hosts[cur_host][1], dest_dir,
                                         trans_max_days))
                cur_host += 1

        if self.config.getboolean('ktype', 'min', fallback=False):
            start_date = datetime.datetime.strptime(
                config['ktype']['min_start_date'], '%Y-%m-%d').date()
            for market in g_market_list:
                self.tasks.append(
                    ImportPytdxToH5(
                        self.log_queue, self.queue, self.config, market,
                        '1MIN', self.quotations, use_hosts[cur_host][0],
                        use_hosts[cur_host][1], dest_dir,
                        start_date.year * 100000000 +
                        start_date.month * 1000000 + start_date.day * 10000))
                cur_host += 1

        if self.config.getboolean('ktype', 'time', fallback=False):
            today = datetime.date.today()
            time_start_date = datetime.datetime.strptime(
                config['ktype']['time_start_date'], '%Y-%m-%d').date()
            time_max_days = (today - time_start_date).days + 1
            for market in g_market_list:
                self.tasks.append(
                    ImportPytdxTimeToH5(self.log_queue, self.queue,
                                        self.config, market,
                                        self.quotations,
                                        use_hosts[cur_host][0],
                                        use_hosts[cur_host][1], dest_dir,
                                        time_max_days))
                cur_host += 1

        if self.config.getboolean('ktype', 'min5', fallback=False):
            start_date = datetime.datetime.strptime(
                config['ktype']['min5_start_date'], '%Y-%m-%d').date()
            for market in g_market_list:
                self.tasks.append(
                    ImportPytdxToH5(
                        self.log_queue, self.queue, self.config, market,
                        '5MIN', self.quotations, use_hosts[cur_host][0],
                        use_hosts[cur_host][1], dest_dir,
                        start_date.year * 100000000 +
                        start_date.month * 1000000 + start_date.day * 10000))
                cur_host += 1

        if self.config.getboolean('ktype', 'day', fallback=False):
            start_date = datetime.datetime.strptime(
                config['ktype']['day_start_date'], '%Y-%m-%d').date()
            for market in g_market_list:
                self.tasks.append(
                    ImportPytdxToH5(
                        self.log_queue, self.queue, self.config, market, 'DAY',
                        self.quotations, use_hosts[cur_host][0],
                        use_hosts[cur_host][1], dest_dir,
                        start_date.year * 100000000 +
                        start_date.month * 1000000 + start_date.day * 10000))
                cur_host += 1

        if self.config.getboolean('weight', 'enable', fallback=False):
            for market in g_market_list:
                self.tasks.append(
                    ImportWeightToSqliteTask(self.log_queue, self.queue,
                                             self.config, dest_dir, market, 'weight', use_hosts[cur_host][0],
                                             use_hosts[cur_host][1]))
                cur_host += 1
                self.tasks.append(
                    ImportWeightToSqliteTask(self.log_queue, self.queue,
                                             self.config, dest_dir, market, 'finance', use_hosts[cur_host][0],
                                             use_hosts[cur_host][1]))
                cur_host += 1

    def run(self):
        try:
            self.init_task()
            self._run()
        except Exception as e:
            self.logger.error(str(e))
            self.send_message(['THREAD', 'FAILURE', str(e)])
        else:
            self.logger.info('导入完毕')
            self.send_message(['THREAD', 'FINISHED'])

    @hku_catch(trace=True, re_raise=True)
    def _run(self):
        hdf5_import_progress = {}
        trans_progress = {}
        time_progress = {}
        for market in g_market_list:
            hdf5_import_progress[market] = {'DAY': 0, '1MIN': 0, '5MIN': 0}
            trans_progress[market] = 0
            time_progress[market] = 0

        # 正在导入代码表
        self.logger.info('导入股票代码表')
        self.send_message(['INFO', '导入股票代码表'])

        if self.config.getboolean('hdf5', 'enable', fallback=True):
            connect = sqlite3.connect("{}/stock.db".format(
                self.config['hdf5']['dir']))
            create_database = sqlite_create_database
            import_new_holidays = sqlite_import_new_holidays
            import_index_name = sqlite_import_index_name
            import_stock_name = sqlite_import_stock_name
        else:
            db_config = {
                'user': self.config['mysql']['usr'],
                'password': self.config['mysql']['pwd'],
                'host': self.config['mysql']['host'],
                'port': self.config['mysql']['port']
            }
            connect = mysql.connector.connect(**db_config)
            create_database = mysql_create_database
            import_new_holidays = mysql_import_new_holidays
            import_index_name = mysql_import_index_name
            import_stock_name = mysql_import_stock_name

        create_database(connect)

        pytdx_api = TdxHq_API()
        hku_check(pytdx_api.connect(self.hosts[0][2], self.hosts[0][3]),
                  "failed connect pytdx {}:{}", self.hosts[0][2],
                  self.hosts[0][3])

        self.logger.info("导入交易所休假日历")
        import_new_holidays(connect)

        count = import_index_name(connect)
        self.logger.info("指数数量: {}".format(count))

        for market in g_market_list:
            count = import_stock_name(connect, pytdx_api, market,
                                      self.quotations)
            if count > 0:
                self.logger.info("{} 新增股票数: {}".format(market, count))
                self.send_message(
                    ['INFO', '{} 新增股票数：{}'.format(market, count)])

        self.process_list.clear()
        for task in self.tasks:
            p = Process(target=task)
            self.process_list.append(p)
            p.start()

        finished_count = len(self.tasks)
        market_count = len(g_market_list)
        while finished_count > 0:
            try:
                message = self.queue.get(timeout=10)
                taskname, market, ktype, progress, total = message
                if progress is None:
                    finished_count -= 1
                    if taskname in ('IMPORT_KDATA', 'IMPORT_TRANS',
                                    'IMPORT_TIME'):
                        self.send_message(
                            [taskname, 'FINISHED', market, ktype, total])
                    elif taskname == 'IMPORT_BLOCKINFO':
                        self.send_message([taskname, ktype])
                    elif taskname == 'IMPORT_ZH_BOND10':
                        self.send_message([taskname, ktype])
                    elif taskname == 'IMPORT_WEIGHT':
                        pass
                    else:
                        self.send_message([taskname, 'FINISHED'])
                    continue

                if taskname == 'IMPORT_WEIGHT':
                    if market == 'INFO':
                        self.send_message(['INFO', ktype])
                    self.send_message([taskname, market, total])
                elif taskname == 'IMPORT_FINANCE':
                    self.send_message([taskname, progress])
                elif taskname == 'IMPORT_KDATA':
                    hdf5_import_progress[market][ktype] = progress
                    current_progress = 0
                    for market in g_market_list:
                        current_progress += hdf5_import_progress[market][ktype]
                    current_progress = current_progress // market_count
                    self.send_message([taskname, ktype, current_progress])
                elif taskname == 'IMPORT_TRANS':
                    trans_progress[market] = progress
                    current_progress = 0
                    for market in g_market_list:
                        current_progress += trans_progress[market]
                    current_progress = current_progress // market_count
                    self.send_message([taskname, ktype, current_progress])
                elif taskname == 'IMPORT_TIME':
                    time_progress[market] = progress
                    current_progress = 0
                    for market in g_market_list:
                        current_progress += time_progress[market]
                    current_progress = current_progress // market_count
                    self.send_message([taskname, ktype, current_progress])
                elif taskname == 'IMPORT_BLOCKINFO':
                    self.send_message([taskname, market, ktype])
                else:
                    self.logger.error("Unknow task: {}".format(taskname))
            except queue.Empty:
                if finished_count > 0:
                    ok = False
                    for p in self.process_list:
                        if p.is_alive():
                            ok = True
                            break
                    if not ok:
                        for task in self.tasks:
                            self.logger.info(
                                f"task ({task.__class__.__name__}) status: {task.status}!"
                            )
                        for p in self.process_list:
                            self.logger.info(
                                f"Process exit code: {p.exitcode}")
                        finished_count = 0
                        self.logger.error(
                            "All process is finished, but some tasks are running!"
                        )

            except Exception as e:
                self.logger.error(str(e))


class_logger(UsePytdxImportToH5Thread)
