# -*- coding: utf-8 -*-

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox, \
                            QAbstractItemView, QHeaderView, QTableWidgetItem
from PyQt5.QtCore import Qt, pyqtSlot, pyqtSignal, QObject, QDate
from PyQt5.QtGui import QTextCursor, QIcon, QBrush, QColor

import os
import sys
import shutil
import logging

from configparser import ConfigParser

from MainWindow import *


class MyMainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, parent=None):
        super(MyMainWindow, self).__init__(parent)
        self.setupUi(self)
        self.initUI()

    def closeEvent(self, event):
        event.accept()

    def initUI(self):
        self.setWindowIcon(QIcon("./hikyuu.ico"))
        self.setFixedSize(self.width(), self.height())

        #从用户目录下子目录.hikyuu下"data_dir.ini"文件中获取目标数据存放路径
        usr_dir = os.path.expanduser('~')
        hku_dir = usr_dir + '/.hikyuu'

        if not os.path.lexists(hku_dir):
            os.mkdir(hku_dir)

        data_dir_config = ConfigParser()
        data_config_file = hku_dir + '/data_dir.ini'
        try:
            data_dir_config.read(data_config_file)
        except:
            data_dir_config['data_dir'] = {}

        if 'data_dir' not in data_dir_config['data_dir']:
            data_dir_config['data_dir']['data_dir'] = "c:\stock" if sys.platform == "win32" else usr_dir + "/stock"

        data_dir = data_dir_config['data_dir']['data_dir']

        #读取importdata.ini配置信息
        import_config_file = data_dir + '/importdata.ini'
        try:
            import_config = self.load_import_config(import_config_file)
        except:
            import_config = self.get_default_hku_config(data_dir)

        from_list = []
        option_list = import_config.options('from')
        for option in option_list:
            tmp = option.split(',')
            if tmp[0] == "tdx":
                break



        if sys.platform == 'win32':
            hku_config_file = data_dir + '/hikyuu_win.ini'
        else:
            hku_config_file = data_dir + '/hikyuu_linux.ini'

        try:
            hku_config = self.load_hku_config(hku_config_file)
        except:
            hku_config = self.get_default_hku_config(data_dir)

        self.dataDirLineEdit.setText(data_dir)
        self.data_config_file = data_config_file
        self.data_dir_config = data_dir_config


    def load_import_config(self, filename):
        import_config = ConfigParser()
        import_config.read(filename)
        return import_config

    def get_default_import_config(self, data_dir):
        import_config = ConfigParser()
        import_config['database'] = {}
        import_config['database']['db'] = data_dir + '/stock.db'
        import_config['database']['sql'] = data_dir + '/create.sql'

        import_config['weight'] = {}
        import_config['weight']['sh'] = data_dir + '/weight/shase/weight'
        import_config['weight']['sz'] = data_dir + '/weight/sznse/weight'

        import_config['from'] = {}
        import_config['from'] = r'1, tdx, d:\TdxW_HuaTai'

        import_config['dest'] = {}
        import_config['dest']['sh_day'] = data_dir + '/sh_day.h5'
        import_config['dest']['sh_1min'] = data_dir + '/sh_1min.h5'
        import_config['dest']['sh_5min'] = data_dir + '/sh_5min.h5'
        import_config['dest']['sz_day'] = data_dir + '/sz_day.h5'
        import_config['dest']['sz_1min'] = data_dir + '/sz_1min.h5'
        import_config['dest']['sz_5min'] = data_dir + '/sz_5min.h5'
        return import_config



    def load_hku_config(self, filename):
        hku_config = ConfigParser()
        hku_config.read(filename)
        return hku_config

    def get_default_hku_config(self, data_dir):
        hku_config = ConfigParser()
        hku_config['hikyuu'] = {}
        hku_config['hikyuu']['tmpdir'] = data_dir

        hku_config['block'] = {}
        hku_config['block']['type'] = 'qianlong'
        hku_config['block']['dir'] = data_dir + '/block'
        hku_config['block']['指数板块'] = 'zsbk.ini'
        hku_config['block']['行业板块'] = 'hybk.ini'
        hku_config['block']['地域板块'] = 'dybk.ini'
        hku_config['block']['概念板块'] = 'gnbk.ini'
        hku_config['block']['self'] = 'self.ini'

        hku_config['preload'] = {}
        hku_config['preload']['day'] = 1
        hku_config['preload']['week'] = 0
        hku_config['preload']['month'] = 0
        hku_config['preload']['quater'] = 0
        hku_config['preload']['halfyear'] = 0
        hku_config['preload']['year'] = 0
        hku_config['preload']['min'] = 0
        hku_config['preload']['min5'] = 0
        hku_config['preload']['min15'] = 0
        hku_config['preload']['min30'] = 0
        hku_config['preload']['min60'] = 0

        hku_config['baseinfo'] = {}
        hku_config['baseinfo']['type'] = 'sqlite3'
        hku_config['baseinfo']['db'] = data_dir + '/stock.db'

        hku_config['kdata'] = {}
        hku_config['kdata']['type'] = 'hdf5'
        hku_config['kdata']['sh_day'] = data_dir + '/sh_day.h5'
        hku_config['kdata']['sh_min'] = data_dir + '/sh_1min.h5'
        hku_config['kdata']['sh_min5'] = data_dir + '/sh_5min.h5'
        hku_config['kdata']['sz_day'] = data_dir + '/sz_day.h5'
        hku_config['kdata']['sz_min'] = data_dir + '/sz_1min.h5'
        hku_config['kdata']['sz_min5'] = data_dir + '/sz_5min.h5'
        return hku_config



if __name__ == "__main__":

    app = QApplication(sys.argv)
    if (len(sys.argv) > 1 and sys.argv[1] == '0'):
        FORMAT = '%(asctime)-15s %(levelname)s: %(message)s [%(name)s::%(funcName)s]'
        logging.basicConfig(format=FORMAT, level=logging.INFO, handlers=[logging.StreamHandler(), ])
        capture_output = False
    else:
        capture_output = True

    myWin = MyMainWindow(None)
    myWin.show()
    sys.exit(app.exec())
