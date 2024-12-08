#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import os.path
import sys
from configparser import ConfigParser

from PyQt5.Qt import QCoreApplication

from hikyuu.data.weight_to_sqlite import qianlong_import_weight
from hikyuu.data.common_pytdx import search_best_tdx

from hikyuu.gui.data.EscapetimeThread import EscapetimeThread
from hikyuu.gui.data.UseTdxImportToH5Thread import UseTdxImportToH5Thread
from hikyuu.gui.data.UsePytdxImportToH5Thread import UsePytdxImportToH5Thread


class HKUImportDataCMD:
    def __init__(self):
        self.initThreads()

    def getUserConfigDir(self):
        return os.path.expanduser('~') + '/.hikyuu'

    def getCurrentConfig(self):
        # 读取保存的配置文件信息，如果不存在，则使用默认配置
        this_dir = self.getUserConfigDir()
        import_config = ConfigParser()
        import_config.read(this_dir + '/importdata-gui.ini', encoding='utf-8')
        return import_config

    def initThreads(self):
        self.escape_time_thread = None
        self.hdf5_import_thread = None
        self.mysql_import_thread = None
        self.import_running = False
        self.progress = {'DAY': 0, '1MIN': 0, '5MIN': 0, 'TRANS': 0, 'TIME': 0}
        self.info_type = {'DAY': '日线数据', '1MIN': '一分钟线', '5MIN': '五分钟线', 'TRANS': '历史分笔', 'TIME': '分时数据'}
        self.escape_time = 0.0
        self.details = []

    def print_progress(self, ktype, progress):
        if progress != self.progress[ktype]:
            print(
                'import progress: {}%  - {} - 已耗时 {:>.2f} 分钟'.format(progress, self.info_type[ktype], self.escape_time)
            )
            self.progress[ktype] = progress

    def on_message_from_thread(self, msg):
        if not msg or len(msg) < 2:
            print("msg is empty!")
            return

        msg_name, msg_task_name = msg[:2]
        if msg_name == 'ESCAPE_TIME':
            self.escape_time = msg_task_name / 60

        elif msg_name == 'HDF5_IMPORT':
            if msg_task_name == 'INFO':
                print(msg[2])

            elif msg_task_name == 'THREAD':
                status = msg[2]
                if status == 'FAILURE':
                    self.details.append(msg[3])
                self.hdf5_import_thread.terminate()
                self.hdf5_import_thread = None
                self.escape_time_thread.stop()
                self.escape_time_thread = None
                print("\n导入完毕, 共耗时 {:>.2f} 分钟".format(self.escape_time))
                print('\n=========================================================')
                print("导入详情:")
                for info in self.details:
                    print(info)
                print('=========================================================')
                self.import_running = False
                QCoreApplication.quit()

            elif msg_task_name == 'IMPORT_KDATA':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.print_progress(ktype, progress)
                else:
                    self.details.append('导入 {} {} 记录数：{}'.format(msg[3], msg[4], msg[5]))

            elif msg_task_name == 'IMPORT_TRANS':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.print_progress('TRANS', progress)
                else:
                    self.details.append('导入 {} 分笔记录数：{}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_TIME':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.print_progress('TIME', progress)
                else:
                    self.details.append('导入 {} 分时记录数：{}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_WEIGHT':
                if msg[2] == 'INFO':
                    pass
                elif msg[2] == 'FINISHED':
                    print('导入权息数据完毕！')
                elif msg[2] == '导入完成!':
                    self.details.append('导入权息记录数：{}'.format(msg[3]))
                elif msg[2] == '权息数据无变化':
                    self.details.append(msg[3])
                else:
                    print('权息{}'.format(msg[2]))

            elif msg_task_name == 'IMPORT_FINANCE':
                print("财务数据下载: {}%".format(msg[2]))

    def start_import_data(self):
        config = self.getCurrentConfig()
        if config.getboolean('hdf5', 'enable'):
            if not os.path.lexists(config['hdf5']['dir']):
                os.makedirs(f"{config['hdf5']['dir']}/tmp")
            elif not os.path.isdir(config['hdf5']['dir']):
                print("错误", '指定的目标数据存放目录不存在！')
                sys.exit(-1)

        if config.getboolean('tdx', 'enable'):
            if not os.path.lexists(config['tdx']['dir']):
                os.makedirs(f"{config['tdx']['dir']}/tmp")
            elif not os.path.isdir(config['tdx']['dir']):
                print("错误", "请确认通达信安装目录是否正确！")
                sys.exit(-1)

        if config.getboolean('mysql', 'enable'):
            if not os.path.lexists(config['mysql']['tmpdir']):
                os.makedirs(config['mysql']['tmpdir'])
            elif not os.path.isdir(config['mysql']['tmpdir']):
                print("错误", "请确认临时目录是否正确！")
                sys.exit(-1)

        self.import_running = True

        print("正在启动任务....")
        QCoreApplication.processEvents()

        if config.getboolean('tdx', 'enable'):
            self.hdf5_import_thread = UseTdxImportToH5Thread(None, config)
        else:
            self.hdf5_import_thread = UsePytdxImportToH5Thread(None, config)

        self.hdf5_import_thread.message.connect(self.on_message_from_thread)
        self.hdf5_import_thread.start()

        self.escape_time = 0.0
        self.escape_time_thread = EscapetimeThread()
        self.escape_time_thread.message.connect(self.on_message_from_thread)
        self.escape_time_thread.start()


def main():
    app = QCoreApplication(sys.argv)
    x = HKUImportDataCMD()
    x.start_import_data()
    sys.exit(app.exec())


if __name__ == '__main__':
    main()
