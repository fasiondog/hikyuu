#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import os.path
import sys
import time
from configparser import ConfigParser

from hikyuu.data.weight_to_sqlite import qianlong_import_weight
from hikyuu.data.common_pytdx import search_best_tdx
from hikyuu.data.hku_config_template import generate_default_config

from hikyuu.gui.data.UseTdxImportToH5Thread import UseTdxImportToH5Thread
from hikyuu.gui.data.UsePytdxImportToH5Thread import UsePytdxImportToH5Thread


class HKUImportDataCMD:
    def __init__(self, ignore_kdata=False):
        self.ignore_kdata = ignore_kdata  # 忽略K线数据导入
        self.initThreads()

    def getUserConfigDir(self):
        return os.path.expanduser('~') + '/.hikyuu'

    def getCurrentConfig(self):
        # 读取保存的配置文件信息，如果不存在，则使用默认配置
        this_dir = self.getUserConfigDir()
        import_config = ConfigParser()
        if not os.path.exists(this_dir + '/importdata-gui.ini'):
            generate_default_config()
        import_config.read(this_dir + '/importdata-gui.ini', encoding='utf-8')

        if self.ignore_kdata:
            import_config.set('ktype', 'day', 'False')
            import_config.set('ktype', 'min', 'False')
            import_config.set('ktype', 'min5', 'False')
            import_config.set('ktype', 'trans', 'False')
            import_config.set('ktype', 'time', 'False')
        return import_config

    def initThreads(self):
        self.hdf5_import_thread = None
        self.mysql_import_thread = None
        self.import_running = False
        self.progress = {'DAY': 0, '1MIN': 0, '5MIN': 0, 'TRANS': 0, 'TIME': 0}
        self.info_type = {'DAY': 'Daily data', '1MIN': '1-minute data', '5MIN': '5-minute data', 'TRANS': 'Historical ticks', 'TIME': 'Time-series data'}
        self.start_import_time = time.time()
        self.details = []

    def time_escaped(self, unit='min'):
        if unit.lower() == 'min':
            return (time.time() - self.start_import_time) / 60
        if unit.lower() == 'hour':
            return (time.time() - self.start_import_time) / 3600
        return time.time() - self.start_import_time

    def print_progress(self, ktype, progress):
        if progress != self.progress[ktype]:
            print(
                'import progress: {}%  - {} - elapsed {:>.2f} minutes'.format(progress,
                                                                     self.info_type[ktype], self.time_escaped())
            )
            self.progress[ktype] = progress

    def on_message_from_thread(self, msg):
        if not msg or len(msg) < 2:
            print("msg is empty!")
            return

        msg_name, msg_task_name = msg[:2]
        if msg_name == 'HDF5_IMPORT':
            if msg_task_name == 'INFO':
                print(msg[2])

            elif msg_task_name == 'THREAD':
                status = msg[2]
                if status == 'FAILURE':
                    self.details.append(msg[3])
                print("\nImport completed, elapsed {:>.2f} minutes".format(self.time_escaped()))
                if not self.ignore_kdata:
                    print('\n=========================================================')
                    print("Import details:")
                    for info in self.details:
                        print(info)
                    print('=========================================================')
                self.import_running = False

            elif msg_task_name == 'IMPORT_KDATA':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.print_progress(ktype, progress)
                else:
                    self.details.append('Imported {} {} records: {}'.format(msg[3], msg[4], msg[5]))

            elif msg_task_name == 'IMPORT_TRANS':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.print_progress('TRANS', progress)
                else:
                    self.details.append('Imported {} tick records: {}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_TIME':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.print_progress('TIME', progress)
                else:
                    self.details.append('Imported {} time-series records: {}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_WEIGHT':
                if msg[2] == 'INFO':
                    pass
                elif msg[2] == 'FINISHED':
                    print('Import weight data completed!')
                elif msg[2] == 'Import completed!':
                    self.details.append('Imported weight records: {}'.format(msg[3]))
                elif msg[2] == 'No change in the weight data':
                    self.details.append(msg[3])
                else:
                    print('Weight {}'.format(msg[2]))

            elif msg_task_name == 'IMPORT_FINANCE':
                print("Finance data download: {}%".format(msg[2]))

    def start_import_data(self):
        config = self.getCurrentConfig()
        if config.getboolean('hdf5', 'enable'):
            if not os.path.lexists(config['hdf5']['dir']):
                os.makedirs(f"{config['hdf5']['dir']}/tmp")
            elif not os.path.isdir(config['hdf5']['dir']):
                print("Error", 'The specified target data directory does not exist!')
                sys.exit(-1)

        if config.getboolean('tdx', 'enable'):
            if not os.path.lexists(config['tdx']['dir']):
                os.makedirs(f"{config['tdx']['dir']}/tmp")
            elif not os.path.isdir(config['tdx']['dir']):
                print("Error", "Please check whether the TDX installation directory is correct!")
                sys.exit(-1)

        if config.getboolean('mysql', 'enable'):
            if not os.path.lexists(config['mysql']['tmpdir']):
                os.makedirs(config['mysql']['tmpdir'])
            elif not os.path.isdir(config['mysql']['tmpdir']):
                print("Error", "Please check whether the temporary directory is correct!")
                sys.exit(-1)

        self.import_running = True

        print("Starting the task....")

        if config.getboolean('tdx', 'enable'):
            self.hdf5_import_thread = UseTdxImportToH5Thread(None, config)
        else:
            self.hdf5_import_thread = UsePytdxImportToH5Thread(None, config)

        self.hdf5_import_thread.message.connect(self.on_message_from_thread)
        self.hdf5_import_thread.run()


def main(ignore_kdata=False):
    x = HKUImportDataCMD(ignore_kdata=ignore_kdata)
    x.start_import_data()


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--ignore-kdata':
        main(ignore_kdata=True)
    else:
        main(ignore_kdata=False)
