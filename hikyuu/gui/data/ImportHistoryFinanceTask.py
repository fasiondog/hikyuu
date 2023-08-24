# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) fasiondog/hikyuu
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
import shutil
import hashlib
from pytdx.hq import TdxHq_API
from hikyuu.util import *


class ImportHistoryFinanceTask:
    def __init__(self, log_queue, queue, dest_dir):
        self.log_queue = log_queue
        self.queue = queue
        self.dest_dir = dest_dir + '/downloads/finance'
        if not os.path.lexists(self.dest_dir):
            os.makedirs(self.dest_dir)
        self.task_name = 'IMPORT_FINANCE'
        self.total_count = 0
        self.status = "no run"

    def connect(self):
        self.api = TdxHq_API()
        hku_check(self.api.connect('120.76.152.87', 7709), "failed connect pytdx!")

    def get_list_info(self):
        result = []
        content = self.api.get_report_file_by_size('tdxfin/gpcw.txt')
        content = content.decode('utf-8')
        content = content.strip().split('\r\n')

        def l2d(item):
            return {'filename': item[0], 'hash': item[1], 'filesize': int(item[2])}

        return [l2d(i.strip().split(',')) for i in content]

    def download(self):
        data_list = self.get_list_info()
        for item in data_list:
            dest_file = '{}/{}'.format(self.dest_dir, item['filename'])
            if not os.path.exists(dest_file):
                self.download_file(item)
            else:
                new_md5 = ''
                with open(dest_file, 'rb') as f:
                    new_md5 = hashlib.md5(f.read()).hexdigest()
                if new_md5 != item['hash']:
                    #print(dest_file, new_md5, item['hash'])
                    self.download_file(item)

    def download_file(self, item):
        filename = item['filename']
        # filesize = item['filesize']
        # get_report_file_by_size 传入实际的 filesize，实际会出错
        data = self.api.get_report_file_by_size(f'tdxfin/{filename}', 0)
        dest_file_name = self.dest_dir + "/" + filename
        with open(dest_file_name, 'wb') as f:
            f.write(data)
        shutil.unpack_archive(dest_file_name, extract_dir=self.dest_dir)
        hku_info(f"Download finance file: {filename}")

    @hku_catch(trace=True)
    def __call__(self):
        self.status = "running"
        capture_multiprocess_all_logger(self.log_queue)
        self.connect()
        data_list = self.get_list_info()
        self.total_count = len(data_list)
        count = 0
        for item in data_list:
            try:
                dest_file = '{}/{}'.format(self.dest_dir, item['filename'])
                if not os.path.exists(dest_file):
                    self.download_file(item)
                else:
                    old_md5 = ''
                    with open(dest_file, 'rb') as f:
                        old_md5 = hashlib.md5(f.read()).hexdigest()
                    if old_md5 != item['hash']:
                        self.download_file(item)
                count += 1
                self.queue.put([self.task_name, None, None, int(100 * count / self.total_count), self.total_count])
            except Exception as e:
                hku_error(str(e))
        self.queue.put([self.task_name, None, None, None, self.total_count])
        self.status = "finished"


if __name__ == "__main__":
    task = ImportHistoryFinanceTask(None, "c:\\stock")
    task()
    print("over!")
