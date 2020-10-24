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

import os
import hashlib
import sqlite3
import urllib.request
import mysql.connector

from pytdx.hq import TdxHq_API
from hikyuu.data.common_pytdx import search_best_tdx
from hikyuu.data.weight_to_sqlite import qianlong_import_weight
from hikyuu.data.pytdx_weight_to_sqlite import pytdx_import_weight_to_sqlite
from hikyuu.data.pytdx_weight_to_mysql import pytdx_import_weight_to_mysql
#from hikyuu.data.pytdx_finance_to_sqlite import pytdx_import_finance


class ImportWeightToSqliteTask:
    def __init__(self, queue, config, dest_dir):
        self.queue = queue
        self.config = config
        self.dest_dir = dest_dir
        self.msg_name = 'IMPORT_WEIGHT'

    def __call__(self):
        total_count = 0
        try:
            if self.config.getboolean('hdf5', 'enable', fallback=True):
                sqlite_file = "{}/stock.db".format(self.config['hdf5']['dir'])
                connect = sqlite3.connect(sqlite_file, timeout=1800)
                pytdx_import_weight = pytdx_import_weight_to_sqlite
                print('use sqlite import weight')
            else:
                db_config = {
                    'user': self.config['mysql']['usr'],
                    'password': self.config['mysql']['pwd'],
                    'host': self.config['mysql']['ip'],
                    'port': self.config['mysql']['port']
                }
                connect = mysql.connector.connect(**db_config)
                pytdx_import_weight = pytdx_import_weight_to_mysql
                print('use mysql import weight')

        except Exception as e:
            #self.queue.put([self.msg_name, str(e), -1, 0, total_count])
            self.queue.put([self.msg_name, 'INFO', str(e), 0, 0])
            self.queue.put([self.msg_name, '', 0, None, total_count])
            return

        try:
            """
            download_dir = self.dest_dir + "/downloads"
            if not os.path.lexists(download_dir):
                os.makedirs(download_dir)

            self.queue.put([self.msg_name, '正在下载钱龙权息信息...', 0, 0, 0])
            net_file = urllib.request.urlopen(
                'http://www.qianlong.com.cn/download/history/weight.rar', timeout=60
            )
            buffer = net_file.read()

            self.queue.put([self.msg_name, '钱龙权息信息下载完成，正在校验是否存在更新...', 0, 0, 0])
            new_md5 = hashlib.md5(buffer).hexdigest()

            dest_filename = download_dir + '/weight.rar'
            old_md5 = None
            if os.path.exists(dest_filename):
                with open(dest_filename, 'rb') as oldfile:
                    old_md5 = hashlib.md5(oldfile.read()).hexdigest()

            #如果没变化不需要解压导入
            if new_md5 != old_md5:
                with open(dest_filename, 'wb') as file:
                    file.write(buffer)

                self.queue.put([self.msg_name, '正在解压钱龙权息信息...', 0, 0, 0])
                x = os.system('unrar x -o+ -inul {} {}'.format(dest_filename, download_dir))
                if x != 0:
                    raise Exception("无法找到unrar命令！")

                self.queue.put([self.msg_name, '正在导入钱龙权息数据...', 0, 0, 0])
                total_count = qianlong_import_weight(connect, download_dir + '/weight', 'SH')
                total_count += qianlong_import_weight(connect, download_dir + '/weight', 'SZ')
                self.queue.put([self.msg_name, '导入钱龙权息数据完毕!', 0, 0, total_count])

            else:
                self.queue.put([self.msg_name, '钱龙权息数据无变化', 0, 0, 0])
            """

            hosts = search_best_tdx()
            api = TdxHq_API()
            api.connect(hosts[0][2], hosts[0][3])

            self.queue.put([self.msg_name, '正在导入权息数据...', 0, 0, 0])
            total_count = pytdx_import_weight(api, connect, "SH")
            total_count += pytdx_import_weight(api, connect, "SZ")
            self.queue.put([self.msg_name, '导入权息数据完毕!', 0, 0, total_count])

            #self.queue.put([self.msg_name, '下载通达信财务信息(上证)...', 0, 0, 0])
            #x = pytdx_import_finance(connect, api, "SH")

            #self.queue.put([self.msg_name, '下载通达信财务信息(深证)...', 0, 0, 0])
            #x += pytdx_import_finance(connect, api, "SZ")
            #self.queue.put([self.msg_name, '导入通达信财务信息完毕!', 0, 0, x])

            api.disconnect()

        except Exception as e:
            #self.queue.put([self.msg_name, str(e), -1, 0, total_count])
            self.queue.put([self.msg_name, 'INFO', str(e), 0, 0])
        finally:
            connect.commit()
            connect.close()

        self.queue.put([self.msg_name, '', 0, None, total_count])
