# -*- coding: utf-8 -*-

import os
import sys
import shutil
import logging
import datetime
from configparser import ConfigParser

from PyQt5.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox
from PyQt5.QtCore import pyqtSlot, QObject, pyqtSignal
from PyQt5.QtGui import QIcon, QTextCursor

import mysql.connector
from mysql.connector import errorcode
from mysql.connector.locales.eng import client_error  #此句仅为pyinstaller打包时能够自动引入

from hikyuu.gui.data.MainWindow import *
from hikyuu.gui.data.EscapetimeThread import EscapetimeThread
from hikyuu.gui.data.UseTdxImportToH5Thread import UseTdxImportToH5Thread
from hikyuu.gui.data.UsePytdxImportToH5Thread import UsePytdxImportToH5Thread
from hikyuu.gui.data.CollectThread import CollectThread

from hikyuu.data import hku_config_template
from hikyuu.util.mylog import add_class_logger_handler, class_logger, hku_logger


class EmittingStream(QObject):
    """输出重定向至QT"""
    textWritten = pyqtSignal(str)

    def write(self, text):
        self.textWritten.emit(str(text))

    def flush(self):
        pass


class MyMainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, parent=None, capture_output=False, use_dark_style=False):
        super(MyMainWindow, self).__init__(parent)
        self._capture_output = capture_output  # 捕获Python stdout 输出
        self._use_dark_style = use_dark_style  # 使用暗黑主题
        self._text_color = '#FFFFFF' if use_dark_style else '#000000'
        self.setupUi(self)
        self.initUI()
        self.initLogger()
        self.initThreads()

    def closeEvent(self, event):
        self.stop_collect()
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
        if not os.path.lexists(self.getUserConfigDir()):
            os.mkdir(self.getUserConfigDir())

        current_config = self.getCurrentConfig()
        filename = self.getUserConfigDir() + '/importdata-gui.ini'
        with open(filename, 'w', encoding='utf-8') as f:
            current_config.write(f)

        filename = self.getUserConfigDir() + '/hikyuu.ini'
        if current_config.getboolean('hdf5', 'enable', fallback=True):
            data_dir = current_config['hdf5']['dir']
            if not os.path.lexists(data_dir + '/tmp'):
                os.mkdir(data_dir + '/tmp')

            # 此处不能使用 utf-8 参数，否则导致Windows下getBlock无法找到板块分类
            # with open(filename, 'w', encoding='utf-8') as f:
            with open(filename, 'w') as f:
                f.write(hku_config_template.hdf5_template.format(dir=data_dir))
        else:
            data_dir = current_config['mysql']['tmpdir']
            with open(filename, 'w') as f:
                f.write(
                    hku_config_template.mysql_template.format(
                        dir=data_dir,
                        host=current_config['mysql']['host'],
                        port=current_config['mysql']['port'],
                        usr=current_config['mysql']['usr'],
                        pwd=current_config['mysql']['pwd']
                    )
                )

        if not os.path.lexists(data_dir):
            os.makedirs(data_dir)

        if not os.path.lexists(data_dir + '/block'):
            current_dir = os.path.dirname(os.path.abspath(__file__))
            dirname, _ = os.path.split(current_dir)
            dirname = os.path.join(dirname, 'config/block')
            shutil.copytree(dirname, data_dir + '/block')
            os.remove(data_dir + '/block/__init__.py')

    def normalOutputWritten(self, text):
        """普通打印信息重定向"""
        if text.find('[DEBUG]') >= 0:
            text = '<font color="#66CC99">{}</font>'.format(text)
        elif text.find('[WARNING]') >= 0:
            text = '<font color="#0000FF">{}</font>'.format(text)
        elif text.find('[ERROR]') >= 0:
            text = '<font color="#FF0000">{}</font>'.format(text)
        elif text.find('[CRITICAL]') >= 0:
            text = '<span style="background-color: #ff0000;">{}</span>'.format(text)
        else:
            # 主动加入<font>标签，避免 append 时多加入空行
            text = '<font color="{}">{}</font>'.format(self._text_color, text)
        cursor = self.log_textEdit.textCursor()
        cursor.movePosition(QTextCursor.End)
        self.log_textEdit.append(text)

    def initLogger(self):
        if not self._capture_output:
            return

        #普通日志输出控制台
        con = logging.StreamHandler(EmittingStream(textWritten=self.normalOutputWritten))
        FORMAT = logging.Formatter(
            '%(asctime)-15s [%(levelname)s] - %(message)s [%(name)s::%(funcName)s]'
        )
        con.setFormatter(FORMAT)
        add_class_logger_handler(
            con, [MyMainWindow, CollectThread, UsePytdxImportToH5Thread, UseTdxImportToH5Thread],
            logging.INFO
        )
        hku_logger.addHandler(con)

    def initUI(self):
        if self._capture_output:
            stream = EmittingStream(textWritten=self.normalOutputWritten)
            sys.stdout = stream
            sys.stderr = stream
        self.log_textEdit.document().setMaximumBlockCount(1000)

        current_dir = os.path.dirname(__file__)
        self.setWindowIcon(QIcon("{}/hikyuu.ico".format(current_dir)))
        #self.setFixedSize(self.width(), self.height())
        self.import_status_label.setText('')
        self.import_detail_textEdit.clear()
        self.reset_progress_bar()
        self.day_start_dateEdit.setMinimumDate(datetime.date(1990, 12, 19))
        self.day_start_dateEdit.setDate(datetime.date(1990, 12, 19))
        today = datetime.date.today()
        self.min_start_dateEdit.setDate(today - datetime.timedelta(90))
        self.min5_start_dateEdit.setDate(today - datetime.timedelta(90))
        self.min_start_dateEdit.setMinimumDate(datetime.date(1990, 12, 19))
        self.min5_start_dateEdit.setMinimumDate(datetime.date(1990, 12, 19))
        self.trans_start_dateEdit.setDate(today - datetime.timedelta(7))
        self.time_start_dateEdit.setDate(today - datetime.timedelta(7))
        self.trans_start_dateEdit.setMinimumDate(today - datetime.timedelta(90))
        self.time_start_dateEdit.setMinimumDate(today - datetime.timedelta(300))
        self.collect_running = False
        self.collect_status_Label.setText("未启动")

        #读取保存的配置文件信息，如果不存在，则使用默认配置
        this_dir = self.getUserConfigDir()
        import_config = ConfigParser()
        if os.path.exists(this_dir + '/importdata-gui.ini'):
            import_config.read(this_dir + '/importdata-gui.ini', encoding='utf-8')

        #初始化导入行情数据类型配置
        self.import_stock_checkBox.setChecked(
            import_config.getboolean('quotation', 'stock', fallback=True)
        )
        self.import_fund_checkBox.setChecked(
            import_config.getboolean('quotation', 'fund', fallback=True)
        )
        self.import_future_checkBox.setChecked(
            import_config.getboolean('quotation', 'future', fallback=False)
        )

        #初始化导入K线类型配置
        self.import_day_checkBox.setChecked(import_config.getboolean('ktype', 'day', fallback=True))
        self.import_min_checkBox.setChecked(import_config.getboolean('ktype', 'min', fallback=True))
        self.import_min5_checkBox.setChecked(
            import_config.getboolean('ktype', 'min5', fallback=True)
        )
        self.import_trans_checkBox.setChecked(
            import_config.getboolean('ktype', 'trans', fallback=False)
        )
        self.import_time_checkBox.setChecked(
            import_config.getboolean('ktype', 'time', fallback=False)
        )
        #self.trans_max_days_spinBox.setValue(import_config.getint('ktype', 'trans_max_days', fallback=70))
        #self.time_max_days_spinBox.setValue(import_config.getint('ktype', 'time_max_days', fallback=70))

        #初始化权息与财务数据设置
        self.import_weight_checkBox.setChecked(
            import_config.getboolean('weight', 'enable', fallback=True)
        )

        #初始化通道信目录配置
        tdx_enable = import_config.getboolean('tdx', 'enable', fallback=False)
        tdx_dir = import_config.get('tdx', 'dir', fallback='d:\TdxW_HuaTai')
        self.tdx_radioButton.setChecked(tdx_enable)
        self.tdx_dir_lineEdit.setEnabled(tdx_enable)
        self.select_tdx_dir_pushButton.setEnabled(tdx_enable)
        self.tdx_dir_lineEdit.setText(tdx_dir)

        #初始化pytdx配置及显示
        self.pytdx_radioButton.setChecked(
            import_config.getboolean('pytdx', 'enable', fallback=True)
        )
        self.use_tdx_number_spinBox.setValue(
            import_config.getint('pytdx', 'use_tdx_number', fallback=10)
        )

        self.on_tdx_or_pytdx_toggled()

        #初始化hdf5设置
        hdf5_enable = import_config.getboolean('hdf5', 'enable', fallback=True)
        self.enable_hdf55_radioButton.setChecked(hdf5_enable)
        hdf5_dir = import_config.get(
            'hdf5',
            'dir',
            fallback="c:\stock" if sys.platform == "win32" else os.path.expanduser('~') + "/stock"
        )
        self.hdf5_dir_lineEdit.setText(hdf5_dir)
        self.hdf5_dir_lineEdit.setEnabled(hdf5_enable)

        #初始化MYSQL设置
        mysql_enable = import_config.getboolean('mysql', 'enable', fallback=False)
        if hdf5_enable:
            mysql_enable = False
        self.enable_mysql_radioButton.setChecked(mysql_enable)
        self.mysql_tmpdir_lineEdit.setText(
            import_config.get('mysql', 'tmpdir', fallback='c:\stock')
        )
        mysql_ip = import_config.get('mysql', 'host', fallback='127.0.0.1')
        self.mysql_ip_lineEdit.setText(mysql_ip)
        self.mysql_ip_lineEdit.setEnabled(mysql_enable)
        mysql_port = import_config.get('mysql', 'port', fallback='3306')
        self.mysql_port_lineEdit.setText(mysql_port)
        self.mysql_port_lineEdit.setEnabled(mysql_enable)
        mysql_usr = import_config.get('mysql', 'usr', fallback='root')
        self.mysql_usr_lineEdit.setText(mysql_usr)
        self.mysql_usr_lineEdit.setEnabled(mysql_enable)
        mysql_pwd = import_config.get('mysql', 'pwd', fallback='')
        self.mysql_pwd_lineEdit.setText(mysql_pwd)
        self.mysql_pwd_lineEdit.setEnabled(mysql_enable)
        self.mysql_test_pushButton.setEnabled(mysql_enable)

        # 初始化定时采集设置
        interval_time = import_config.getint('collect', 'interval', fallback=60 * 60)
        self.collect_sample_spinBox.setValue(interval_time)
        use_zhima_proxy = import_config.getboolean('collect', 'use_zhima_proxy', fallback=False)
        self.collect_use_zhima_checkBox.setChecked(use_zhima_proxy)
        self.collect_phase1_start_timeEdit.setTime(
            datetime.time.fromisoformat(
                import_config.get('collect', 'phase1_start', fallback='09:05')
            )
        )
        self.collect_phase1_last_timeEdit.setTime(
            datetime.time.fromisoformat(
                import_config.get('collect', 'phase1_end', fallback='15:15')
            )
        )

    def getCurrentConfig(self):
        import_config = ConfigParser()
        import_config['quotation'] = {
            'stock': self.import_stock_checkBox.isChecked(),
            'fund': self.import_fund_checkBox.isChecked(),
            'future': self.import_future_checkBox.isChecked()
        }
        import_config['ktype'] = {
            'day': self.import_day_checkBox.isChecked(),
            'min': self.import_min_checkBox.isChecked(),
            'min5': self.import_min5_checkBox.isChecked(),
            'trans': self.import_trans_checkBox.isChecked(),
            'time': self.import_time_checkBox.isChecked(),
            'day_start_date': self.day_start_dateEdit.date().toString('yyyy-MM-dd'),
            'min_start_date': self.min_start_dateEdit.date().toString('yyyy-MM-dd'),
            'min5_start_date': self.min5_start_dateEdit.date().toString('yyyy-MM-dd'),
            'trans_start_date': self.trans_start_dateEdit.date().toString('yyyy-MM-dd'),
            'time_start_date': self.time_start_dateEdit.date().toString('yyyy-MM-dd')
        }
        import_config['weight'] = {
            'enable': self.import_weight_checkBox.isChecked(),
        }
        import_config['tdx'] = {
            'enable': self.tdx_radioButton.isChecked(),
            'dir': self.tdx_dir_lineEdit.text()
        }
        import_config['pytdx'] = {
            'enable': self.pytdx_radioButton.isChecked(),
            'use_tdx_number': self.use_tdx_number_spinBox.value()
        }
        import_config['hdf5'] = {
            'enable': self.enable_hdf55_radioButton.isChecked(),
            'dir': self.hdf5_dir_lineEdit.text()
        }
        import_config['mysql'] = {
            'enable': self.enable_mysql_radioButton.isChecked(),
            'tmpdir': self.mysql_tmpdir_lineEdit.text(),
            'host': self.mysql_ip_lineEdit.text(),
            'port': self.mysql_port_lineEdit.text(),
            'usr': self.mysql_usr_lineEdit.text(),
            'pwd': self.mysql_pwd_lineEdit.text()
        }
        import_config['collect'] = {
            'interval': self.collect_sample_spinBox.value(),
            'use_zhima_proxy': self.collect_use_zhima_checkBox.isChecked(),
            'phase1_start': self.collect_phase1_start_timeEdit.time().toString(),
            'phase1_end': self.collect_phase1_last_timeEdit.time().toString()
        }
        return import_config

    def initThreads(self):
        self.escape_time_thread = None
        self.hdf5_import_thread = None
        self.mysql_import_thread = None
        self.collect_sh_thread = None
        self.collect_sz_thread = None

        self.import_running = False
        self.hdf5_import_progress_bar = {
            'DAY': self.hdf5_day_progressBar,
            '1MIN': self.hdf5_min_progressBar,
            '5MIN': self.hdf5_5min_progressBar
        }

    @pyqtSlot()
    def on_pytdx_radioButton_clicked(self):
        if self.pytdx_radioButton.isChecked():
            self.tdx_radioButton.setChecked(False)
        self.on_tdx_or_pytdx_toggled()

    @pyqtSlot()
    def on_tdx_radioButton_clicked(self):
        if self.tdx_radioButton.isChecked():
            self.pytdx_radioButton.setChecked(False)
        self.on_tdx_or_pytdx_toggled()

    def on_tdx_or_pytdx_toggled(self):
        tdx_enable = self.tdx_radioButton.isChecked()
        self.tdx_dir_lineEdit.setEnabled(tdx_enable)
        self.select_tdx_dir_pushButton.setEnabled(tdx_enable)
        self.import_trans_checkBox.setEnabled(not tdx_enable)
        self.import_time_checkBox.setEnabled(not tdx_enable)
        self.trans_start_dateEdit.setEnabled(not tdx_enable)
        self.time_start_dateEdit.setEnabled(not tdx_enable)
        self.use_tdx_number_spinBox.setEnabled(not tdx_enable)

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
    def on_hdf5_dir_pushButton_clicked(self):
        dlg = QFileDialog()
        dlg.setFileMode(QFileDialog.Directory)
        config = self.getCurrentConfig()
        dlg.setDirectory(config['hdf5']['dir'])
        if dlg.exec_():
            dirname = dlg.selectedFiles()
            self.hdf5_dir_lineEdit.setText(dirname[0])

    @pyqtSlot()
    def on_enable_hdf55_radioButton_clicked(self):
        if self.enable_hdf55_radioButton.isChecked():
            self.enable_mysql_radioButton.setChecked(False)
        self.on_enable_hdf5_or_mysql_toggled()

    @pyqtSlot()
    def on_enable_mysql_radioButton_clicked(self):
        if self.enable_mysql_radioButton.isChecked():
            self.enable_hdf55_radioButton.setChecked(False)
        self.on_enable_hdf5_or_mysql_toggled()

    def on_enable_hdf5_or_mysql_toggled(self):
        hdf5_enable = self.enable_hdf55_radioButton.isChecked()
        mysql_enable = not hdf5_enable
        self.hdf5_dir_lineEdit.setEnabled(hdf5_enable)
        self.mysql_ip_lineEdit.setEnabled(mysql_enable)
        self.mysql_port_lineEdit.setEnabled(mysql_enable)
        self.mysql_usr_lineEdit.setEnabled(mysql_enable)
        self.mysql_pwd_lineEdit.setEnabled(mysql_enable)
        self.mysql_test_pushButton.setEnabled(mysql_enable)

    @pyqtSlot()
    def on_mysql_tmpdir_pushButton_clicked(self):
        dlg = QFileDialog()
        dlg.setFileMode(QFileDialog.Directory)
        config = self.getCurrentConfig()
        dlg.setDirectory(config['mysql']['tmpdir'])
        if dlg.exec_():
            dirname = dlg.selectedFiles()
            self.mysql_tmpdir_lineEdit.setText(dirname[0])

    @pyqtSlot()
    def on_mysql_test_pushButton_clicked(self):
        """测试数据库连接"""
        db_config = {
            'user': self.mysql_usr_lineEdit.text(),
            'password': self.mysql_pwd_lineEdit.text(),
            'host': self.mysql_ip_lineEdit.text(),
            'port': self.mysql_port_lineEdit.text()
        }

        try:
            cnx = mysql.connector.connect(**db_config)
            cnx.close()
        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
                QMessageBox.critical(self, "测试数据库连接", "MYSQL密码或用户名错误！")
            elif err.errno == errorcode.ER_BAD_DB_ERROR:
                QMessageBox.critical(self, "测试数据库连接", "MySQL数据库不存在！")
            else:
                QMessageBox.critical(self, "测试数据库连接", err.msg)
            return

        QMessageBox.about(self, "测试数据库连接", " 连接成功！")

    def reset_progress_bar(self):
        self.hdf5_weight_label.setText('')
        self.hdf5_day_progressBar.setValue(0)
        self.hdf5_min_progressBar.setValue(0)
        self.hdf5_5min_progressBar.setValue(0)
        self.hdf5_trans_progressBar.setValue(0)
        self.hdf5_time_progressBar.setValue(0)
        #self.finance_progressBar.setValue(0)
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
            self.import_status_label.setText(
                "耗时：{:>.2f} 秒 （{:>.2f}分钟）".format(self.escape_time, self.escape_time / 60)
            )

        elif msg_name == 'HDF5_IMPORT':
            if msg_task_name == 'INFO':
                self.import_detail_textEdit.append(msg[2])

            elif msg_task_name == 'THREAD':
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
                    self.import_detail_textEdit.append(
                        '导入 {} {} 记录数：{}'.format(msg[3], msg[4], msg[5])
                    )

            elif msg_task_name == 'IMPORT_TRANS':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.hdf5_trans_progressBar.setValue(progress)
                else:
                    self.import_detail_textEdit.append('导入 {} 分笔记录数：{}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_TIME':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.hdf5_time_progressBar.setValue(progress)
                else:
                    self.import_detail_textEdit.append('导入 {} 分时记录数：{}'.format(msg[3], msg[5]))

            elif msg_task_name == 'IMPORT_WEIGHT':
                self.hdf5_weight_label.setText(msg[2])
                if msg[2] == '导入权息数据完毕!':
                    self.import_detail_textEdit.append('导入权息记录数：{}'.format(msg[3]))
                #elif msg[2] == '导入通达信财务信息完毕!':
                #    self.import_detail_textEdit.append('导入通达信财务记录数：{}'.format(msg[3]))

            #elif msg_task_name == 'IMPORT_FINANCE':
            #    if msg[2] != 'FINISHED':
            #        self.finance_progressBar.setValue(msg[2])

    @pyqtSlot()
    def on_start_import_pushButton_clicked(self):
        config = self.getCurrentConfig()
        dest_dir = config.get('hdf5', 'dir')
        if not os.path.exists(dest_dir) or not os.path.isdir(dest_dir):
            QMessageBox.about(self, "错误", '指定的目标数据存放目录不存在！')
            return

        if config.getboolean('tdx', 'enable') \
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
            self.hdf5_import_thread = UsePytdxImportToH5Thread(config)

        self.hdf5_import_thread.message.connect(self.on_message_from_thread)
        self.hdf5_import_thread.start()

        self.escape_time = 0.0
        self.escape_time_thread = EscapetimeThread()
        self.escape_time_thread.message.connect(self.on_message_from_thread)
        self.escape_time_thread.start()

    def start_collect(self):
        self.collect_sh_thread = CollectThread(self.getCurrentConfig(), 'SH')
        self.collect_sh_thread.start()
        self.collect_sz_thread = CollectThread(self.getCurrentConfig(), 'SZ')
        self.collect_sz_thread.start()

    def stop_collect(self):
        self.logger.info("终止采集！")
        if self.collect_sh_thread is not None:
            self.collect_sh_thread.stop()
        if self.collect_sz_thread is not None:
            self.collect_sz_thread.stop()

    @pyqtSlot()
    def on_collect_start_pushButton_clicked(self):
        if self.collect_running:
            self.collect_start_pushButton.setEnabled(False)
            self.collect_status_Label.setText("正在停止...")
            QApplication.processEvents()
            self.stop_collect()
            self.collect_start_pushButton.setText("启动定时采集")
            self.collect_running = False
            self.collect_status_Label.setText("已停止")
            self.collect_start_pushButton.setEnabled(True)
        else:
            config = self.getCurrentConfig()
            if not config.getboolean("mysql", "enable", fallback=False):
                QMessageBox.critical(self, "定时采集", "仅在存储设置为 MySQL 时支持定时采集！")
                return
            self.collect_status_Label.setText("正在启动...")
            self.collect_start_pushButton.setEnabled(False)
            QApplication.processEvents()
            self.start_collect()
            self.collect_start_pushButton.setText("停止采集")
            self.collect_running = True
            self.collect_status_Label.setText("运行中...")
            self.collect_start_pushButton.setEnabled(True)


class_logger(MyMainWindow)


def start():
    app = QApplication(sys.argv)
    myWin = MyMainWindow(None)
    myWin.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    import requests
    import urllib
    logging.getLogger("requests").setLevel(logging.WARNING)
    logging.getLogger("urllib3").setLevel(logging.WARNING)
    app = QApplication(sys.argv)
    use_dark_style = False  # 使用暗黑主题
    if use_dark_style:
        import qdarkstyle
        app.setStyleSheet(qdarkstyle.load_stylesheet(qt_api='pyqt5'))
    if (len(sys.argv) > 1 and sys.argv[1] == '0'):
        FORMAT = '%(asctime)-15s [%(levelname)s]: %(message)s [%(name)s::%(funcName)s]'
        logging.basicConfig(
            format=FORMAT, level=logging.INFO, handlers=[
                logging.StreamHandler(),
            ]
        )
        #myWin = MyMainWindow(capture_output=True, use_dark_style=use_dark_style)
        myWin = MyMainWindow(capture_output=False, use_dark_style=use_dark_style)
    else:
        myWin = MyMainWindow(capture_output=True, use_dark_style=use_dark_style)

    myWin.show()
    sys.exit(app.exec())
