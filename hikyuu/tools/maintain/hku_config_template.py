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

hdf5_template = """
[hikyuu]
tmpdir = {dir}/tmp

[block]
type  = qianlong
dir = {dir}/block
指数板块 = zsbk.ini
行业板块 = hybk.ini
地域板块 = dybk.ini
概念板块 = gnbk.ini
self = self.ini

[preload]
day = 1
week = 0
month = 0
quarter = 0
halfyear = 0
year = 0

[baseinfo]
type = sqlite3
db = {dir}/stock.db

[kdata]
;type = tdx
;dir = D:\\TdxW_HuaTai\\vipdoc
type = hdf5
sh_day = {dir}/sh_day.h5
sh_min = {dir}/sh_1min.h5
sh_min5 = {dir}/sh_5min.h5
sz_day = {dir}/sz_day.h5
sz_min = {dir}/sz_1min.h5
sz_min5 = {dir}/sz_5min.h5
sh_time = {dir}/sh_time.h5
sz_time = {dir}/sz_time.h5
sh_trans = {dir}/sh_trans.h5
sz_trans = {dir}/sz_trans.h5
"""


mysql_template = """
[hikyuu]
tmpdir = {dir}/tmp

[block]
type  = qianlong
dir = {dir}/block
指数板块 = zsbk.ini
行业板块 = hybk.ini
地域板块 = dybk.ini
概念板块 = gnbk.ini
self = self.ini

[preload]
day = 1
week = 0
month = 0
quarter = 0
halfyear = 0
year = 0

[baseinfo]
type = mysql
host = {host}
port = {port}
usr = {usr}
pwd = {pwd}

[kdata]
type = mysql
host = {host}
port = {port}
usr = {usr}
pwd = {pwd}

"""