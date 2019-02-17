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

from tdx_to_h5 import tdx_import_data

class ProgressBar:
    def __init__(self, src):
        self.src = src

    def __call__(self, cur, total):
        self.src.queue.put([self.src.task_name, self.src.market, self.src.ktype, (cur+1) * 100 // total, 0])

class ImportTdxToH5Task:
    def __init__(self, queue, sqlitefile, market, ktype, quotations, src_dir, dest_dir):
        super(self.__class__, self).__init__()
        self.task_name = 'IMPORT_KDATA'
        self.queue = queue
        self.sqlitefile = sqlitefile
        self.market = market.upper()
        self.ktype = ktype.upper()
        self.quotations = quotations
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
        #print(self.__class__.__name__, self.market, self.ktype, "__del__")
        pass

    def __call__(self):
        count = 0
        try:
            connect = sqlite3.connect(self.sqlitefile)
            progress = ProgressBar(self)
            count = tdx_import_data(connect, self.market, self.ktype, self.quotations, self.src_dir, self.dest_dir, progress)
        except Exception as e:
            print(e)
        self.queue.put([self.task_name, self.market, self.ktype, None, count])
