# -*- coding: utf-8 -*-

import os
import sys
import shutil
import logging
import datetime
import multiprocessing
from configparser import ConfigParser
from logging.handlers import QueueListener

# 优先加载，处理 VS 17.10 升级后依赖 dll 不兼容问题
import hikyuu

import PyQt5

from PyQt5.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox
from PyQt5.QtCore import pyqtSlot, QObject, pyqtSignal
from PyQt5.QtGui import QIcon, QTextCursor, QFont

import mysql.connector
from mysql.connector import errorcode
from mysql.connector.locales.eng import client_error  # 此句仅为pyinstaller打包时能够自动引入

from hikyuu.gui.data.MainWindow import *
from hikyuu.gui.data.EscapetimeThread import EscapetimeThread
from hikyuu.gui.data.UseTdxImportToH5Thread import UseTdxImportToH5Thread
from hikyuu.gui.data.ImportTdxToH5Task import ImportTdxToH5Task
from hikyuu.gui.data.UsePytdxImportToH5Thread import UsePytdxImportToH5Thread
# from hikyuu.gui.data.CollectToMySQLThread import CollectToMySQLThread
# from hikyuu.gui.data.CollectToMemThread import CollectToMemThread
from hikyuu.gui.data.CollectSpotThread import CollectSpotThread
from hikyuu.gui.data.SchedImportThread import SchedImportThread
from hikyuu.gui.spot_server import release_nng_senders

from hikyuu import can_upgrade, get_last_version
from hikyuu.data import hku_config_template
from hikyuu.util import *


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
        if self.import_running:
            QMessageBox.about(self, '提示', '正在执行导入任务，请耐心等候！')
            event.ignore()
            return

        if self.sched_import_thread is not None and self.sched_import_thread.isRunning():
            self.sched_import_thread.terminate()
            self.collect_spot_thread.wait()

        if self.collect_spot_thread is not None and self.collect_spot_thread.isRunning():
            release_nng_senders()
            self.collect_spot_thread.terminate()
            self.collect_spot_thread.wait()

        self.saveConfig()

        if self.hdf5_import_thread:
            self.hdf5_import_thread.stop()
        if self.escape_time_thread:
            self.escape_time_thread.stop()
        if hasattr(self, 'mp_log_q_lisener'):
            self.mp_log_q_lisener.stop()
        event.accept()

    def getUserConfigDir(self):
        return os.path.expanduser('~') + '/.hikyuu'

    def getHikyuuConfigFileName(self):
        return self.getUserConfigDir() + '/hikyuu.ini'

    def saveConfig(self):
        if not os.path.lexists(self.getUserConfigDir()):
            os.mkdir(self.getUserConfigDir())

        current_config = self.getCurrentConfig()
        filename = self.getUserConfigDir() + '/importdata-gui.ini'
        with open(filename, 'w', encoding='utf-8') as f:
            current_config.write(f)
        filename = self.getHikyuuConfigFileName()
        if current_config.getboolean('hdf5', 'enable', fallback=True):
            data_dir = current_config['hdf5']['dir']
            if not os.path.lexists(data_dir + '/tmp'):
                os.mkdir(data_dir + '/tmp')
            # 此处不能使用 utf-8 参数，否则导致Windows下getBlock无法找到板块分类
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(
                    hku_config_template.hdf5_template.format(
                        dir=data_dir,
                        quotation_server=current_config.get(
                            'collect', 'quotation_server', fallback='ipc:///tmp/hikyuu_real.ipc'),
                        day=current_config.getboolean('preload', 'day', fallback=True),
                        week=current_config.getboolean('preload', 'week', fallback=False),
                        month=current_config.getboolean('preload', 'month', fallback=False),
                        quarter=current_config.getboolean('preload', 'quarter', fallback=False),
                        halfyear=current_config.getboolean('preload', 'halfyear', fallback=False),
                        year=current_config.getboolean('preload', 'year', fallback=False),
                        min1=current_config.getboolean('preload', 'min', fallback=False),
                        min5=current_config.getboolean('preload', 'min5', fallback=False),
                        min15=current_config.getboolean('preload', 'min15', fallback=False),
                        min30=current_config.getboolean('preload', 'min30', fallback=False),
                        min60=current_config.getboolean('preload', 'min60', fallback=False),
                        hour2=current_config.getboolean('preload', 'hour2', fallback=False),
                        day_max=current_config.getint('preload', 'day_max', fallback=100000),
                        week_max=current_config.getint('preload', 'week_max', fallback=100000),
                        month_max=current_config.getint('preload', 'month_max', fallback=100000),
                        quarter_max=current_config.getint('preload', 'quarter_max', fallback=100000),
                        halfyear_max=current_config.getint('preload', 'halfyear_max', fallback=100000),
                        year_max=current_config.getint('preload', 'year_max', fallback=100000),
                        min1_max=current_config.getint('preload', 'min_max', fallback=4096),
                        min5_max=current_config.getint('preload', 'min5_max', fallback=4096),
                        min15_max=current_config.getint('preload', 'min15_max', fallback=4096),
                        min30_max=current_config.getint('preload', 'min30_max', fallback=4096),
                        min60_max=current_config.getint('preload', 'min60_max', fallback=4096),
                        hour2_max=current_config.getint('preload', 'hour2_max', fallback=4096),
                    )
                )

        else:
            data_dir = current_config['mysql']['tmpdir']
            with open(filename, 'w', encoding="utf-8") as f:
                f.write(
                    hku_config_template.mysql_template.format(
                        dir=data_dir,
                        quotation_server=current_config.get(
                            'collect', 'quotation_server', fallback='ipc:///tmp/hikyuu_real.ipc'),
                        host=current_config['mysql']['host'],
                        port=current_config['mysql']['port'],
                        usr=current_config['mysql']['usr'],
                        pwd=current_config['mysql']['pwd'],
                        day=current_config.getboolean('preload', 'day', fallback=True),
                        week=current_config.getboolean('preload', 'week', fallback=False),
                        month=current_config.getboolean('preload', 'month', fallback=False),
                        quarter=current_config.getboolean('preload', 'quarter', fallback=False),
                        halfyear=current_config.getboolean('preload', 'halfyear', fallback=False),
                        year=current_config.getboolean('preload', 'year', fallback=False),
                        min1=current_config.getboolean('preload', 'min', fallback=False),
                        min5=current_config.getboolean('preload', 'min5', fallback=False),
                        min15=current_config.getboolean('preload', 'min15', fallback=False),
                        min30=current_config.getboolean('preload', 'min30', fallback=False),
                        min60=current_config.getboolean('preload', 'min60', fallback=False),
                        hour2=current_config.getboolean('preload', 'hour2', fallback=False),
                        day_max=current_config.getint('preload', 'day_max', fallback=100000),
                        week_max=current_config.getint('preload', 'week_max', fallback=100000),
                        month_max=current_config.getint('preload', 'month_max', fallback=100000),
                        quarter_max=current_config.getint('preload', 'quarter_max', fallback=100000),
                        halfyear_max=current_config.getint('preload', 'halfyear_max', fallback=100000),
                        year_max=current_config.getint('preload', 'year_max', fallback=100000),
                        min1_max=current_config.getint('preload', 'min_max', fallback=4096),
                        min5_max=current_config.getint('preload', 'min5_max', fallback=4096),
                        min15_max=current_config.getint('preload', 'min15_max', fallback=4096),
                        min30_max=current_config.getint('preload', 'min30_max', fallback=4096),
                        min60_max=current_config.getint('preload', 'min60_max', fallback=4096),
                        hour2_max=current_config.getint('preload', 'hour2_max', fallback=4096),
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

    @pyqtSlot()
    def on_save_pushButton_clicked(self):
        try:
            self.saveConfig()
            QMessageBox.about(self, '', '保存成功')
        except Exception as e:
            QMessageBox.about(self, "错误", str(e))

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

        # 普通日志输出控制台
        if self._stream is None:
            self._stream = EmittingStream(textWritten=self.normalOutputWritten)
        self.log_handler = logging.StreamHandler(self._stream)
        FORMAT = logging.Formatter('%(asctime)-15s [%(levelname)s] - %(message)s [%(name)s::%(funcName)s]')
        self.log_handler.setFormatter(FORMAT)
        add_class_logger_handler(
            [
                MyMainWindow,
                CollectSpotThread,  # CollectToMySQLThread, CollectToMemThread,
                UsePytdxImportToH5Thread,
                UseTdxImportToH5Thread,
                ImportTdxToH5Task,
                SchedImportThread
            ],
            logging.INFO
        )
        for name in logging.Logger.manager.loggerDict.keys():
            logger = logging.getLogger(name)
            logger.addHandler(self.log_handler)
            # logger.setLevel(logging.DEBUG)
            logger.setLevel(logging.INFO)

        # 多进程日志队列
        self.mp_log_q = multiprocessing.Queue()
        self.mp_log_q_lisener = QueueListener(self.mp_log_q, self.log_handler)
        self.mp_log_q_lisener.start()

    def initUI(self):
        # 读取配置文件放在 output 重定向之前，防止配置文件读取失败没有提示
        # 读取保存的配置文件信息，如果不存在，则使用默认配置
        this_dir = self.getUserConfigDir()
        import_config = ConfigParser()
        if os.path.exists(this_dir + '/importdata-gui.ini'):
            import_config.read(this_dir + '/importdata-gui.ini', encoding='utf-8')

        self._is_sched_import_running = False
        self._is_collect_running = False
        self._stream = None
        if self._capture_output:
            self._stream = EmittingStream(textWritten=self.normalOutputWritten)
            if self._stream is not None:
                sys.stdout = self._stream
                # python构建时丢失stderr通道，导致安装后的hikyuutdx执行时，
                # logging总是报 'NoneType' object has no attribute 'write'
                if sys.stderr:
                    sys.stderr = self._stream
        self.log_textEdit.document().setMaximumBlockCount(1000)

        current_dir = os.path.dirname(__file__)
        self.setWindowIcon(QIcon("{}/hikyuu.ico".format(current_dir)))
        # self.setFixedSize(self.width(), self.height())
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
        self.collect_status_label.setText("已停止")

        # 初始化导入行情数据类型配置
        self.import_stock_checkBox.setChecked(import_config.getboolean('quotation', 'stock', fallback=True))
        self.import_fund_checkBox.setChecked(import_config.getboolean('quotation', 'fund', fallback=True))
        self.import_future_checkBox.setChecked(import_config.getboolean('quotation', 'future', fallback=False))

        # 初始化导入K线类型配置
        self.import_day_checkBox.setChecked(import_config.getboolean('ktype', 'day', fallback=True))
        self.import_min_checkBox.setChecked(import_config.getboolean('ktype', 'min', fallback=True))
        self.import_min5_checkBox.setChecked(import_config.getboolean('ktype', 'min5', fallback=True))
        self.import_trans_checkBox.setChecked(import_config.getboolean('ktype', 'trans', fallback=False))
        self.import_time_checkBox.setChecked(import_config.getboolean('ktype', 'time', fallback=False))
        # self.trans_max_days_spinBox.setValue(import_config.getint('ktype', 'trans_max_days', fallback=70))
        # self.time_max_days_spinBox.setValue(import_config.getint('ktype', 'time_max_days', fallback=70))

        # 初始化权息与财务数据设置
        self.import_weight_checkBox.setChecked(import_config.getboolean('weight', 'enable', fallback=True))

        # 初始化通道信目录配置
        tdx_enable = import_config.getboolean('tdx', 'enable', fallback=False)
        tdx_dir = import_config.get('tdx', 'dir', fallback='d:/TdxW_HuaTai')
        self.tdx_radioButton.setChecked(tdx_enable)
        self.tdx_dir_lineEdit.setEnabled(tdx_enable)
        self.select_tdx_dir_pushButton.setEnabled(tdx_enable)
        self.tdx_dir_lineEdit.setText(tdx_dir)

        # 初始化pytdx配置及显示
        self.pytdx_radioButton.setChecked(import_config.getboolean('pytdx', 'enable', fallback=True))
        self.use_tdx_number_spinBox.setValue(import_config.getint('pytdx', 'use_tdx_number', fallback=10))

        self.on_tdx_or_pytdx_toggled()

        # 初始化hdf5设置
        hdf5_enable = import_config.getboolean('hdf5', 'enable', fallback=True)
        self.enable_hdf55_radioButton.setChecked(hdf5_enable)
        hdf5_dir = import_config.get(
            'hdf5', 'dir', fallback="c:/stock" if sys.platform == "win32" else os.path.expanduser('~') + "/stock"
        )
        self.hdf5_dir_lineEdit.setText(hdf5_dir)
        self.hdf5_dir_lineEdit.setEnabled(hdf5_enable)

        # 初始化MYSQL设置
        mysql_enable = import_config.getboolean('mysql', 'enable', fallback=False)
        if hdf5_enable:
            mysql_enable = False
        self.enable_mysql_radioButton.setChecked(mysql_enable)
        self.mysql_tmpdir_lineEdit.setText(import_config.get('mysql', 'tmpdir', fallback='c:/stock'))
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

        self.sched_import_timeEdit.setTime(
            datetime.time.fromisoformat(import_config.get('schec', 'time', fallback='18:00'))
        )

        # 初始化定时采集设置
        self.quotation_server = import_config.get(
            'collect', 'quotation_server', fallback='ipc:///tmp/hikyuu_real.ipc')
        hku_warn_if(len(self.quotation_server) < 3, "Invalid quotation server addr!")
        if self.quotation_server[:3] == "ipc":
            self.quotation_server = 'ipc:///tmp/hikyuu_real.ipc'
        interval_time = import_config.getint('collect', 'interval', fallback=60 * 60)
        self.collect_sample_spinBox.setValue(interval_time)
        use_zhima_proxy = import_config.getboolean('collect', 'use_zhima_proxy', fallback=False)
        self.collect_use_zhima_checkBox.setChecked(use_zhima_proxy)
        # data_source = import_config.get('collect', 'source', fallback='qq')
        self.collect_source_comboBox.setCurrentIndex(0)
        self.collect_phase1_start_timeEdit.setTime(
            datetime.time.fromisoformat(import_config.get('collect', 'phase1_start', fallback='09:00'))
        )
        self.collect_phase1_last_timeEdit.setTime(
            datetime.time.fromisoformat(import_config.get('collect', 'phase1_end', fallback='12:05'))
        )
        self.collect_phase2_start_timeEdit.setTime(
            datetime.time.fromisoformat(import_config.get('collect', 'phase2_start', fallback='13:00'))
        )
        self.collect_phase2_last_timeEdit.setTime(
            datetime.time.fromisoformat(import_config.get('collect', 'phase2_end', fallback='15:05'))
        )

        # 预加载设置
        self.preload_day_checkBox.setChecked(import_config.getboolean('preload', 'day', fallback=True))
        self.preload_week_checkBox.setChecked(import_config.getboolean('preload', 'week', fallback=False))
        self.preload_month_checkBox.setChecked(import_config.getboolean('preload', 'month', fallback=False))
        self.preload_quarter_checkBox.setChecked(import_config.getboolean('preload', 'quarter', fallback=False))
        self.preload_halfyear_checkBox.setChecked(import_config.getboolean('preload', 'halfyear', fallback=False))
        self.preload_year_checkBox.setChecked(import_config.getboolean('preload', 'year', fallback=False))
        self.preload_min1_checkBox.setChecked(import_config.getboolean('preload', 'min', fallback=False))
        self.preload_min5_checkBox.setChecked(import_config.getboolean('preload', 'min5', fallback=False))
        self.preload_min15_checkBox.setChecked(import_config.getboolean('preload', 'min15', fallback=False))
        self.preload_min30_checkBox.setChecked(import_config.getboolean('preload', 'min30', fallback=False))
        self.preload_min60_checkBox.setChecked(import_config.getboolean('preload', 'min60', fallback=False))
        self.preload_hour2_checkBox.setChecked(import_config.getboolean('preload', 'hour2', fallback=False))
        self.preload_day_spinBox.setValue(import_config.getint('preload', 'day_max', fallback=100000))
        self.preload_week_spinBox.setValue(import_config.getint('preload', 'week_max', fallback=100000))
        self.preload_month_spinBox.setValue(import_config.getint('preload', 'month_max', fallback=100000))
        self.preload_quarter_spinBox.setValue(import_config.getint('preload', 'quarter_max', fallback=100000))
        self.preload_halfyear_spinBox.setValue(import_config.getint('preload', 'halfyear_max', fallback=100000))
        self.preload_year_spinBox.setValue(import_config.getint('preload', 'year_max', fallback=100000))
        self.preload_min1_spinBox.setValue(import_config.getint('preload', 'min_max', fallback=5120))
        self.preload_min5_spinBox.setValue(import_config.getint('preload', 'min5_max', fallback=5120))
        self.preload_min15_spinBox.setValue(import_config.getint('preload', 'min15_max', fallback=5120))
        self.preload_min30_spinBox.setValue(import_config.getint('preload', 'min30_max', fallback=5120))
        self.preload_min60_spinBox.setValue(import_config.getint('preload', 'min60_max', fallback=5120))
        self.preload_hour2_spinBox.setValue(import_config.getint('preload', 'hour2_max', fallback=5120))

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
        import_config['tdx'] = {'enable': self.tdx_radioButton.isChecked(), 'dir': self.tdx_dir_lineEdit.text()}
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
        import_config['sched'] = {
            'time': self.sched_import_timeEdit.time().toString(),
        }
        import_config['collect'] = {
            'quotation_server': self.quotation_server,
            'interval': self.collect_sample_spinBox.value(),
            'source': self.collect_source_comboBox.currentText(),
            'use_zhima_proxy': self.collect_use_zhima_checkBox.isChecked(),
            'phase1_start': self.collect_phase1_start_timeEdit.time().toString(),
            'phase1_end': self.collect_phase1_last_timeEdit.time().toString(),
            'phase2_start': self.collect_phase2_start_timeEdit.time().toString(),
            'phase2_end': self.collect_phase2_last_timeEdit.time().toString(),
        }
        import_config['preload'] = {
            'day': self.preload_day_checkBox.isChecked(),
            'week': self.preload_week_checkBox.isChecked(),
            'month': self.preload_month_checkBox.isChecked(),
            'quarter': self.preload_quarter_checkBox.isChecked(),
            'halfyear': self.preload_halfyear_checkBox.isChecked(),
            'year': self.preload_year_checkBox.isChecked(),
            'min': self.preload_min1_checkBox.isChecked(),
            'min5': self.preload_min5_checkBox.isChecked(),
            'min15': self.preload_min15_checkBox.isChecked(),
            'min30': self.preload_min30_checkBox.isChecked(),
            'min60': self.preload_min60_checkBox.isChecked(),
            'hour2': self.preload_hour2_checkBox.isChecked(),
            'day_max': self.preload_day_spinBox.value(),
            'week_max': self.preload_week_spinBox.value(),
            'month_max': self.preload_month_spinBox.value(),
            'quarter_max': self.preload_quarter_spinBox.value(),
            'halfyear_max': self.preload_halfyear_spinBox.value(),
            'year_max': self.preload_year_spinBox.value(),
            'min_max': self.preload_min1_spinBox.value(),
            'min5_max': self.preload_min5_spinBox.value(),
            'min15_max': self.preload_min15_spinBox.value(),
            'min30_max': self.preload_min30_spinBox.value(),
            'min60_max': self.preload_min60_spinBox.value(),
            'hour2_max': self.preload_hour2_spinBox.value(),
        }
        return import_config

    def initThreads(self):
        self.escape_time_thread = None
        self.hdf5_import_thread = None
        self.mysql_import_thread = None
        self.collect_spot_thread = None
        self.sched_import_thread = None

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
        # self.finance_progressBar.setValue(0)
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
                "耗时：{:>.2f} 秒 （{:>.2f}分钟） {}".format(self.escape_time, self.escape_time / 60, datetime.datetime.now())
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
                if can_upgrade():
                    self.import_detail_textEdit.append("========================================================")
                    self.import_detail_textEdit.append(
                        "Hikyuu 新版本 ({}) 已发布，建议更新".format(get_last_version()))
                    self.import_detail_textEdit.append("更新命令: pip instal hikyuu --upgrade")
                    self.import_detail_textEdit.append("========================================================")
                self.import_running = False

            elif msg_task_name == 'IMPORT_KDATA':
                ktype, progress = msg[2:4]
                if ktype != 'FINISHED':
                    self.hdf5_import_progress_bar[ktype].setValue(progress)
                else:
                    self.import_detail_textEdit.append('导入 {} {} 记录数：{}'.format(msg[3], msg[4], msg[5]))

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
                elif msg[2] == '导入通达信财务信息完毕!':
                    self.import_detail_textEdit.append('导入通达信财务记录数：{}'.format(msg[3]))

            elif msg_task_name == 'IMPORT_FINANCE':
                # self.finance_progressBar.setValue(msg[2])
                self.logger.info(f"财务数据下载: {msg[2]}%")

            elif msg_task_name == 'IMPORT_BLOCKINFO':
                if msg[2] != 'FINISHED':
                    self.import_detail_textEdit.append(msg[2])

            elif msg_task_name == 'IMPORT_ZH_BOND10':
                if msg[2] != 'FINISHED':
                    self.import_detail_textEdit.append(msg[2])

    @pyqtSlot()
    def on_start_import_pushButton_clicked(self):
        try:
            self.saveConfig()
        except Exception as e:
            QMessageBox.about(self, "保存配置信息失败", str(e))
            return

        config = self.getCurrentConfig()
        if config.getboolean('hdf5', 'enable') \
                and (not os.path.lexists(config['hdf5']['dir']) or not os.path.isdir(config['hdf5']['dir'])):
            QMessageBox.about(self, "错误", '指定的目标数据存放目录不存在！')
            return

        if config.getboolean('tdx', 'enable') \
            and (not os.path.lexists(config['tdx']['dir'])
                 or not os.path.isdir(config['tdx']['dir'])):
            QMessageBox.about(self, "错误", "请确认通达信安装目录是否正确！")
            return

        self.import_running = True
        self.start_import_pushButton.setEnabled(False)
        self.reset_progress_bar()

        self.import_status_label.setText("正在启动任务....")
        QApplication.processEvents()

        if self.tdx_radioButton.isChecked():
            self.hdf5_import_thread = UseTdxImportToH5Thread(self, config)
        else:
            self.hdf5_import_thread = UsePytdxImportToH5Thread(self, config)

        self.hdf5_import_thread.message.connect(self.on_message_from_thread)
        self.hdf5_import_thread.start()

        self.escape_time = 0.0
        self.escape_time_thread = EscapetimeThread()
        self.escape_time_thread.message.connect(self.on_message_from_thread)
        self.escape_time_thread.start()

    @pyqtSlot()
    def on_sched_import_pushButton_clicked(self):
        self.sched_import_pushButton.setEnabled(False)
        if self._is_sched_import_running:
            self._is_sched_import_running = False
            self.sched_import_pushButton.setText("启动定时导入")
            self.sched_import_thread.terminate()
            self.sched_import_thread.wait()
            self.logger.info("已停止定时导入")
            self.sched_import_thread = None
            self.sched_import_pushButton.setEnabled(True)
            self.start_import_pushButton.setEnabled(True)
            return

        try:
            self.saveConfig()
        except Exception as e:
            QMessageBox.about(self, "保存配置信息错误", str(e))
            return

        self.start_import_pushButton.setEnabled(False)
        self._is_sched_import_running = True
        self.sched_import_thread = SchedImportThread(self.getCurrentConfig())
        self.sched_import_thread.message.connect(self.on_start_import_pushButton_clicked)
        self.sched_import_thread.start()
        self.sched_import_pushButton.setText("停止定时导入")
        self.sched_import_pushButton.setEnabled(True)

    @pyqtSlot()
    def on_collect_start_pushButton_clicked(self):
        if self._is_collect_running:
            if self.collect_spot_thread is not None and self.collect_spot_thread.isRunning():
                release_nng_senders()
                self.collect_spot_thread.terminate()
                self.collect_spot_thread.wait()
                self.collect_spot_thread = None
            self._is_collect_running = False
            self.logger.info("停止采集")
            self.collect_status_label.setText("已停止")
            self.collect_start_pushButton.setText("启动采集")
            QMessageBox.about(self, '', '已停止')
        else:
            if self.collect_spot_thread is None or self.collect_spot_thread.isFinished():
                self.collect_spot_thread = CollectSpotThread(
                    self.getCurrentConfig(),
                    self.getHikyuuConfigFileName(),
                )
                self.collect_spot_thread.start()
            self._is_collect_running = True
            self.collect_status_label.setText("运行中")
            self.collect_start_pushButton.setText("停止采集")
            QMessageBox.about(self, '', '已启动，请在控制台日志查看是否正常运行')


class_logger(MyMainWindow)


def start():
    import requests
    import urllib
    logging.getLogger("requests").setLevel(logging.WARNING)
    logging.getLogger("urllib3").setLevel(logging.WARNING)
    app = QApplication(sys.argv)
    use_dark_style = False  # 使用暗黑主题
    if use_dark_style:
        import qdarkstyle
        app.setStyleSheet(qdarkstyle.load_stylesheet(qt_api='pyqt5'))
    myWin = MyMainWindow(capture_output=True, use_dark_style=use_dark_style)
    myWin.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    import requests
    import urllib
    logging.getLogger("requests").setLevel(logging.WARNING)
    logging.getLogger("urllib3").setLevel(logging.WARNING)
    logging.getLogger("pytdx").setLevel(logging.WARNING)

    # 自适应分辨率，防止字体显示不全
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_UseHighDpiPixmaps)
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling)

    app = QApplication(sys.argv)
    f = QFont('SimSun')
    f.setPixelSize(12)
    app.setFont(f)

    use_dark_style = False  # 使用暗黑主题
    if use_dark_style:
        import qdarkstyle
        app.setStyleSheet(qdarkstyle.load_stylesheet(qt_api='pyqt5'))
    if (len(sys.argv) > 1 and sys.argv[1] == '0'):
        FORMAT = '%(asctime)-15s [%(levelname)s]: %(message)s [%(name)s::%(funcName)s]'
        logging.basicConfig(format=FORMAT, level=logging.INFO, handlers=[
            logging.StreamHandler(),
        ])
        myWin = MyMainWindow(capture_output=False, use_dark_style=use_dark_style)
    else:
        myWin = MyMainWindow(capture_output=True, use_dark_style=use_dark_style)

    myWin.show()
    sys.exit(app.exec())
