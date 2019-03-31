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
import sqlite3
import urllib.request


class ImportHistoryFinanceTask:
    def __init__(self, queue, dest_dir):
        self.queue = queue
        self.dest_dir = dest_dir + '/downloads/finance'
        if not os.path.lexists(self.dest_dir):
            os.makedirs(self.dest_dir)
        self.task_name = 'IMPORT_FINANCE'

    def get_list_info(self):
        result = []
        try:
            net_file = urllib.request.urlopen("http://down.tdx.com.cn:8001/fin/gpcw.txt", timeout=60)
            buffer = net_file.read()
            buffer = buffer.decode("utf-8")
            def list_to_dict(l):
                return {
                    'filename': l[0],
                    'hash': l[1],
                    'filesize': int(l[2])
                }
            result = [list_to_dict(l) for l in [line.strip().split(",") for line in buffer.strip().split('\n')]]
        except:
            print("failed download http://down.tdx.com.cn:8001/fin/gpcw.txt")
            result = []
        return result

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
        try:
            filename = item['filename']
            #print(filename)
            net_file = urllib.request.urlopen("http://down.tdx.com.cn:8001/fin/{}".format(filename), timeout=60)
            dest_file_name = self.dest_dir + "/" + filename
            with open(dest_file_name, 'wb') as f:
                f.write(net_file.read())
            shutil.unpack_archive(dest_file_name, extract_dir=self.dest_dir)
            return True
        except:
            return False

    def __call__(self):
        data_list = self.get_list_info()
        total_count = len(data_list)
        count = 0
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
            count += 1
            #print('{}'.format(int(100*count/total_count)))
            self.queue.put([self.task_name, None, None, int(100*count/total_count), total_count])
        self.queue.put([self.task_name, None, None, None, total_count])


if __name__ == "__main__":
    task = ImportHistoryFinanceTask(None, "c:\\stock")
    task()
    print("over!")
