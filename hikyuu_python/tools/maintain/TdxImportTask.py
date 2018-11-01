#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
import sqlite3
import subprocess
import urllib.request
from multiprocessing import Process, Queue

from hdf5import import *

class ProgressBar:
    def __init__(self, src):
        self.src = src

    def __call__(self, cur, total):
        self.src.queue.put([self.src.__class__.__name__, self.src.market, self.src.ktype, (cur+1) * 100 // total, 0])

class TdxImportTask:
    def __init__(self, queue, sqlitefile, market, ktype, quotation, src_dir, dest_dir):
        super(self.__class__, self).__init__()
        self.queue = queue
        self.sqlitefile = sqlitefile
        self.market = market.upper()
        self.ktype = ktype.upper()
        self.quotation = quotation
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
        self.dest_dir = dest_dir

    def __del__(self):
        print(self.__class__.__name__, self.market, self.ktype, "__del__")

    def __call__(self):
        count = 0
        try:
            connect = sqlite3.connect(self.sqlitefile)
            progress = ProgressBar(self)
            count = tdx_import_data(connect, self.market, self.ktype, self.quotation, self.src_dir, self.dest_dir, progress)
        except Exception as e:
            print(e)
        self.queue.put([self.__class__.__name__, self.market, self.ktype, None, count])

class WeightImportTask:
    def __init__(self, queue, sqlitefile, dest_dir):
        self.queue = queue
        self.sqlitefile = sqlitefile
        self.dest_dir = dest_dir

    def __call__(self):
        total_count = 0
        try:
            self.queue.put([self.__class__.__name__, '正在下载...', 0, 0, 0])
            connect = sqlite3.connect(self.sqlitefile)
            net_file = urllib.request.urlopen('http://www.qianlong.com.cn/download/history/weight.rar', timeout=60)
            dest_filename = self.dest_dir+'/weight.rar'
            with open(dest_filename, 'wb') as file:
                file.write(net_file.read())

            self.queue.put([self.__class__.__name__, '下载完成，正在解压...', 0, 0, 0])
            os.system('unrar x -o+ -inul {} {}'.format(dest_filename, self.dest_dir))

            self.queue.put([self.__class__.__name__, '解压完毕，导入权息数据...', 0, 0, 0])
            total_count = qianlong_import_weight(connect, self.dest_dir + '/weight', 'SH')
            total_count += qianlong_import_weight(connect, self.dest_dir + '/weight', 'SZ')
            self.queue.put([self.__class__.__name__, '导入完成', 0, 0, total_count])

        except Exception as e:
            print(e)

        self.queue.put([self.__class__.__name__, '导入完成', 0, None, total_count])
