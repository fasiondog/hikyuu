#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import sqlite3

from multiprocessing import Queue, Process
from PyQt5.QtCore import QThread, pyqtSignal
from TdxImportTask import TdxImportTask, WeightImportTask

from hdf5import import *

class HDF5ImportThread(QThread):
    message = pyqtSignal(list)

    def __init__(self, config):
        super(self.__class__, self).__init__()
        self.config = config
        self.msg_name = 'HDF5_IMPORT'
        self.msg_task_name = ''

        if not self.check():
            self.working = False
            return

        self.process_list = []

        src_dir = config['tdx']['dir']
        dest_dir = config['hdf5']['dir']
        sqlite_file_name = dest_dir + "/stock.db"

        self.quotations = []
        if self.config['quotation']['stock']:
            self.quotations.append('stock')
        if self.config['quotation']['fund']:
            self.quotations.append('fund')

        self.queue = Queue()
        self.tasks = []
        if self.config.getboolean('weight', 'enable', fallback=False):
            self.tasks.append(WeightImportTask(self.queue, sqlite_file_name, dest_dir))
        if self.config.getboolean('ktype', 'day', fallback=False):
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SH', 'DAY', self.quotations, src_dir, dest_dir))
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SZ', 'DAY', self.quotations, src_dir, dest_dir))
        if self.config.getboolean('ktype', 'min5', fallback=False):
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SH', '5MIN', self.quotations, src_dir, dest_dir))
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SZ', '5MIN', self.quotations, src_dir, dest_dir))
        if self.config.getboolean('ktype', 'min', fallback=False):
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SH', '1MIN', self.quotations, src_dir, dest_dir))
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SZ', '1MIN', self.quotations, src_dir, dest_dir))


    def __del__(self):
        #print("HDF5ImportThread.__del__")
        for p in self.process_list:
            if p.is_alive():
                p.terminate()

    def check(self):
        return True

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

        connect = sqlite3.connect(dest_dir + "\\stock.db")
        create_database(connect)

        tdx_import_stock_name_from_file(connect, src_dir + "\\T0002\\hq_cache\\shm.tnf", 'SH', self.quotations)
        tdx_import_stock_name_from_file(connect, src_dir + "\\T0002\\hq_cache\\szm.tnf", 'SZ', self.quotations)

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
                if taskname == 'TdxImportTask':
                    self.send_message(['IMPORT_KDATA', 'FINISHED', market, ktype, total])
                else:
                    self.send_message([taskname, 'FINISHED'])
                continue

            if taskname == 'WeightImportTask':
                self.send_message(['IMPORT_WEIGHT', market, total])
            elif taskname == 'TdxImportTask':
                hdf5_import_progress[market][ktype] = progress
                current_progress = (hdf5_import_progress['SH'][ktype] + hdf5_import_progress['SZ'][ktype]) // 2
                self.send_message(['IMPORT_KDATA', ktype, current_progress])
            else:
                print("Unknow task: ", taskname)
