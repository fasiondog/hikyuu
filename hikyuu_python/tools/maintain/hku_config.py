# -*- coding: utf-8 -*-

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox, \
                            QAbstractItemView, QHeaderView, QTableWidgetItem
from PyQt5.QtCore import Qt, pyqtSlot, pyqtSignal, QObject, QDate
from PyQt5.QtGui import QTextCursor, QIcon, QBrush, QColor

import os
import sys
import shutil
import logging

from configparser import ConfigParser

from pytdx.config.hosts import hq_hosts

from MainWindow import *

class MyMainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, parent=None):
        super(MyMainWindow, self).__init__(parent)
        self.setupUi(self)
        self.initUI()

    def closeEvent(self, event):
        self.saveConfig()
        event.accept()

    def initUI(self):
        self.setWindowIcon(QIcon("./hikyuu.ico"))
        self.setFixedSize(self.width(), self.height())

        #读取保存的配置文件信息，如果不存在，则使用默认配置
        this_dir = os.getcwd()
        import_config = ConfigParser()
        if os.path.exists(this_dir + '/importdata.ini'):
            import_config.read(this_dir + '/importdata.ini')

        #初始化通道信目录配置
        tdx_enable = import_config.getboolean('tdx', 'enable', fallback=True)
        tdx_dir = import_config.get('tdx', 'dir', fallback='d:\TdxW_HuaTai')
        self.tdx_enable_checkBox.setChecked(tdx_enable)
        self.tdx_dir_lineEdit.setText(tdx_dir)

        #初始化大智慧目录配置
        dzh_enable = import_config.getboolean('dzh', 'enable', fallback=False)
        dzh_dir = import_config.get('dzh', 'dir', fallback='')
        self.dzh_checkBox.setChecked(dzh_enable)
        self.dzh_dir_lineEdit.setText(dzh_dir)

        #初始化pytdx配置及显示
        tdx_server = import_config.get('pytdx', 'server', fallback='招商证券深圳行情')
        self.tdx_servers_comboBox.setDuplicatesEnabled(True)
        default_tdx_index = 0
        for i, host in enumerate(hq_hosts):
            self.tdx_servers_comboBox.addItem(host[0], host[1])
            if host[0] == tdx_server:
                default_tdx_index = i
        self.tdx_servers_comboBox.setCurrentIndex(default_tdx_index)
        self.tdx_port_lineEdit.setText(str(hq_hosts[default_tdx_index][2]))

        #初始化hdf5设置
        hdf5_enable = import_config.getboolean('hdf5', 'enable', fallback=True)
        hdf5_dir = import_config.get('hdf5', 'dir', fallback="c:\stock" if sys.platform == "win32" else os.path.expanduser('~') + "/stock")
        self.hdf5_enable_checkBox.setChecked(hdf5_enable)
        self.hdf5_dir_lineEdit.setText(hdf5_dir)

        #初始化MYSQL设置
        mysql_enable = import_config.getboolean('mysql', 'enable', fallback=False)
        mysql_host = import_config.get('mysql', 'host', fallback='127.0.0.1')
        mysql_port = import_config.get('mysql', 'port', fallback='8809')
        mysql_usr = import_config.get('mysql', 'usr', fallback='root')
        mysql_pwd = import_config.get('mysql', 'pwd', fallback='')
        self.mysql_enable_checkBox.setChecked(mysql_enable)
        self.mysql_host_lineEdit.setText(mysql_host)
        self.mysql_port_lineEdit.setText(mysql_port)
        self.mysql_usr_lineEdit.setText(mysql_usr)
        self.mysql_pwd_lineEdit.setText(mysql_pwd)

    def getCurrentConfig(self):
        import_config = ConfigParser()
        import_config['tdx'] = {'enable': self.tdx_enable_checkBox.isChecked(),
                                'dir': self.tdx_dir_lineEdit.text()}
        import_config['dzh'] = {'enable': self.dzh_checkBox.isChecked(),
                                'dir': self.dzh_dir_lineEdit.text()}
        import_config['pytdx'] = {'server': self.tdx_servers_comboBox.currentText(),
                                  'ip': hq_hosts[self.tdx_servers_comboBox.currentIndex()][1],
                                  'port': hq_hosts[self.tdx_servers_comboBox.currentIndex()][2]}
        import_config['hdf5'] = {'enable': self.hdf5_enable_checkBox.isChecked(),
                                 'dir': self.hdf5_dir_lineEdit.text()}
        import_config['mysql'] = {'enable': self.mysql_enable_checkBox.isChecked(),
                                  'host': self.mysql_host_lineEdit.text(),
                                  'port': self.mysql_port_lineEdit.text(),
                                  'usr': self.mysql_usr_lineEdit.text(),
                                  'pwd': self.mysql_pwd_lineEdit.text()}
        return import_config

    def saveConfig(self):
        filename = os.getcwd() + '/importdata.ini'
        with open(filename, 'w') as f:
            self.getCurrentConfig().write(f)

    @pyqtSlot()
    def on_select_tdx_dir_pushButton_clicked(self):
        dlg = QFileDialog()
        dlg.setFileMode(QFileDialog.Directory)
        config = self.getCurrentConfig()
        dlg.setDirectory(config['tdx']['dir'])
        if dlg.exec_():
            dirname = dlg.selectedFiles()
            self.tdx_dir_lineEdit.setText(dirname[0])

    @pyqtSlot()
    def on_select_dzh_dir_pushButton_clicked(self):
        dlg = QFileDialog()
        dlg.setFileMode(QFileDialog.Directory)
        config = self.getCurrentConfig()
        dlg.setDirectory(config['dzh']['dir'])
        if dlg.exec_():
            dirname = dlg.selectedFiles()
            self.dzh_dir_lineEdit.setText(dirname[0])

    @pyqtSlot()
    def on_hdf5_dir_pushButton_clicked(self):
        dlg = QFileDialog()
        dlg.setFileMode(QFileDialog.Directory)
        config = self.getCurrentConfig()
        dlg.setDirectory(config['hdf5']['dir'])
        if dlg.exec_():
            dirname = dlg.selectedFiles()
            self.hdf5_dir_lineEdit.setText(dirname[0])

    @pyqtSlot()
    def on_start_import_pushButton_clicked(self):
        config = self.getCurrentConfig()



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
