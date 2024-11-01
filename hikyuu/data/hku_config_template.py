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
quotation_server = {quotation_server}

[block]
type = sqlite3
db = {dir}/stock.db
;type  = qianlong
;dir = {dir}/block
;指数板块 = zsbk.ini
;行业板块 = hybk.ini
;地域板块 = dybk.ini
;概念板块 = gnbk.ini
;self = self.ini

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
hour2= {hour2}
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
hour2_max = {hour2_max}

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
quotation_server = {quotation_server}

[block]
type = mysql
host = {host}
port = {port}
usr = {usr}
pwd = {pwd}

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
hour2 = {hour2}
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
hour2_max = {hour2_max}

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


import_config_template = """
[quotation]
stock = True
fund = True
future = False

[ktype]
day = True
min = True
min5 = True
trans = True
time = True
day_start_date = 1990-12-19
min_start_date = 2023-09-19
min5_start_date = 2023-09-19
trans_start_date = 2023-12-11
time_start_date = 2023-12-11

[weight]
enable = True

[tdx]
enable = False
;dir = d:/TdxW_HuaTai

[pytdx]
enable = False
use_tdx_number = 10

[hdf5]
enable = True
dir = {dir}

[mysql]
enable = False
;tmpdir = D:/stock
;host = 127.0.0.1
;port = 3306
;usr = root
;pwd = 

[sched]
time = 18:00:00

[collect]
quotation_server = ipc:///tmp/hikyuu_real.ipc
interval = 305
source = qq
use_zhima_proxy = False
phase1_start = 00:00:00
phase1_end = 11:35:00
phase2_start = 12:00:00
phase2_end = 15:05:00

[preload]
day = True
week = True
month = True
quarter = False
halfyear = False
year = False
min = False
min5 = False
min15 = False
min30 = False
min60 = False
hour2 = False
day_max = 100000
week_max = 100000
month_max = 100000
quarter_max = 100000
halfyear_max = 100000
year_max = 100000
min_max = 5120
min5_max = 5120
min15_max = 5120
min30_max = 5120
min60_max = 5120
hour2_max = 5120
"""


def generate_default_config():
    import os
    import sys
    import shutil
    from hikyuu.data.hku_config_template import hdf5_template
    user_dir = os.path.expanduser('~')
    data_dir = "c:\\stock" if sys.platform == 'win32' else f"{user_dir}/stock"
    hdf5_config = hdf5_template.format(dir=data_dir, quotation_server='ipc:///tmp/hikyuu_real.ipc',
                                       day=True, week=False,
                                       month=False, quarter=False, halfyear=False, year=False,
                                       min1=False, min5=False, min15=False, min30=False,
                                       min60=False, hour2=False, day_max=100000, week_max=100000,
                                       month_max=100000, quarter_max=100000, halfyear_max=100000,
                                       year_max=100000, min1_max=5120, min5_max=5120, min15_max=5120,
                                       min30_max=5120, min60_max=5120, hour2_max=5120)
    config_dir = f"{user_dir}/.hikyuu"
    if not os.path.lexists(config_dir):
        os.makedirs(config_dir)
    filename = f"{config_dir}/hikyuu.ini"
    if not os.path.exists(filename):
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(hdf5_config)

    if not os.path.lexists(data_dir):
        os.makedirs(data_dir)
    filename = f"{config_dir}/importdata-gui.ini"
    if not os.path.exists(filename):
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(import_config_template.format(dir=data_dir))

    if not os.path.lexists(data_dir + '/block'):
        current_dir = os.path.dirname(os.path.abspath(__file__))
        dirname, _ = os.path.split(current_dir)
        dirname = os.path.join(dirname, 'config/block')
        shutil.copytree(dirname, data_dir + '/block')
        os.remove(data_dir + '/block/__init__.py')
