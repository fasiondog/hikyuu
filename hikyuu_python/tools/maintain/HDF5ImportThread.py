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

    def __init__(self, config, quotations, ktypes):
        super(self.__class__, self).__init__()
        self.config = config
        self.quotations = quotations
        self.ktypes = ktypes
        self.msg_name = 'HDF5_IMPORT'
        self.msg_task_name = ''

        if not self.check():
            return

        src_dir = config['tdx']['dir']
        dest_dir = config['hdf5']['dir']
        sqlite_file_name = dest_dir + "/hikyuu-stock.db"

        self.queue = Queue()
        self.tasks = []
        self.tasks.append(WeightImportTask(self.queue, sqlite_file_name, dest_dir))
        if 'DAY' in self.ktypes:
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SH', 'DAY','stock', src_dir, dest_dir))
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SZ', 'DAY','stock', src_dir, dest_dir))
        if '5MIN' in self.ktypes:
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SH', '5MIN','stock', src_dir, dest_dir))
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SZ', '5MIN', 'stock', src_dir, dest_dir))
        if '1MIN' in self.ktypes:
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SH', '1MIN','stock', src_dir, dest_dir))
            self.tasks.append(TdxImportTask(self.queue, sqlite_file_name, 'SZ', '1MIN', 'stock', src_dir, dest_dir))


    def __del__(self):
        print("HDF5ImportThread.__del__")

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

        connect = sqlite3.connect(dest_dir + "\\hikyuu-stock.db")
        create_database(connect)

        tdx_import_stock_name_from_file(connect, src_dir + "\\T0002\\hq_cache\\shm.tnf", 'SH', 'stock')
        tdx_import_stock_name_from_file(connect, src_dir + "\\T0002\\hq_cache\\szm.tnf", 'SZ', 'stock')


        for task in self.tasks:
            p = Process(target=task)
            p.start()

        finished_count = len(self.tasks)
        while finished_count > 0:
            message = self.queue.get()
            taskname, market, ktype, progress, total = message
            if progress is None:
                finished_count -= 1
                if taskname == 'TdxImportTask':
                    self.send_message(['IMPORT_KDATA', 'FINISHED', ktype, total])
                else:
                    self.send_message([taskname, 'FINISHED'])
                continue

            if taskname == 'WeightImportTask':
                self.send_message(['IMPORT_WEIGHT', market])
            elif taskname == 'TdxImportTask':
                hdf5_import_progress[market][ktype] = progress
                current_progress = (hdf5_import_progress['SH'][ktype] + hdf5_import_progress['SZ'][ktype]) // 2
                self.send_message(['IMPORT_KDATA', ktype, current_progress])
            else:
                print("Unknow task: ", taskname)
