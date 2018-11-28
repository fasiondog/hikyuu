# -*- coding: utf-8 -*-

import os
import logging
from configparser import ConfigParser
from pytdx.config.hosts import hq_hosts

from PyQt5.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox
from PyQt5.QtCore import pyqtSlot
from PyQt5.QtGui import  QIcon

from MainWindow import *
from EscapetimeThread import EscapetimeThread
from UseTdxImportToH5Thread import UseTdxImportToH5Thread
from UsePytdxImportToH5Thread import UsePytdxImportToH5Thread
from pytdx_common import search_best_tdx

import hku_config_template


class MyMainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, parent=None):
        super(MyMainWindow, self).__init__(parent)
        self.setupUi(self)
        self.initUI()
        self.initThreads()

    def closeEvent(self, event):
        if self.import_running:
            QMessageBox.about(self, '提示', '正在执行导入任务，请耐心等候！')
            event.ignore()
            return

        self.saveConfig()

        if self.hdf5_import_thread:
            self.hdf5_import_thread.stop()
        if self.escape_time_thread:
            self.escape_time_thread.stop()
        event.accept()

    def getUserConfigDir(self):
        return os.path.expanduser('~') + '/.hikyuu'

    def saveConfig(self):
        current_config = self.getCurrentConfig()
        filename = self.getUserConfigDir() + '/importdata-gui.ini'
        with open(filename, 'w', encoding='utf-8') as f:
            current_config.write(f)

        filename = self.getUserConfigDir() + '/hikyuu.ini'
        data_dir = current_config['hdf5']['dir']
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(hku_config_template.hdf5_template.format(dir=data_dir))

    def initUI(self):
        self.setWindowIcon(QIcon("./hikyuu.ico"))
        self.setFixedSize(self.width(), self.height())
        self.import_status_label.setText('')
        self.import_detail_textEdit.clear()
        self.reset_progress_bar()

        #读取保存的配置文件信息，如果不存在，则使用默认配置
        this_dir = self.getUserConfigDir()
        import_config = ConfigParser()
        if os.path.exists(this_dir + '/importdata-gui.ini'):
            import_config.read(this_dir + '/importdata-gui.ini', encoding = 'utf-8')

        #初始化导入行情数据类型配置
        self.import_stock_checkBox.setChecked(import_config.getboolean('quotation', 'stock', fallback=True))
        self.import_fund_checkBox.setChecked(import_config.getboolean('quotation', 'fund', fallback=True))
        self.import_future_checkBox.setChecked(import_config.getboolean('quotation', 'future', fallback=False))

        #初始化导入K线类型配置
        self.import_day_checkBox.setChecked(import_config.getboolean('ktype', 'day', fallback=True))
        self.import_min_checkBox.setChecked(import_config.getboolean('ktype', 'min', fallback=True))
        self.import_min5_checkBox.setChecked(import_config.getboolean('ktype', 'min5', fallback=True))
        self.import_trans_checkBox.setChecked(import_config.getboolean('ktype', 'trans', fallback=False))
        self.import_time_checkBox.setChecked(import_config.getboolean('ktype', 'time', fallback=False))
        self.trans_max_days_spinBox.setValue(import_config.getint('ktype', 'trans_max_days', fallback=70))
        self.time_max_days_spinBox.setValue(import_config.getint('ktype', 'time_max_days', fallback=70))

        #初始化权息数据设置
        self.import_weight_checkBox.setChecked(import_config.getboolean('weight', 'enable', fallback=True))

        #初始化通道信目录配置
        tdx_enable = import_config.getboolean('tdx', 'enable', fallback=True)
        tdx_dir = import_config.get('tdx', 'dir', fallback='d:\TdxW_HuaTai')
        self.tdx_radioButton.setChecked(tdx_enable)
        self.tdx_dir_lineEdit.setEnabled(tdx_enable)
        self.select_tdx_dir_pushButton.setEnabled(tdx_enable)
        self.tdx_dir_lineEdit.setText(tdx_dir)
        self.tdx_radioButton.toggled.connect(self.on_tdx_or_pytdx_toggled)

        #初始化pytdx配置及显示
        pytdx_enable = import_config.getboolean('pytdx', 'enable', fallback=False)
        self.pytdx_radioButton.setChecked(pytdx_enable)

        #初始化hdf5设置
        hdf5_enable = import_config.getboolean('hdf5', 'enable', fallback=True)
        hdf5_dir = import_config.get('hdf5', 'dir', fallback="c:\stock" if sys.platform == "win32" else os.path.expanduser('~') + "/stock")
        self.hdf5_dir_lineEdit.setText(hdf5_dir)

        self.on_tdx_or_pytdx_toggled()

    def getCurrentConfig(self):
        import_config = ConfigParser()
        import_config['quotation'] = {'stock': self.import_stock_checkBox.isChecked(),
                                      'fund': self.import_fund_checkBox.isChecked(),
                                      'future': self.import_future_checkBox.isChecked()}
        import_config['ktype'] = {'day': self.import_day_checkBox.isChecked(),
                                  'min': self.import_min_checkBox.isChecked(),
                                  'min5': self.import_min5_checkBox.isChecked(),
                                  'trans': self.import_trans_checkBox.isChecked(),
                                  'time': self.import_time_checkBox.isChecked(),
                                  'trans_max_days': self.trans_max_days_spinBox.value(),
                                  'time_max_days': self.time_max_days_spinBox.value()}
        import_config['weight'] = {'enable': self.import_weight_checkBox.isChecked(),}
        import_config['tdx'] = {'enable': self.tdx_radioButton.isChecked(),
                                'dir': self.tdx_dir_lineEdit.text()}
        import_config['pytdx'] = {'enable': self.pytdx_radioButton.isChecked()}
        import_config['hdf5'] = {'enable': True,
                                 'dir': self.hdf5_dir_lineEdit.text()}
        return import_config

    def initThreads(self):
        self.escape_time_thread = None
        self.hdf5_import_thread = None
        self.mysql_import_thread = None

        self.import_running = False
        self.hdf5_import_progress_bar = {'DAY': self.hdf5_day_progressBar,
                                         '1MIN': self.hdf5_min_progressBar,
                                         '5MIN': self.hdf5_5min_progressBar}

    def on_tdx_or_pytdx_toggled(self):
        tdx_enable = self.tdx_radioButton.isChecked()
        self.tdx_dir_lineEdit.setEnabled(tdx_enable)
        self.select_tdx_dir_pushButton.setEnabled(tdx_enable)
        self.import_trans_checkBox.setChecked(not tdx_enable)
        self.import_trans_checkBox.setEnabled(not tdx_enable)
        self.import_time_checkBox.setChecked(not tdx_enable)
        self.import_time_checkBox.setEnabled(not tdx_enable)
        self.trans_max_days_spinBox.setEnabled(not tdx_enable)
        self.time_max_days_spinBox.setEnabled(not tdx_enable)

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

    def reset_progress_bar(self):
        self.hdf5_weight_label.setText('')
        self.hdf5_day_progressBar.setValue(0)
        self.hdf5_min_progressBar.setValue(0)
        self.hdf5_5min_progressBar.setValue(0)
        self.hdf5_trans_progressBar.setValue(0)
        self.hdf5_time_progressBar.setValue(0)
        self.import_detail_textEdit.clear()

    def on_escapte_time(self, escape):
        self.import_status_label.setText("耗时：{:>.2f} 秒".format(escape))

    def on_message_from_thread(self, msg):
        if not msg or len(msg) < 2:
            print("msg is empty!")
            return

        msg_name, msg_task_name = msg[:2]
        if msg_name == 'ESCAPE_TIME':
            self.escape_time = msg_task_name
            self.import_status_label.setText("耗时：{:>.2f} 秒 （{:>.2f}分钟）".format(self.escape_time, self.escape_time/60))

        elif msg_name == 'HDF5_IMPORT':
            if msg_task_name == 'THREAD':
                status = msg[2]
                if status == 'FAILURE':
                    self.import_status_label.setText("耗时：{:>.2f} 秒 导入异常！".format(self.escape_time))
                    self.import_detail_textEdit.append(msg[3])
                self.hdf5_import_thread.terminate()
                self.hdf5_import_thread = None
                self.escape_time_thread.stop()
                self.escape_time_thread = None
                self.start_import_pushButton.setEnabled(True)
                self.import_detail_textEdit.append("导入完毕！")
                self.import_running = False

            elif msg_task_name == 'IMPORT_KDATA':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.hdf5_import_progress_bar[ktype].setValue(progress)
                else:
                    self.import_detail_textEdit.append('导入 {} {} 记录数：{}'
                                                       .format(msg[3], msg[4], msg[5]))

            elif msg_task_name == 'IMPORT_TRANS':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.hdf5_trans_progressBar.setValue(progress)
                else:
                    self.import_detail_textEdit.append('导入 {} 分笔记录数：{}'
                                                       .format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_TIME':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.hdf5_time_progressBar.setValue(progress)
                else:
                    self.import_detail_textEdit.append('导入 {} 分时记录数：{}'
                                                       .format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_WEIGHT':
                self.hdf5_weight_label.setText(msg[2])
                if msg[2] == '导入完成!':
                    self.import_detail_textEdit.append('导入权息记录数：{}'.format(msg[3]))


    @pyqtSlot()
    def on_start_import_pushButton_clicked(self):
        config = self.getCurrentConfig()
        dest_dir = config['hdf5']['dir']
        if not os.path.exists(dest_dir) or not os.path.isdir(dest_dir):
            QMessageBox.about(self, "错误", '指定的目标数据存放目录不存在！')
            return

        if config['tdx']['enable'] \
            and (not os.path.exists(config['tdx']['dir']
                 or os.path.isdir(config['tdx']['dir']))):
            QMessageBox.about(self, "错误", "请确认通达信安装目录是否正确！")
            return

        self.import_running = True
        self.start_import_pushButton.setEnabled(False)
        self.reset_progress_bar()

        self.import_status_label.setText("正在启动任务....")
        QApplication.processEvents()

        if self.tdx_radioButton.isChecked():
            self.hdf5_import_thread = UseTdxImportToH5Thread(config)
        else:
            self.import_status_label.setText("正在搜索通达信行情服务器....")
            QApplication.processEvents()
            hosts = search_best_tdx()
            if not hosts:
                self.import_status_label.setText("无法连接通达信行情服务器！请检查网络设置！")
                QMessageBox.about(self, "提示", "无法连接的通达信行情服务器！请检查网络设置！")
                self.import_running = False
                self.start_import_pushButton.setEnabled(True)
                return
            self.hdf5_import_thread = UsePytdxImportToH5Thread(config, hosts)

        self.hdf5_import_thread.message.connect(self.on_message_from_thread)
        self.hdf5_import_thread.start()

        self.escape_time = 0.0
        self.escape_time_thread = EscapetimeThread()
        self.escape_time_thread.message.connect(self.on_message_from_thread)
        self.escape_time_thread.start()


    @pyqtSlot()
    def on_ping_tdx_pushButton_clicked(self):
        pass

    @pyqtSlot()
    def on_search_tdx_pushButton_clicked(self):
        pass


if __name__ == "__main__":
    import sys
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
