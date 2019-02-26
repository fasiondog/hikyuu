#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import os.path
import datetime
import sqlite3
import math
import sys
import urllib.request
import hashlib
from configparser import ConfigParser

from hikyuu.data.weight_to_sqlite import qianlong_import_weight
from hikyuu.data.common_pytdx import search_best_tdx

from hikyuu.gui.data.UsePytdxImportToH5Thread import UsePytdxImportToH5Thread
from PyQt5.Qt import QCoreApplication

def import_weight(connect, dest_dir):
    try:
        print('正在下载权息数据...')
        net_file = urllib.request.urlopen('http://www.qianlong.com.cn/download/history/weight.rar', timeout=60)
        buffer = net_file.read()
        print('下载完成，正在校验是否存在更新...')
        new_md5 = hashlib.md5(buffer).hexdigest()
        dest_filename = dest_dir+'/weight.rar'
        old_md5 = None
        if os.path.exists(dest_filename):
            with open(dest_filename, 'rb') as oldfile:
                old_md5 = hashlib.md5(oldfile.read()).hexdigest()
        #如果没变化不需要解压导入
        if new_md5 != old_md5:
            with open(dest_filename, 'wb') as file:
                file.write(buffer)
            print('下载完成，正在解压...')
            x = os.system('unrar x -o+ -inul {} {}'.format(dest_filename, dest_dir))
            if x != 0:
                raise Exception("无法找到unrar命令！")
            print('解压完毕，正在导入权息数据...')
            total_count = qianlong_import_weight(connect, dest_dir + '/weight', 'SH')
            total_count += qianlong_import_weight(connect, dest_dir + '/weight', 'SZ')
            print('导入完成，共导入权息数量：%d' % total_count)
        else:
            print('导入完成，权息数据无变化')
    except Exception as e:
        print(e)
    finally:
        connect.commit()
        connect.close()


class HKUImportDataCMD:
    def __init__(self):
        self.initThreads()

    def getUserConfigDir(self):
        return os.path.expanduser('~') + '/.hikyuu'

    def getCurrentConfig(self):
        #读取保存的配置文件信息，如果不存在，则使用默认配置
        this_dir = self.getUserConfigDir()
        import_config = ConfigParser()
        import_config.read(this_dir + '/importdata-gui.ini', encoding = 'utf-8')
        return import_config

    def initThreads(self):
        self.escape_time_thread = None
        self.hdf5_import_thread = None
        self.mysql_import_thread = None

        self.import_running = False

        self.progress = {'DAY': 0, '1MIN': 0, '5MIN': 0, 'TRANS':0, 'TIME':0}
        #self.hdf5_import_progress_bar = {'DAY': self.hdf5_day_progressBar,
        #                                 '1MIN': self.hdf5_min_progressBar,
        #                                 '5MIN': self.hdf5_5min_progressBar}

    def print_progress(self, ktype, progress):
        if progress != self.progress[ktype]:
            print('import {} data process: {}%'.format(ktype, progress))
            self.progress[ktype] = progress

    def on_message_from_thread(self, msg):
        if not msg or len(msg) < 2:
            print("msg is empty!")
            return

        msg_name, msg_task_name = msg[:2]
        if msg_name == 'HDF5_IMPORT':
            if msg_task_name == 'INFO':
                #self.import_detail_textEdit.append(msg[2])
                print(msg)

            elif msg_task_name == 'THREAD':
                status = msg[2]
                if status == 'FAILURE':
                    #self.import_status_label.setText("耗时：{:>.2f} 秒 导入异常！".format(self.escape_time))
                    #self.import_detail_textEdit.append(msg[3])
                    print(msg)
                self.hdf5_import_thread.terminate()
                self.hdf5_import_thread = None
                #self.escape_time_thread.stop()
                self.escape_time_thread = None
                print("导入完毕！")
                self.import_running = False
                QCoreApplication.quit()

            elif msg_task_name == 'IMPORT_KDATA':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.print_progress(ktype, progress)
                    #self.hdf5_import_progress_bar[ktype].setValue(progress)
                else:
                    #self.import_detail_textEdit.append('导入 {} {} 记录数：{}'
                    #                                   .format(msg[3], msg[4], msg[5]))
                    print('导入 {} {} 记录数：{}'.format(msg[3], msg[4], msg[5]))

            elif msg_task_name == 'IMPORT_TRANS':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    #self.hdf5_trans_progressBar.setValue(progress)
                    self.print_progress('TRANS', progress)
                else:
                    #self.import_detail_textEdit.append('导入 {} 分笔记录数：{}'
                    #                                   .format(msg[3], msg[5]))
                    print('导入 {} 分笔记录数：{}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_TIME':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    #self.hdf5_time_progressBar.setValue(progress)
                    self.print_progress('TIME', progress)
                else:
                    #self.import_detail_textEdit.append('导入 {} 分时记录数：{}'
                    #                                   .format(msg[3], msg[5]))
                    print('导入 {} 分时记录数：{}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_WEIGHT':
                pass
                #self.hdf5_weight_label.setText(msg[2])
                #if msg[2] == '导入完成!':
                #    self.import_detail_textEdit.append('导入权息记录数：{}'.format(msg[3]))


    def start_import_data(self):
        config = self.getCurrentConfig()
        dest_dir = config.get('hdf5', 'dir')
        if not os.path.exists(dest_dir) or not os.path.isdir(dest_dir):
            print("错误:", '指定的目标数据存放目录不存在！')
            return

        self.import_running = True

        print("正在启动任务....")
        QCoreApplication.processEvents()

        self.hdf5_import_thread = UsePytdxImportToH5Thread(config)

        self.hdf5_import_thread.message.connect(self.on_message_from_thread)
        self.hdf5_import_thread.start()
        #self.hdf5_import_thread.quit()
        #self.hdf5_import_thread.wait()
        print("*****************")


if __name__ == '__main__':   
    
    import time
    starttime = time.time()
    
    dest_dir = "c:\\stock"
    
    connect = sqlite3.connect(dest_dir + "\\hikyuu-stock.db")
    #import_weight(connect, dest_dir)
    connect.close()
    
    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime-starttime))
    print("%.2fm" % ((endtime-starttime)/60))

    x = HKUImportDataCMD()
    app = QCoreApplication(sys.argv)
    x.start_import_data()
    sys.exit(app.exec())

