#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
import sqlite3
from multiprocessing import Process, Queue

from hdf5import import *

class ProgressBar:
    def __init__(self, src):
        self.src = src

    def __call__(self, cur, total):
        self.src.queue.put([self.src.market, self.src.ktype, (cur+1) * 100 // total])

class TdxImportTask:
    def __init__(self, queue, sqlitefile, market, ktype, quotation, src_dir, dest_dir, progress=None):
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
        self.progress = progress

    def __del__(self):
        print(self.market, "__del__")

    def __call__(self):
        try:
            connect = sqlite3.connect(self.sqlitefile)
            progress = ProgressBar(self)
            count = tdx_import_data(connect, self.market, self.ktype, self.quotation, self.src_dir, self.dest_dir, progress)
            self.queue.put([self.market, self.ktype, None])
            print(self.market, count)
        except Exception as e:
            print(e)


