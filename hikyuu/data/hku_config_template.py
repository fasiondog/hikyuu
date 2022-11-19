# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
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
datadir = {dir}

[block]
type  = qianlong
dir = {dir}/block
指数板块 = zsbk.ini
行业板块 = hybk.ini
地域板块 = dybk.ini
概念板块 = gnbk.ini
self = self.ini

[preload]
day = {day}
week = {week}
month = {month}
quarter = {quarter}
halfyear = {halfyear}
year = {year}
min = {min1}
min5 = {min5}
min15 = {min15}
min30 = {min30}
min60 = {min60}
day_max = {day_max}
week_max = {week_max}
month_max = {month_max}
quarter_max = {quarter_max}
halfyear_max = {halfyear_max}
year_max = {year_max}
min_max = {min1_max}
min5_max = {min5_max}
min15_max = {min15_max}
min30_max = {min30_max}
min60_max = {min60_max}


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
bj_day = {dir}/bj_day.h5
bj_min = {dir}/bj_1min.h5
bj_min5 = {dir}/bj_5min.h5
bj_time = {dir}/bj_time.h5
bj_trans = {dir}/bj_trans.h5
"""

mysql_template = """
[hikyuu]
tmpdir = {dir}
datadir = {dir}

[block]
type  = qianlong
dir = {dir}/block
指数板块 = zsbk.ini
行业板块 = hybk.ini
地域板块 = dybk.ini
概念板块 = gnbk.ini
self = self.ini

[preload]
day = {day}
week = {week}
month = {month}
quarter = {quarter}
halfyear = {halfyear}
year = {year}
min = {min1}
min5 = {min5}
min15 = {min15}
min30 = {min30}
min60 = {min60}
day_max = {day_max}
week_max = {week_max}
month_max = {month_max}
quarter_max = {quarter_max}
halfyear_max = {halfyear_max}
year_max = {year_max}
min_max = {min1_max}
min5_max = {min5_max}
min15_max = {min15_max}
min30_max = {min30_max}
min60_max = {min60_max}

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