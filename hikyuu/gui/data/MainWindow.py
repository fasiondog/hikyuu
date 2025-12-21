# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'MainWindow.ui'
##
## Created by: Qt User Interface Compiler version 6.10.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QCheckBox, QComboBox, QDateEdit,
    QDateTimeEdit, QFrame, QGridLayout, QGroupBox,
    QHBoxLayout, QLabel, QLayout, QLineEdit,
    QMainWindow, QProgressBar, QPushButton, QRadioButton,
    QSizePolicy, QSpacerItem, QSpinBox, QTabWidget,
    QTextBrowser, QTextEdit, QTimeEdit, QVBoxLayout,
    QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(1139, 713)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.horizontalLayout_12 = QHBoxLayout(self.centralwidget)
        self.horizontalLayout_12.setObjectName(u"horizontalLayout_12")
        self.tabWidget = QTabWidget(self.centralwidget)
        self.tabWidget.setObjectName(u"tabWidget")
        self.tabWidget.setEnabled(True)
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.tabWidget.sizePolicy().hasHeightForWidth())
        self.tabWidget.setSizePolicy(sizePolicy)
        self.tab_4 = QWidget()
        self.tab_4.setObjectName(u"tab_4")
        self.verticalLayout_13 = QVBoxLayout(self.tab_4)
        self.verticalLayout_13.setObjectName(u"verticalLayout_13")
        self.groupBox_2 = QGroupBox(self.tab_4)
        self.groupBox_2.setObjectName(u"groupBox_2")
        sizePolicy1 = QSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.groupBox_2.sizePolicy().hasHeightForWidth())
        self.groupBox_2.setSizePolicy(sizePolicy1)
        self.verticalLayout_12 = QVBoxLayout(self.groupBox_2)
        self.verticalLayout_12.setObjectName(u"verticalLayout_12")
        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalLayout.setContentsMargins(10, 10, -1, -1)
        self.pytdx_radioButton = QRadioButton(self.groupBox_2)
        self.pytdx_radioButton.setObjectName(u"pytdx_radioButton")
        self.pytdx_radioButton.setEnabled(True)

        self.horizontalLayout.addWidget(self.pytdx_radioButton)

        self.label_16 = QLabel(self.groupBox_2)
        self.label_16.setObjectName(u"label_16")
        self.label_16.setAlignment(Qt.AlignRight|Qt.AlignTrailing|Qt.AlignVCenter)

        self.horizontalLayout.addWidget(self.label_16)

        self.use_tdx_number_spinBox = QSpinBox(self.groupBox_2)
        self.use_tdx_number_spinBox.setObjectName(u"use_tdx_number_spinBox")
        self.use_tdx_number_spinBox.setMinimum(1)
        self.use_tdx_number_spinBox.setMaximum(20)
        self.use_tdx_number_spinBox.setValue(10)

        self.horizontalLayout.addWidget(self.use_tdx_number_spinBox)

        self.label_17 = QLabel(self.groupBox_2)
        self.label_17.setObjectName(u"label_17")

        self.horizontalLayout.addWidget(self.label_17)

        self.horizontalSpacer_13 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout.addItem(self.horizontalSpacer_13)


        self.verticalLayout_12.addLayout(self.horizontalLayout)

        self.horizontalLayout_5 = QHBoxLayout()
        self.horizontalLayout_5.setObjectName(u"horizontalLayout_5")
        self.horizontalLayout_5.setContentsMargins(10, 0, -1, -1)
        self.tdx_radioButton = QRadioButton(self.groupBox_2)
        self.tdx_radioButton.setObjectName(u"tdx_radioButton")

        self.horizontalLayout_5.addWidget(self.tdx_radioButton)


        self.verticalLayout_12.addLayout(self.horizontalLayout_5)

        self.gridLayout_2 = QGridLayout()
        self.gridLayout_2.setObjectName(u"gridLayout_2")
        self.gridLayout_2.setContentsMargins(10, -1, -1, 10)
        self.select_tdx_dir_pushButton = QPushButton(self.groupBox_2)
        self.select_tdx_dir_pushButton.setObjectName(u"select_tdx_dir_pushButton")
        sizePolicy1.setHeightForWidth(self.select_tdx_dir_pushButton.sizePolicy().hasHeightForWidth())
        self.select_tdx_dir_pushButton.setSizePolicy(sizePolicy1)

        self.gridLayout_2.addWidget(self.select_tdx_dir_pushButton, 0, 3, 1, 1)

        self.label_2 = QLabel(self.groupBox_2)
        self.label_2.setObjectName(u"label_2")

        self.gridLayout_2.addWidget(self.label_2, 0, 0, 1, 1)

        self.tdx_dir_lineEdit = QLineEdit(self.groupBox_2)
        self.tdx_dir_lineEdit.setObjectName(u"tdx_dir_lineEdit")
        sizePolicy2 = QSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        sizePolicy2.setHorizontalStretch(1)
        sizePolicy2.setVerticalStretch(0)
        sizePolicy2.setHeightForWidth(self.tdx_dir_lineEdit.sizePolicy().hasHeightForWidth())
        self.tdx_dir_lineEdit.setSizePolicy(sizePolicy2)

        self.gridLayout_2.addWidget(self.tdx_dir_lineEdit, 0, 1, 1, 2)


        self.verticalLayout_12.addLayout(self.gridLayout_2)

        self.horizontalLayout_24 = QHBoxLayout()
        self.horizontalLayout_24.setObjectName(u"horizontalLayout_24")
        self.horizontalLayout_24.setContentsMargins(10, 0, -1, -1)
        self.qmt_radioButton = QRadioButton(self.groupBox_2)
        self.qmt_radioButton.setObjectName(u"qmt_radioButton")

        self.horizontalLayout_24.addWidget(self.qmt_radioButton)


        self.verticalLayout_12.addLayout(self.horizontalLayout_24)


        self.verticalLayout_13.addWidget(self.groupBox_2)

        self.groupBox_7 = QGroupBox(self.tab_4)
        self.groupBox_7.setObjectName(u"groupBox_7")
        self.verticalLayout_11 = QVBoxLayout(self.groupBox_7)
        self.verticalLayout_11.setObjectName(u"verticalLayout_11")
        self.horizontalLayout_4 = QHBoxLayout()
        self.horizontalLayout_4.setObjectName(u"horizontalLayout_4")
        self.horizontalLayout_4.setContentsMargins(10, 10, -1, -1)
        self.import_weight_checkBox = QCheckBox(self.groupBox_7)
        self.import_weight_checkBox.setObjectName(u"import_weight_checkBox")

        self.horizontalLayout_4.addWidget(self.import_weight_checkBox)

        self.import_history_finance_checkBox = QCheckBox(self.groupBox_7)
        self.import_history_finance_checkBox.setObjectName(u"import_history_finance_checkBox")

        self.horizontalLayout_4.addWidget(self.import_history_finance_checkBox)

        self.import_block_checkBox = QCheckBox(self.groupBox_7)
        self.import_block_checkBox.setObjectName(u"import_block_checkBox")

        self.horizontalLayout_4.addWidget(self.import_block_checkBox)


        self.verticalLayout_11.addLayout(self.horizontalLayout_4)

        self.horizontalLayout_3 = QHBoxLayout()
        self.horizontalLayout_3.setObjectName(u"horizontalLayout_3")
        self.horizontalLayout_3.setContentsMargins(10, -1, -1, -1)
        self.import_stock_checkBox = QCheckBox(self.groupBox_7)
        self.import_stock_checkBox.setObjectName(u"import_stock_checkBox")

        self.horizontalLayout_3.addWidget(self.import_stock_checkBox)

        self.import_fund_checkBox = QCheckBox(self.groupBox_7)
        self.import_fund_checkBox.setObjectName(u"import_fund_checkBox")

        self.horizontalLayout_3.addWidget(self.import_fund_checkBox)

        self.import_future_checkBox = QCheckBox(self.groupBox_7)
        self.import_future_checkBox.setObjectName(u"import_future_checkBox")
        self.import_future_checkBox.setEnabled(False)

        self.horizontalLayout_3.addWidget(self.import_future_checkBox)

        self.horizontalSpacer_11 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_3.addItem(self.horizontalSpacer_11)


        self.verticalLayout_11.addLayout(self.horizontalLayout_3)

        self.horizontalLayout_2 = QHBoxLayout()
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.horizontalLayout_2.setContentsMargins(10, -1, -1, -1)
        self.import_day_checkBox = QCheckBox(self.groupBox_7)
        self.import_day_checkBox.setObjectName(u"import_day_checkBox")

        self.horizontalLayout_2.addWidget(self.import_day_checkBox)

        self.import_min5_checkBox = QCheckBox(self.groupBox_7)
        self.import_min5_checkBox.setObjectName(u"import_min5_checkBox")

        self.horizontalLayout_2.addWidget(self.import_min5_checkBox)

        self.import_min_checkBox = QCheckBox(self.groupBox_7)
        self.import_min_checkBox.setObjectName(u"import_min_checkBox")

        self.horizontalLayout_2.addWidget(self.import_min_checkBox)

        self.import_trans_checkBox = QCheckBox(self.groupBox_7)
        self.import_trans_checkBox.setObjectName(u"import_trans_checkBox")
        self.import_trans_checkBox.setEnabled(True)

        self.horizontalLayout_2.addWidget(self.import_trans_checkBox)

        self.import_time_checkBox = QCheckBox(self.groupBox_7)
        self.import_time_checkBox.setObjectName(u"import_time_checkBox")
        self.import_time_checkBox.setEnabled(True)

        self.horizontalLayout_2.addWidget(self.import_time_checkBox)

        self.horizontalSpacer_12 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_2.addItem(self.horizontalSpacer_12)


        self.verticalLayout_11.addLayout(self.horizontalLayout_2)

        self.line = QFrame(self.groupBox_7)
        self.line.setObjectName(u"line")
        self.line.setFrameShape(QFrame.Shape.HLine)
        self.line.setFrameShadow(QFrame.Shadow.Sunken)

        self.verticalLayout_11.addWidget(self.line)

        self.gridLayout_4 = QGridLayout()
        self.gridLayout_4.setObjectName(u"gridLayout_4")
        self.gridLayout_4.setContentsMargins(10, 10, -1, -1)
        self.label_7 = QLabel(self.groupBox_7)
        self.label_7.setObjectName(u"label_7")

        self.gridLayout_4.addWidget(self.label_7, 4, 0, 1, 1)

        self.label_6 = QLabel(self.groupBox_7)
        self.label_6.setObjectName(u"label_6")

        self.gridLayout_4.addWidget(self.label_6, 3, 0, 1, 1)

        self.min5_start_dateEdit = QDateEdit(self.groupBox_7)
        self.min5_start_dateEdit.setObjectName(u"min5_start_dateEdit")
        self.min5_start_dateEdit.setMinimumDateTime(QDateTime(QDate(1989, 12, 21), QTime(0, 0, 0)))
        self.min5_start_dateEdit.setCalendarPopup(True)

        self.gridLayout_4.addWidget(self.min5_start_dateEdit, 1, 1, 1, 1)

        self.min_start_dateEdit = QDateEdit(self.groupBox_7)
        self.min_start_dateEdit.setObjectName(u"min_start_dateEdit")
        self.min_start_dateEdit.setCalendarPopup(True)

        self.gridLayout_4.addWidget(self.min_start_dateEdit, 2, 1, 1, 1)

        self.label_3 = QLabel(self.groupBox_7)
        self.label_3.setObjectName(u"label_3")

        self.gridLayout_4.addWidget(self.label_3, 0, 0, 1, 1)

        self.label_10 = QLabel(self.groupBox_7)
        self.label_10.setObjectName(u"label_10")

        self.gridLayout_4.addWidget(self.label_10, 1, 0, 1, 1)

        self.label_15 = QLabel(self.groupBox_7)
        self.label_15.setObjectName(u"label_15")

        self.gridLayout_4.addWidget(self.label_15, 2, 0, 1, 1)

        self.trans_start_dateEdit = QDateEdit(self.groupBox_7)
        self.trans_start_dateEdit.setObjectName(u"trans_start_dateEdit")
        self.trans_start_dateEdit.setCalendarPopup(True)

        self.gridLayout_4.addWidget(self.trans_start_dateEdit, 3, 1, 1, 1)

        self.day_start_dateEdit = QDateEdit(self.groupBox_7)
        self.day_start_dateEdit.setObjectName(u"day_start_dateEdit")
        self.day_start_dateEdit.setMinimumDateTime(QDateTime(QDate(1989, 12, 21), QTime(0, 0, 0)))
        self.day_start_dateEdit.setCalendarPopup(True)

        self.gridLayout_4.addWidget(self.day_start_dateEdit, 0, 1, 1, 1)

        self.time_start_dateEdit = QDateEdit(self.groupBox_7)
        self.time_start_dateEdit.setObjectName(u"time_start_dateEdit")
        self.time_start_dateEdit.setCalendarPopup(True)

        self.gridLayout_4.addWidget(self.time_start_dateEdit, 4, 1, 1, 1)

        self.horizontalSpacer_9 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.gridLayout_4.addItem(self.horizontalSpacer_9, 1, 2, 1, 1)


        self.verticalLayout_11.addLayout(self.gridLayout_4)

        self.verticalSpacer_16 = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.verticalLayout_11.addItem(self.verticalSpacer_16)


        self.verticalLayout_13.addWidget(self.groupBox_7)

        self.tabWidget.addTab(self.tab_4, "")
        self.tab_3 = QWidget()
        self.tab_3.setObjectName(u"tab_3")
        self.verticalLayout_8 = QVBoxLayout(self.tab_3)
        self.verticalLayout_8.setObjectName(u"verticalLayout_8")
        self.groupBox_4 = QGroupBox(self.tab_3)
        self.groupBox_4.setObjectName(u"groupBox_4")
        sizePolicy1.setHeightForWidth(self.groupBox_4.sizePolicy().hasHeightForWidth())
        self.groupBox_4.setSizePolicy(sizePolicy1)
        self.groupBox_4.setMinimumSize(QSize(0, 80))
        self.verticalLayout_14 = QVBoxLayout(self.groupBox_4)
        self.verticalLayout_14.setObjectName(u"verticalLayout_14")
        self.horizontalLayout_19 = QHBoxLayout()
        self.horizontalLayout_19.setObjectName(u"horizontalLayout_19")
        self.horizontalLayout_19.setContentsMargins(10, -1, -1, -1)
        self.enable_hdf55_radioButton = QRadioButton(self.groupBox_4)
        self.enable_hdf55_radioButton.setObjectName(u"enable_hdf55_radioButton")

        self.horizontalLayout_19.addWidget(self.enable_hdf55_radioButton)

        self.enable_mysql_radioButton = QRadioButton(self.groupBox_4)
        self.enable_mysql_radioButton.setObjectName(u"enable_mysql_radioButton")

        self.horizontalLayout_19.addWidget(self.enable_mysql_radioButton)

        self.enable_clickhouse_radioButton = QRadioButton(self.groupBox_4)
        self.enable_clickhouse_radioButton.setObjectName(u"enable_clickhouse_radioButton")

        self.horizontalLayout_19.addWidget(self.enable_clickhouse_radioButton)

        self.horizontalSpacer_10 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_19.addItem(self.horizontalSpacer_10)


        self.verticalLayout_14.addLayout(self.horizontalLayout_19)


        self.verticalLayout_8.addWidget(self.groupBox_4)

        self.groupBox_3 = QGroupBox(self.tab_3)
        self.groupBox_3.setObjectName(u"groupBox_3")
        sizePolicy1.setHeightForWidth(self.groupBox_3.sizePolicy().hasHeightForWidth())
        self.groupBox_3.setSizePolicy(sizePolicy1)
        self.groupBox_3.setMinimumSize(QSize(0, 80))
        self.verticalLayout_10 = QVBoxLayout(self.groupBox_3)
        self.verticalLayout_10.setObjectName(u"verticalLayout_10")
        self.gridLayout_3 = QGridLayout()
        self.gridLayout_3.setObjectName(u"gridLayout_3")
        self.gridLayout_3.setContentsMargins(10, -1, -1, -1)
        self.hdf5_dir_lineEdit = QLineEdit(self.groupBox_3)
        self.hdf5_dir_lineEdit.setObjectName(u"hdf5_dir_lineEdit")

        self.gridLayout_3.addWidget(self.hdf5_dir_lineEdit, 0, 1, 1, 1)

        self.hdf5_dir_pushButton = QPushButton(self.groupBox_3)
        self.hdf5_dir_pushButton.setObjectName(u"hdf5_dir_pushButton")

        self.gridLayout_3.addWidget(self.hdf5_dir_pushButton, 0, 2, 1, 1)

        self.label = QLabel(self.groupBox_3)
        self.label.setObjectName(u"label")

        self.gridLayout_3.addWidget(self.label, 0, 0, 1, 1)


        self.verticalLayout_10.addLayout(self.gridLayout_3)


        self.verticalLayout_8.addWidget(self.groupBox_3)

        self.groupBox = QGroupBox(self.tab_3)
        self.groupBox.setObjectName(u"groupBox")
        self.groupBox.setFlat(False)
        self.groupBox.setCheckable(False)
        self.verticalLayout_9 = QVBoxLayout(self.groupBox)
        self.verticalLayout_9.setObjectName(u"verticalLayout_9")
        self.gridLayout_5 = QGridLayout()
        self.gridLayout_5.setObjectName(u"gridLayout_5")
        self.gridLayout_5.setContentsMargins(10, 10, -1, 10)
        self.mysql_usr_lineEdit = QLineEdit(self.groupBox)
        self.mysql_usr_lineEdit.setObjectName(u"mysql_usr_lineEdit")

        self.gridLayout_5.addWidget(self.mysql_usr_lineEdit, 3, 1, 1, 1)

        self.label_11 = QLabel(self.groupBox)
        self.label_11.setObjectName(u"label_11")

        self.gridLayout_5.addWidget(self.label_11, 3, 0, 1, 1)

        self.mysql_port_lineEdit = QLineEdit(self.groupBox)
        self.mysql_port_lineEdit.setObjectName(u"mysql_port_lineEdit")

        self.gridLayout_5.addWidget(self.mysql_port_lineEdit, 2, 1, 1, 1)

        self.label_19 = QLabel(self.groupBox)
        self.label_19.setObjectName(u"label_19")

        self.gridLayout_5.addWidget(self.label_19, 2, 0, 1, 1)

        self.mysql_ip_lineEdit = QLineEdit(self.groupBox)
        self.mysql_ip_lineEdit.setObjectName(u"mysql_ip_lineEdit")

        self.gridLayout_5.addWidget(self.mysql_ip_lineEdit, 1, 1, 1, 1)

        self.label_13 = QLabel(self.groupBox)
        self.label_13.setObjectName(u"label_13")

        self.gridLayout_5.addWidget(self.label_13, 4, 0, 1, 1)

        self.mysql_tmpdir_lineEdit = QLineEdit(self.groupBox)
        self.mysql_tmpdir_lineEdit.setObjectName(u"mysql_tmpdir_lineEdit")

        self.gridLayout_5.addWidget(self.mysql_tmpdir_lineEdit, 0, 1, 1, 1)

        self.mysql_tmpdir_pushButton = QPushButton(self.groupBox)
        self.mysql_tmpdir_pushButton.setObjectName(u"mysql_tmpdir_pushButton")

        self.gridLayout_5.addWidget(self.mysql_tmpdir_pushButton, 0, 2, 1, 1)

        self.mysql_test_pushButton = QPushButton(self.groupBox)
        self.mysql_test_pushButton.setObjectName(u"mysql_test_pushButton")

        self.gridLayout_5.addWidget(self.mysql_test_pushButton, 2, 2, 1, 1)

        self.label_20 = QLabel(self.groupBox)
        self.label_20.setObjectName(u"label_20")

        self.gridLayout_5.addWidget(self.label_20, 0, 0, 1, 1)

        self.label_18 = QLabel(self.groupBox)
        self.label_18.setObjectName(u"label_18")

        self.gridLayout_5.addWidget(self.label_18, 1, 0, 1, 1)

        self.mysql_pwd_lineEdit = QLineEdit(self.groupBox)
        self.mysql_pwd_lineEdit.setObjectName(u"mysql_pwd_lineEdit")

        self.gridLayout_5.addWidget(self.mysql_pwd_lineEdit, 4, 1, 1, 1)


        self.verticalLayout_9.addLayout(self.gridLayout_5)


        self.verticalLayout_8.addWidget(self.groupBox)

        self.groupBox_8 = QGroupBox(self.tab_3)
        self.groupBox_8.setObjectName(u"groupBox_8")
        self.verticalLayout_19 = QVBoxLayout(self.groupBox_8)
        self.verticalLayout_19.setObjectName(u"verticalLayout_19")
        self.gridLayout_7 = QGridLayout()
        self.gridLayout_7.setObjectName(u"gridLayout_7")
        self.gridLayout_7.setContentsMargins(10, 10, -1, 10)
        self.clickhouse_pwd_lineEdit = QLineEdit(self.groupBox_8)
        self.clickhouse_pwd_lineEdit.setObjectName(u"clickhouse_pwd_lineEdit")

        self.gridLayout_7.addWidget(self.clickhouse_pwd_lineEdit, 5, 1, 1, 1)

        self.label_53 = QLabel(self.groupBox_8)
        self.label_53.setObjectName(u"label_53")

        self.gridLayout_7.addWidget(self.label_53, 0, 0, 1, 1)

        self.label_54 = QLabel(self.groupBox_8)
        self.label_54.setObjectName(u"label_54")

        self.gridLayout_7.addWidget(self.label_54, 1, 0, 1, 1)

        self.clickhouse_ip_lineEdit = QLineEdit(self.groupBox_8)
        self.clickhouse_ip_lineEdit.setObjectName(u"clickhouse_ip_lineEdit")

        self.gridLayout_7.addWidget(self.clickhouse_ip_lineEdit, 1, 1, 1, 1)

        self.clickhouse_tmpdir_lineEdit = QLineEdit(self.groupBox_8)
        self.clickhouse_tmpdir_lineEdit.setObjectName(u"clickhouse_tmpdir_lineEdit")

        self.gridLayout_7.addWidget(self.clickhouse_tmpdir_lineEdit, 0, 1, 1, 1)

        self.label_51 = QLabel(self.groupBox_8)
        self.label_51.setObjectName(u"label_51")

        self.gridLayout_7.addWidget(self.label_51, 2, 0, 1, 1)

        self.label_50 = QLabel(self.groupBox_8)
        self.label_50.setObjectName(u"label_50")

        self.gridLayout_7.addWidget(self.label_50, 4, 0, 1, 1)

        self.clickhouse_usr_lineEdit = QLineEdit(self.groupBox_8)
        self.clickhouse_usr_lineEdit.setObjectName(u"clickhouse_usr_lineEdit")

        self.gridLayout_7.addWidget(self.clickhouse_usr_lineEdit, 4, 1, 1, 1)

        self.label_55 = QLabel(self.groupBox_8)
        self.label_55.setObjectName(u"label_55")

        self.gridLayout_7.addWidget(self.label_55, 3, 0, 1, 1)

        self.clickhouse_port_lineEdit = QLineEdit(self.groupBox_8)
        self.clickhouse_port_lineEdit.setObjectName(u"clickhouse_port_lineEdit")

        self.gridLayout_7.addWidget(self.clickhouse_port_lineEdit, 3, 1, 1, 1)

        self.label_52 = QLabel(self.groupBox_8)
        self.label_52.setObjectName(u"label_52")

        self.gridLayout_7.addWidget(self.label_52, 5, 0, 1, 1)

        self.clickhouse_test_pushButton = QPushButton(self.groupBox_8)
        self.clickhouse_test_pushButton.setObjectName(u"clickhouse_test_pushButton")

        self.gridLayout_7.addWidget(self.clickhouse_test_pushButton, 2, 2, 1, 1)

        self.clickhouse_tmpdir_pushButton = QPushButton(self.groupBox_8)
        self.clickhouse_tmpdir_pushButton.setObjectName(u"clickhouse_tmpdir_pushButton")

        self.gridLayout_7.addWidget(self.clickhouse_tmpdir_pushButton, 0, 2, 1, 1)

        self.clickhouse_http_port_lineEdit = QLineEdit(self.groupBox_8)
        self.clickhouse_http_port_lineEdit.setObjectName(u"clickhouse_http_port_lineEdit")

        self.gridLayout_7.addWidget(self.clickhouse_http_port_lineEdit, 2, 1, 1, 1)


        self.verticalLayout_19.addLayout(self.gridLayout_7)


        self.verticalLayout_8.addWidget(self.groupBox_8)

        self.verticalSpacer_12 = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.verticalLayout_8.addItem(self.verticalSpacer_12)

        self.tabWidget.addTab(self.tab_3, "")
        self.tab_2 = QWidget()
        self.tab_2.setObjectName(u"tab_2")
        self.verticalLayout_7 = QVBoxLayout(self.tab_2)
        self.verticalLayout_7.setObjectName(u"verticalLayout_7")
        self.verticalSpacer_20 = QSpacerItem(20, 9, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)

        self.verticalLayout_7.addItem(self.verticalSpacer_20)

        self.label_43 = QLabel(self.tab_2)
        self.label_43.setObjectName(u"label_43")

        self.verticalLayout_7.addWidget(self.label_43)

        self.horizontalLayout_11 = QHBoxLayout()
        self.horizontalLayout_11.setObjectName(u"horizontalLayout_11")
        self.horizontalLayout_11.setContentsMargins(10, 20, 10, -1)
        self.sched_import_pushButton = QPushButton(self.tab_2)
        self.sched_import_pushButton.setObjectName(u"sched_import_pushButton")

        self.horizontalLayout_11.addWidget(self.sched_import_pushButton)

        self.horizontalLayout_14 = QHBoxLayout()
        self.horizontalLayout_14.setObjectName(u"horizontalLayout_14")
        self.label_40 = QLabel(self.tab_2)
        self.label_40.setObjectName(u"label_40")

        self.horizontalLayout_14.addWidget(self.label_40)

        self.sched_import_timeEdit = QTimeEdit(self.tab_2)
        self.sched_import_timeEdit.setObjectName(u"sched_import_timeEdit")

        self.horizontalLayout_14.addWidget(self.sched_import_timeEdit)

        self.horizontalSpacer_7 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_14.addItem(self.horizontalSpacer_7)


        self.horizontalLayout_11.addLayout(self.horizontalLayout_14)


        self.verticalLayout_7.addLayout(self.horizontalLayout_11)

        self.verticalSpacer_11 = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)

        self.verticalLayout_7.addItem(self.verticalSpacer_11)

        self.horizontalLayout_18 = QHBoxLayout()
        self.horizontalLayout_18.setObjectName(u"horizontalLayout_18")
        self.horizontalLayout_18.setContentsMargins(10, -1, 10, -1)
        self.start_import_pushButton = QPushButton(self.tab_2)
        self.start_import_pushButton.setObjectName(u"start_import_pushButton")

        self.horizontalLayout_18.addWidget(self.start_import_pushButton)

        self.import_status_label = QLabel(self.tab_2)
        self.import_status_label.setObjectName(u"import_status_label")

        self.horizontalLayout_18.addWidget(self.import_status_label)

        self.horizontalSpacer_8 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_18.addItem(self.horizontalSpacer_8)


        self.verticalLayout_7.addLayout(self.horizontalLayout_18)

        self.verticalSpacer_10 = QSpacerItem(20, 20, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.verticalLayout_7.addItem(self.verticalSpacer_10)

        self.groupBox_5 = QGroupBox(self.tab_2)
        self.groupBox_5.setObjectName(u"groupBox_5")
        self.verticalLayout_5 = QVBoxLayout(self.groupBox_5)
        self.verticalLayout_5.setObjectName(u"verticalLayout_5")
        self.gridLayout = QGridLayout()
        self.gridLayout.setObjectName(u"gridLayout")
        self.hdf5_time_progressBar = QProgressBar(self.groupBox_5)
        self.hdf5_time_progressBar.setObjectName(u"hdf5_time_progressBar")
        self.hdf5_time_progressBar.setValue(0)

        self.gridLayout.addWidget(self.hdf5_time_progressBar, 4, 1, 1, 1)

        self.hdf5_weight_label = QLabel(self.groupBox_5)
        self.hdf5_weight_label.setObjectName(u"hdf5_weight_label")

        self.gridLayout.addWidget(self.hdf5_weight_label, 5, 1, 1, 1)

        self.label_14 = QLabel(self.groupBox_5)
        self.label_14.setObjectName(u"label_14")

        self.gridLayout.addWidget(self.label_14, 2, 0, 1, 1)

        self.hdf5_trans_progressBar = QProgressBar(self.groupBox_5)
        self.hdf5_trans_progressBar.setObjectName(u"hdf5_trans_progressBar")
        self.hdf5_trans_progressBar.setValue(0)

        self.gridLayout.addWidget(self.hdf5_trans_progressBar, 3, 1, 1, 1)

        self.label_4 = QLabel(self.groupBox_5)
        self.label_4.setObjectName(u"label_4")

        self.gridLayout.addWidget(self.label_4, 3, 0, 1, 1)

        self.label_8 = QLabel(self.groupBox_5)
        self.label_8.setObjectName(u"label_8")

        self.gridLayout.addWidget(self.label_8, 0, 0, 1, 1)

        self.label_5 = QLabel(self.groupBox_5)
        self.label_5.setObjectName(u"label_5")

        self.gridLayout.addWidget(self.label_5, 4, 0, 1, 1)

        self.hdf5_5min_progressBar = QProgressBar(self.groupBox_5)
        self.hdf5_5min_progressBar.setObjectName(u"hdf5_5min_progressBar")
        self.hdf5_5min_progressBar.setValue(0)

        self.gridLayout.addWidget(self.hdf5_5min_progressBar, 1, 1, 1, 1)

        self.hdf5_day_progressBar = QProgressBar(self.groupBox_5)
        self.hdf5_day_progressBar.setObjectName(u"hdf5_day_progressBar")
        self.hdf5_day_progressBar.setValue(0)

        self.gridLayout.addWidget(self.hdf5_day_progressBar, 0, 1, 1, 1)

        self.label_12 = QLabel(self.groupBox_5)
        self.label_12.setObjectName(u"label_12")

        self.gridLayout.addWidget(self.label_12, 1, 0, 1, 1)

        self.hdf5_min_progressBar = QProgressBar(self.groupBox_5)
        self.hdf5_min_progressBar.setObjectName(u"hdf5_min_progressBar")
        self.hdf5_min_progressBar.setValue(0)

        self.gridLayout.addWidget(self.hdf5_min_progressBar, 2, 1, 1, 1)

        self.label_9 = QLabel(self.groupBox_5)
        self.label_9.setObjectName(u"label_9")

        self.gridLayout.addWidget(self.label_9, 5, 0, 1, 1)

        self.horizontalSpacer_14 = QSpacerItem(5, 20, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.gridLayout.addItem(self.horizontalSpacer_14, 0, 2, 1, 1)


        self.verticalLayout_5.addLayout(self.gridLayout)

        self.verticalSpacer_9 = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.verticalLayout_5.addItem(self.verticalSpacer_9)

        self.import_detail_textEdit = QTextEdit(self.groupBox_5)
        self.import_detail_textEdit.setObjectName(u"import_detail_textEdit")
        self.import_detail_textEdit.setFrameShape(QFrame.NoFrame)
        self.import_detail_textEdit.setReadOnly(True)

        self.verticalLayout_5.addWidget(self.import_detail_textEdit)


        self.verticalLayout_7.addWidget(self.groupBox_5)

        self.tabWidget.addTab(self.tab_2, "")
        self.tab_6 = QWidget()
        self.tab_6.setObjectName(u"tab_6")
        self.verticalLayout_3 = QVBoxLayout(self.tab_6)
        self.verticalLayout_3.setObjectName(u"verticalLayout_3")
        self.groupBox_6 = QGroupBox(self.tab_6)
        self.groupBox_6.setObjectName(u"groupBox_6")
        self.verticalLayout_4 = QVBoxLayout(self.groupBox_6)
        self.verticalLayout_4.setObjectName(u"verticalLayout_4")
        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.verticalLayout.setContentsMargins(20, 20, -1, -1)
        self.label_35 = QLabel(self.groupBox_6)
        self.label_35.setObjectName(u"label_35")

        self.verticalLayout.addWidget(self.label_35)

        self.label_36 = QLabel(self.groupBox_6)
        self.label_36.setObjectName(u"label_36")

        self.verticalLayout.addWidget(self.label_36)

        self.lazy_checkBox = QCheckBox(self.groupBox_6)
        self.lazy_checkBox.setObjectName(u"lazy_checkBox")

        self.verticalLayout.addWidget(self.lazy_checkBox)


        self.verticalLayout_4.addLayout(self.verticalLayout)

        self.verticalSpacer_7 = QSpacerItem(20, 20, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)

        self.verticalLayout_4.addItem(self.verticalSpacer_7)

        self.gridLayout_6 = QGridLayout()
        self.gridLayout_6.setObjectName(u"gridLayout_6")
        self.gridLayout_6.setContentsMargins(20, -1, -1, -1)
        self.preload_month_spinBox = QSpinBox(self.groupBox_6)
        self.preload_month_spinBox.setObjectName(u"preload_month_spinBox")
        self.preload_month_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_month_spinBox, 2, 2, 1, 1)

        self.preload_quarter_spinBox = QSpinBox(self.groupBox_6)
        self.preload_quarter_spinBox.setObjectName(u"preload_quarter_spinBox")
        self.preload_quarter_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_quarter_spinBox, 3, 2, 1, 1)

        self.preload_min5_checkBox = QCheckBox(self.groupBox_6)
        self.preload_min5_checkBox.setObjectName(u"preload_min5_checkBox")

        self.gridLayout_6.addWidget(self.preload_min5_checkBox, 7, 0, 1, 1)

        self.label_33 = QLabel(self.groupBox_6)
        self.label_33.setObjectName(u"label_33")

        self.gridLayout_6.addWidget(self.label_33, 9, 1, 1, 1)

        self.label_26 = QLabel(self.groupBox_6)
        self.label_26.setObjectName(u"label_26")

        self.gridLayout_6.addWidget(self.label_26, 2, 1, 1, 1)

        self.preload_week_spinBox = QSpinBox(self.groupBox_6)
        self.preload_week_spinBox.setObjectName(u"preload_week_spinBox")
        self.preload_week_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_week_spinBox, 1, 2, 1, 1)

        self.label_27 = QLabel(self.groupBox_6)
        self.label_27.setObjectName(u"label_27")

        self.gridLayout_6.addWidget(self.label_27, 3, 1, 1, 1)

        self.preload_year_checkBox = QCheckBox(self.groupBox_6)
        self.preload_year_checkBox.setObjectName(u"preload_year_checkBox")

        self.gridLayout_6.addWidget(self.preload_year_checkBox, 5, 0, 1, 1)

        self.preload_halfyear_spinBox = QSpinBox(self.groupBox_6)
        self.preload_halfyear_spinBox.setObjectName(u"preload_halfyear_spinBox")
        self.preload_halfyear_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_halfyear_spinBox, 4, 2, 1, 1)

        self.preload_day_spinBox = QSpinBox(self.groupBox_6)
        self.preload_day_spinBox.setObjectName(u"preload_day_spinBox")
        self.preload_day_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_day_spinBox, 0, 2, 1, 1)

        self.preload_min30_checkBox = QCheckBox(self.groupBox_6)
        self.preload_min30_checkBox.setObjectName(u"preload_min30_checkBox")

        self.gridLayout_6.addWidget(self.preload_min30_checkBox, 9, 0, 1, 1)

        self.label_28 = QLabel(self.groupBox_6)
        self.label_28.setObjectName(u"label_28")

        self.gridLayout_6.addWidget(self.label_28, 4, 1, 1, 1)

        self.preload_min15_spinBox = QSpinBox(self.groupBox_6)
        self.preload_min15_spinBox.setObjectName(u"preload_min15_spinBox")
        self.preload_min15_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_min15_spinBox, 8, 2, 1, 1)

        self.save_pushButton = QPushButton(self.groupBox_6)
        self.save_pushButton.setObjectName(u"save_pushButton")

        self.gridLayout_6.addWidget(self.save_pushButton, 4, 4, 1, 1)

        self.label_34 = QLabel(self.groupBox_6)
        self.label_34.setObjectName(u"label_34")

        self.gridLayout_6.addWidget(self.label_34, 10, 1, 1, 1)

        self.preload_hour2_checkBox = QCheckBox(self.groupBox_6)
        self.preload_hour2_checkBox.setObjectName(u"preload_hour2_checkBox")

        self.gridLayout_6.addWidget(self.preload_hour2_checkBox, 11, 0, 1, 1)

        self.preload_halfyear_checkBox = QCheckBox(self.groupBox_6)
        self.preload_halfyear_checkBox.setObjectName(u"preload_halfyear_checkBox")

        self.gridLayout_6.addWidget(self.preload_halfyear_checkBox, 4, 0, 1, 1)

        self.preload_day_checkBox = QCheckBox(self.groupBox_6)
        self.preload_day_checkBox.setObjectName(u"preload_day_checkBox")

        self.gridLayout_6.addWidget(self.preload_day_checkBox, 0, 0, 1, 1)

        self.label_24 = QLabel(self.groupBox_6)
        self.label_24.setObjectName(u"label_24")

        self.gridLayout_6.addWidget(self.label_24, 0, 1, 1, 1)

        self.preload_min1_checkBox = QCheckBox(self.groupBox_6)
        self.preload_min1_checkBox.setObjectName(u"preload_min1_checkBox")

        self.gridLayout_6.addWidget(self.preload_min1_checkBox, 6, 0, 1, 1)

        self.preload_min15_checkBox = QCheckBox(self.groupBox_6)
        self.preload_min15_checkBox.setObjectName(u"preload_min15_checkBox")

        self.gridLayout_6.addWidget(self.preload_min15_checkBox, 8, 0, 1, 1)

        self.preload_min1_spinBox = QSpinBox(self.groupBox_6)
        self.preload_min1_spinBox.setObjectName(u"preload_min1_spinBox")
        self.preload_min1_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_min1_spinBox, 6, 2, 1, 1)

        self.preload_min60_checkBox = QCheckBox(self.groupBox_6)
        self.preload_min60_checkBox.setObjectName(u"preload_min60_checkBox")

        self.gridLayout_6.addWidget(self.preload_min60_checkBox, 10, 0, 1, 1)

        self.preload_min60_spinBox = QSpinBox(self.groupBox_6)
        self.preload_min60_spinBox.setObjectName(u"preload_min60_spinBox")
        self.preload_min60_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_min60_spinBox, 10, 2, 1, 1)

        self.label_29 = QLabel(self.groupBox_6)
        self.label_29.setObjectName(u"label_29")

        self.gridLayout_6.addWidget(self.label_29, 5, 1, 1, 1)

        self.preload_hour2_spinBox = QSpinBox(self.groupBox_6)
        self.preload_hour2_spinBox.setObjectName(u"preload_hour2_spinBox")
        self.preload_hour2_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_hour2_spinBox, 11, 2, 1, 1)

        self.preload_week_checkBox = QCheckBox(self.groupBox_6)
        self.preload_week_checkBox.setObjectName(u"preload_week_checkBox")

        self.gridLayout_6.addWidget(self.preload_week_checkBox, 1, 0, 1, 1)

        self.horizontalSpacer_6 = QSpacerItem(20, 20, QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Minimum)

        self.gridLayout_6.addItem(self.horizontalSpacer_6, 4, 3, 1, 1)

        self.preload_quarter_checkBox = QCheckBox(self.groupBox_6)
        self.preload_quarter_checkBox.setObjectName(u"preload_quarter_checkBox")

        self.gridLayout_6.addWidget(self.preload_quarter_checkBox, 3, 0, 1, 1)

        self.label_31 = QLabel(self.groupBox_6)
        self.label_31.setObjectName(u"label_31")

        self.gridLayout_6.addWidget(self.label_31, 7, 1, 1, 1)

        self.label_25 = QLabel(self.groupBox_6)
        self.label_25.setObjectName(u"label_25")

        self.gridLayout_6.addWidget(self.label_25, 1, 1, 1, 1)

        self.preload_month_checkBox = QCheckBox(self.groupBox_6)
        self.preload_month_checkBox.setObjectName(u"preload_month_checkBox")

        self.gridLayout_6.addWidget(self.preload_month_checkBox, 2, 0, 1, 1)

        self.label_30 = QLabel(self.groupBox_6)
        self.label_30.setObjectName(u"label_30")

        self.gridLayout_6.addWidget(self.label_30, 6, 1, 1, 1)

        self.preload_min5_spinBox = QSpinBox(self.groupBox_6)
        self.preload_min5_spinBox.setObjectName(u"preload_min5_spinBox")
        self.preload_min5_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_min5_spinBox, 7, 2, 1, 1)

        self.label_42 = QLabel(self.groupBox_6)
        self.label_42.setObjectName(u"label_42")

        self.gridLayout_6.addWidget(self.label_42, 11, 1, 1, 1)

        self.label_32 = QLabel(self.groupBox_6)
        self.label_32.setObjectName(u"label_32")

        self.gridLayout_6.addWidget(self.label_32, 8, 1, 1, 1)

        self.preload_min30_spinBox = QSpinBox(self.groupBox_6)
        self.preload_min30_spinBox.setObjectName(u"preload_min30_spinBox")
        self.preload_min30_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_min30_spinBox, 9, 2, 1, 1)

        self.preload_year_spinBox = QSpinBox(self.groupBox_6)
        self.preload_year_spinBox.setObjectName(u"preload_year_spinBox")
        self.preload_year_spinBox.setMaximum(999999)

        self.gridLayout_6.addWidget(self.preload_year_spinBox, 5, 2, 1, 1)


        self.verticalLayout_4.addLayout(self.gridLayout_6)

        self.verticalSpacer_8 = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.verticalLayout_4.addItem(self.verticalSpacer_8)


        self.verticalLayout_3.addWidget(self.groupBox_6)

        self.tabWidget.addTab(self.tab_6, "")
        self.tab = QWidget()
        self.tab.setObjectName(u"tab")
        self.verticalLayout_2 = QVBoxLayout(self.tab)
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.verticalSpacer_3 = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.verticalLayout_2.addItem(self.verticalSpacer_3)

        self.horizontalLayout_15 = QHBoxLayout()
        self.horizontalLayout_15.setObjectName(u"horizontalLayout_15")
        self.horizontalLayout_15.setContentsMargins(20, -1, -1, -1)
        self.collect_start_pushButton = QPushButton(self.tab)
        self.collect_start_pushButton.setObjectName(u"collect_start_pushButton")

        self.horizontalLayout_15.addWidget(self.collect_start_pushButton)

        self.collect_status_label = QLabel(self.tab)
        self.collect_status_label.setObjectName(u"collect_status_label")

        self.horizontalLayout_15.addWidget(self.collect_status_label)

        self.horizontalSpacer = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_15.addItem(self.horizontalSpacer)


        self.verticalLayout_2.addLayout(self.horizontalLayout_15)

        self.verticalSpacer = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)

        self.verticalLayout_2.addItem(self.verticalSpacer)

        self.horizontalLayout_13 = QHBoxLayout()
        self.horizontalLayout_13.setObjectName(u"horizontalLayout_13")
        self.horizontalLayout_13.setContentsMargins(20, -1, -1, -1)
        self.label_39 = QLabel(self.tab)
        self.label_39.setObjectName(u"label_39")

        self.horizontalLayout_13.addWidget(self.label_39)

        self.collect_source_comboBox = QComboBox(self.tab)
        self.collect_source_comboBox.addItem("")
        self.collect_source_comboBox.addItem("")
        self.collect_source_comboBox.setObjectName(u"collect_source_comboBox")

        self.horizontalLayout_13.addWidget(self.collect_source_comboBox)

        self.horizontalSpacer_2 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_13.addItem(self.horizontalSpacer_2)


        self.verticalLayout_2.addLayout(self.horizontalLayout_13)

        self.verticalSpacer_2 = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)

        self.verticalLayout_2.addItem(self.verticalSpacer_2)

        self.horizontalLayout_10 = QHBoxLayout()
        self.horizontalLayout_10.setObjectName(u"horizontalLayout_10")
        self.horizontalLayout_10.setContentsMargins(20, -1, -1, -1)
        self.label_23 = QLabel(self.tab)
        self.label_23.setObjectName(u"label_23")

        self.horizontalLayout_10.addWidget(self.label_23)

        self.collect_sample_spinBox = QSpinBox(self.tab)
        self.collect_sample_spinBox.setObjectName(u"collect_sample_spinBox")
        self.collect_sample_spinBox.setMaximum(86400)

        self.horizontalLayout_10.addWidget(self.collect_sample_spinBox)

        self.horizontalSpacer_3 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_10.addItem(self.horizontalSpacer_3)


        self.verticalLayout_2.addLayout(self.horizontalLayout_10)

        self.verticalSpacer_4 = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.verticalLayout_2.addItem(self.verticalSpacer_4)

        self.horizontalLayout_8 = QHBoxLayout()
        self.horizontalLayout_8.setObjectName(u"horizontalLayout_8")
        self.horizontalLayout_8.setContentsMargins(20, -1, -1, -1)
        self.label_21 = QLabel(self.tab)
        self.label_21.setObjectName(u"label_21")

        self.horizontalLayout_8.addWidget(self.label_21)

        self.horizontalLayout_6 = QHBoxLayout()
        self.horizontalLayout_6.setObjectName(u"horizontalLayout_6")
        self.collect_phase1_start_timeEdit = QTimeEdit(self.tab)
        self.collect_phase1_start_timeEdit.setObjectName(u"collect_phase1_start_timeEdit")
        self.collect_phase1_start_timeEdit.setCurrentSection(QDateTimeEdit.HourSection)

        self.horizontalLayout_6.addWidget(self.collect_phase1_start_timeEdit)

        self.label_22 = QLabel(self.tab)
        self.label_22.setObjectName(u"label_22")

        self.horizontalLayout_6.addWidget(self.label_22)

        self.collect_phase1_last_timeEdit = QTimeEdit(self.tab)
        self.collect_phase1_last_timeEdit.setObjectName(u"collect_phase1_last_timeEdit")

        self.horizontalLayout_6.addWidget(self.collect_phase1_last_timeEdit)

        self.horizontalSpacer_4 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_6.addItem(self.horizontalSpacer_4)


        self.horizontalLayout_8.addLayout(self.horizontalLayout_6)


        self.verticalLayout_2.addLayout(self.horizontalLayout_8)

        self.horizontalLayout_9 = QHBoxLayout()
        self.horizontalLayout_9.setObjectName(u"horizontalLayout_9")
        self.horizontalLayout_9.setContentsMargins(20, -1, -1, -1)
        self.label_37 = QLabel(self.tab)
        self.label_37.setObjectName(u"label_37")

        self.horizontalLayout_9.addWidget(self.label_37)

        self.horizontalLayout_7 = QHBoxLayout()
        self.horizontalLayout_7.setObjectName(u"horizontalLayout_7")
        self.collect_phase2_start_timeEdit = QTimeEdit(self.tab)
        self.collect_phase2_start_timeEdit.setObjectName(u"collect_phase2_start_timeEdit")
        self.collect_phase2_start_timeEdit.setCurrentSection(QDateTimeEdit.HourSection)

        self.horizontalLayout_7.addWidget(self.collect_phase2_start_timeEdit)

        self.label_38 = QLabel(self.tab)
        self.label_38.setObjectName(u"label_38")

        self.horizontalLayout_7.addWidget(self.label_38)

        self.collect_phase2_last_timeEdit = QTimeEdit(self.tab)
        self.collect_phase2_last_timeEdit.setObjectName(u"collect_phase2_last_timeEdit")

        self.horizontalLayout_7.addWidget(self.collect_phase2_last_timeEdit)

        self.horizontalSpacer_5 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_7.addItem(self.horizontalSpacer_5)


        self.horizontalLayout_9.addLayout(self.horizontalLayout_7)


        self.verticalLayout_2.addLayout(self.horizontalLayout_9)

        self.verticalSpacer_5 = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.verticalLayout_2.addItem(self.verticalSpacer_5)

        self.horizontalLayout_16 = QHBoxLayout()
        self.horizontalLayout_16.setObjectName(u"horizontalLayout_16")
        self.horizontalLayout_16.setContentsMargins(20, 0, -1, -1)
        self.collect_use_zhima_checkBox = QCheckBox(self.tab)
        self.collect_use_zhima_checkBox.setObjectName(u"collect_use_zhima_checkBox")
        sizePolicy1.setHeightForWidth(self.collect_use_zhima_checkBox.sizePolicy().hasHeightForWidth())
        self.collect_use_zhima_checkBox.setSizePolicy(sizePolicy1)

        self.horizontalLayout_16.addWidget(self.collect_use_zhima_checkBox)


        self.verticalLayout_2.addLayout(self.horizontalLayout_16)

        self.verticalSpacer_6 = QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)

        self.verticalLayout_2.addItem(self.verticalSpacer_6)

        self.horizontalLayout_17 = QHBoxLayout()
        self.horizontalLayout_17.setObjectName(u"horizontalLayout_17")
        self.horizontalLayout_17.setContentsMargins(20, 0, -1, -1)
        self.textBrowser = QTextBrowser(self.tab)
        self.textBrowser.setObjectName(u"textBrowser")

        self.horizontalLayout_17.addWidget(self.textBrowser)


        self.verticalLayout_2.addLayout(self.horizontalLayout_17)

        self.tabWidget.addTab(self.tab, "")
        self.tab_star = QWidget()
        self.tab_star.setObjectName(u"tab_star")
        self.tab_star.setEnabled(True)
        self.verticalLayout_18 = QVBoxLayout(self.tab_star)
        self.verticalLayout_18.setObjectName(u"verticalLayout_18")
        self.verticalSpacer_13 = QSpacerItem(5, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)

        self.verticalLayout_18.addItem(self.verticalSpacer_13)

        self.label_46 = QLabel(self.tab_star)
        self.label_46.setObjectName(u"label_46")
        font = QFont()
        font.setFamilies([u"Arial"])
        self.label_46.setFont(font)
        self.label_46.setWordWrap(True)

        self.verticalLayout_18.addWidget(self.label_46)

        self.line_2 = QFrame(self.tab_star)
        self.line_2.setObjectName(u"line_2")
        self.line_2.setFrameShape(QFrame.Shape.HLine)
        self.line_2.setFrameShadow(QFrame.Shadow.Sunken)

        self.verticalLayout_18.addWidget(self.line_2)

        self.verticalLayout_15 = QVBoxLayout()
        self.verticalLayout_15.setObjectName(u"verticalLayout_15")
        self.label_47 = QLabel(self.tab_star)
        self.label_47.setObjectName(u"label_47")
        font1 = QFont()
        font1.setBold(True)
        self.label_47.setFont(font1)

        self.verticalLayout_15.addWidget(self.label_47)

        self.horizontalLayout_20 = QHBoxLayout()
        self.horizontalLayout_20.setObjectName(u"horizontalLayout_20")
        self.label_45 = QLabel(self.tab_star)
        self.label_45.setObjectName(u"label_45")

        self.horizontalLayout_20.addWidget(self.label_45)

        self.email_lineEdit = QLineEdit(self.tab_star)
        self.email_lineEdit.setObjectName(u"email_lineEdit")

        self.horizontalLayout_20.addWidget(self.email_lineEdit)

        self.fetch_trial_pushButton = QPushButton(self.tab_star)
        self.fetch_trial_pushButton.setObjectName(u"fetch_trial_pushButton")

        self.horizontalLayout_20.addWidget(self.fetch_trial_pushButton)


        self.verticalLayout_15.addLayout(self.horizontalLayout_20)


        self.verticalLayout_18.addLayout(self.verticalLayout_15)

        self.line_3 = QFrame(self.tab_star)
        self.line_3.setObjectName(u"line_3")
        self.line_3.setFrameShape(QFrame.Shape.HLine)
        self.line_3.setFrameShadow(QFrame.Shadow.Sunken)

        self.verticalLayout_18.addWidget(self.line_3)

        self.verticalLayout_16 = QVBoxLayout()
        self.verticalLayout_16.setObjectName(u"verticalLayout_16")
        self.label_48 = QLabel(self.tab_star)
        self.label_48.setObjectName(u"label_48")
        self.label_48.setFont(font1)

        self.verticalLayout_16.addWidget(self.label_48)

        self.horizontalLayout_22 = QHBoxLayout()
        self.horizontalLayout_22.setObjectName(u"horizontalLayout_22")

        self.verticalLayout_16.addLayout(self.horizontalLayout_22)

        self.horizontalLayout_26 = QHBoxLayout()
        self.horizontalLayout_26.setObjectName(u"horizontalLayout_26")
        self.horizontalLayout_26.setSizeConstraint(QLayout.SetFixedSize)
        self.label_44 = QLabel(self.tab_star)
        self.label_44.setObjectName(u"label_44")
        sizePolicy3 = QSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        sizePolicy3.setHorizontalStretch(0)
        sizePolicy3.setVerticalStretch(0)
        sizePolicy3.setHeightForWidth(self.label_44.sizePolicy().hasHeightForWidth())
        self.label_44.setSizePolicy(sizePolicy3)
        self.label_44.setMaximumSize(QSize(300, 160))
        self.label_44.setPixmap(QPixmap(u"../images/star.png"))
        self.label_44.setScaledContents(True)

        self.horizontalLayout_26.addWidget(self.label_44, 0, Qt.AlignLeft)

        self.label_56 = QLabel(self.tab_star)
        self.label_56.setObjectName(u"label_56")
        self.label_56.setMaximumSize(QSize(160, 160))
        self.label_56.setFrameShape(QFrame.NoFrame)
        self.label_56.setPixmap(QPixmap(u"../images/liandongxiaopu.png"))
        self.label_56.setScaledContents(True)

        self.horizontalLayout_26.addWidget(self.label_56, 0, Qt.AlignLeft)

        self.horizontalSpacer_15 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_26.addItem(self.horizontalSpacer_15)


        self.verticalLayout_16.addLayout(self.horizontalLayout_26)


        self.verticalLayout_18.addLayout(self.verticalLayout_16)

        self.line_4 = QFrame(self.tab_star)
        self.line_4.setObjectName(u"line_4")
        self.line_4.setFrameShape(QFrame.Shape.HLine)
        self.line_4.setFrameShadow(QFrame.Shadow.Sunken)

        self.verticalLayout_18.addWidget(self.line_4)

        self.verticalLayout_17 = QVBoxLayout()
        self.verticalLayout_17.setObjectName(u"verticalLayout_17")
        self.label_49 = QLabel(self.tab_star)
        self.label_49.setObjectName(u"label_49")
        self.label_49.setFont(font1)

        self.verticalLayout_17.addWidget(self.label_49)

        self.label_license = QLabel(self.tab_star)
        self.label_license.setObjectName(u"label_license")

        self.verticalLayout_17.addWidget(self.label_license)


        self.verticalLayout_18.addLayout(self.verticalLayout_17)

        self.verticalSpacer_21 = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.verticalLayout_18.addItem(self.verticalSpacer_21)

        self.tabWidget.addTab(self.tab_star, "")

        self.horizontalLayout_12.addWidget(self.tabWidget)

        self.verticalLayout_6 = QVBoxLayout()
        self.verticalLayout_6.setObjectName(u"verticalLayout_6")
        self.label_41 = QLabel(self.centralwidget)
        self.label_41.setObjectName(u"label_41")

        self.verticalLayout_6.addWidget(self.label_41)

        self.log_textEdit = QTextEdit(self.centralwidget)
        self.log_textEdit.setObjectName(u"log_textEdit")
        self.log_textEdit.setReadOnly(True)
        self.log_textEdit.setAcceptRichText(True)

        self.verticalLayout_6.addWidget(self.log_textEdit)


        self.horizontalLayout_12.addLayout(self.verticalLayout_6)

        MainWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(MainWindow)

        self.tabWidget.setCurrentIndex(0)


        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"\u6570\u636e\u5bfc\u5165\u5de5\u5177", None))
        self.groupBox_2.setTitle(QCoreApplication.translate("MainWindow", u"\u6570\u636e\u6e90\u8bbe\u7f6e", None))
        self.pytdx_radioButton.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528Pytdx\u4e0b\u8f7d\u6570\u636e", None))
        self.label_16.setText(QCoreApplication.translate("MainWindow", u"\u540c\u65f6\u4f7f\u7528", None))
        self.label_17.setText(QCoreApplication.translate("MainWindow", u"\u4e2a\u901a\u8fbe\u4fe1\u670d\u52a1\u5668\u8fdb\u884c\u4e0b\u8f7d", None))
        self.tdx_radioButton.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528\u901a\u8fbe\u4fe1\u76d8\u540e\u6570\u636e\uff08\u4e0d\u652f\u6301\u5206\u7b14\u3001\u5206\u65f6\u6570\u636e\uff0c\u9700\u8981\u914d\u7f6e\u5b89\u88c5\u8def\u5f84\uff09", None))
        self.select_tdx_dir_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u9009\u62e9", None))
        self.label_2.setText(QCoreApplication.translate("MainWindow", u"\u901a\u8fbe\u4fe1\u5b89\u88c5\u76ee\u5f55\uff1a", None))
        self.qmt_radioButton.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528 qmt \u6570\u636e\u4e0b\u8f7d\uff08\u9700\u8981 miniqmt\uff09\uff08\u5c4f\u853d\uff0cqmt\u4e0b\u8f7d\u6570\u636e\u592a\u6162\uff0c\u4e14\u4e0d\u7a33\u5b9a\uff09", None))
        self.groupBox_7.setTitle(QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u8bbe\u7f6e", None))
        self.import_weight_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u4e0b\u8f7d\u6743\u606f\u6570\u636e", None))
        self.import_history_finance_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u4e0b\u8f7d\u5386\u53f2\u8d22\u52a1\u6570\u636e", None))
        self.import_block_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u4e0b\u8f7d\u884c\u4e1a\u4e0e\u6307\u6570\u677f\u5757", None))
        self.import_stock_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u80a1\u7968", None))
        self.import_fund_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u57fa\u91d1", None))
        self.import_future_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u671f\u8d27", None))
        self.import_day_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u65e5\u7ebf", None))
        self.import_min5_checkBox.setText(QCoreApplication.translate("MainWindow", u"5\u5206\u949f\u7ebf", None))
        self.import_min_checkBox.setText(QCoreApplication.translate("MainWindow", u"1\u5206\u949f\u7ebf", None))
        self.import_trans_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u5206\u7b14", None))
        self.import_time_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u5206\u65f6", None))
        self.label_7.setText(QCoreApplication.translate("MainWindow", u"\u521d\u6b21\u5bfc\u5165\u5206\u65f6\u8d77\u59cb\u65e5\u671f\uff1a", None))
        self.label_6.setText(QCoreApplication.translate("MainWindow", u"\u521d\u6b21\u5bfc\u5165\u5206\u7b14\u8d77\u59cb\u65e5\u671f\uff1a", None))
        self.label_3.setText(QCoreApplication.translate("MainWindow", u"\u521d\u6b21\u5bfc\u5165\u65e5\u7ebf\u8d77\u59cb\u65e5\u671f\uff1a", None))
        self.label_10.setText(QCoreApplication.translate("MainWindow", u"\u521d\u6b21\u5bfc\u51655\u5206\u949f\u7ebf\u8d77\u59cb\u65e5\u671f\uff1a", None))
        self.label_15.setText(QCoreApplication.translate("MainWindow", u"\u521d\u6b21\u5bfc\u51651\u5206\u949f\u7ebf\u8d77\u59cb\u65e5\u671f\uff1a", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_4), QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u8bbe\u7f6e", None))
        self.groupBox_4.setTitle(QCoreApplication.translate("MainWindow", u"\u5b58\u50a8\u5f15\u64ce\u8bbe\u7f6e", None))
        self.enable_hdf55_radioButton.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528HDF5\uff08\u63a8\u8350\uff09", None))
        self.enable_mysql_radioButton.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528MYSQL", None))
        self.enable_clickhouse_radioButton.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528Clickhouse(\u6350\u8d60\u6743\u76ca\uff09", None))
        self.groupBox_3.setTitle(QCoreApplication.translate("MainWindow", u"HDF5\u5b58\u50a8\u8bbe\u7f6e", None))
        self.hdf5_dir_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u9009\u62e9", None))
        self.label.setText(QCoreApplication.translate("MainWindow", u"\u76ee\u6807\u6570\u636e\uff08HDF5\uff09\u5b58\u653e\u76ee\u5f55\uff1a", None))
        self.groupBox.setTitle(QCoreApplication.translate("MainWindow", u"MYSQL\u5b58\u50a8\u8bbe\u7f6e", None))
        self.label_11.setText(QCoreApplication.translate("MainWindow", u"\u7528\u6237\u540d\uff1a", None))
        self.label_19.setText(QCoreApplication.translate("MainWindow", u"\u7aef\u53e3\u53f7\uff1a", None))
        self.label_13.setText(QCoreApplication.translate("MainWindow", u"\u5bc6\u7801", None))
        self.mysql_tmpdir_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u9009\u62e9", None))
        self.mysql_test_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u6d4b\u8bd5\u8fde\u63a5", None))
        self.label_20.setText(QCoreApplication.translate("MainWindow", u"\u4e34\u672c\u673a\u65f6\u6587\u4ef6\u76ee\u5f55\uff1a", None))
        self.label_18.setText(QCoreApplication.translate("MainWindow", u"\u4e3b\u673a\u540d/IP\uff1a", None))
        self.groupBox_8.setTitle(QCoreApplication.translate("MainWindow", u"Clickhouse\u8bbe\u7f6e(\u6350\u8d60\u6743\u76ca)", None))
        self.label_53.setText(QCoreApplication.translate("MainWindow", u"\u672c\u673a\u4e34\u65f6\u6587\u4ef6\u76ee\u5f55\uff1a", None))
        self.label_54.setText(QCoreApplication.translate("MainWindow", u"\u4e3b\u673a\u540d/IP\uff1a", None))
        self.label_51.setText(QCoreApplication.translate("MainWindow", u"HTTP\u7aef\u53e3\u53f7\uff1a", None))
        self.label_50.setText(QCoreApplication.translate("MainWindow", u"\u7528\u6237\u540d\uff1a", None))
        self.label_55.setText(QCoreApplication.translate("MainWindow", u"TCP\u7aef\u53e3\u53f7", None))
        self.label_52.setText(QCoreApplication.translate("MainWindow", u"\u5bc6\u7801", None))
        self.clickhouse_test_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u6d4b\u8bd5\u8fde\u63a5", None))
        self.clickhouse_tmpdir_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u9009\u62e9", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_3), QCoreApplication.translate("MainWindow", u"\u5b58\u50a8\u8bbe\u7f6e", None))
        self.label_43.setText(QCoreApplication.translate("MainWindow", u"    \u6ce8\uff1aHDF5\u5b58\u50a8\u5bfc\u5165\u65f6\uff0c\u8bf7\u5173\u95ed\u5176\u4ed6 hikyuu \u8fdb\u7a0b\uff0cHDF5\u4e0d\u652f\u6301\u540c\u65f6\u8bfb\u5199\uff01", None))
        self.sched_import_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u542f\u52a8\u5b9a\u65f6\u5bfc\u5165", None))
        self.label_40.setText(QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u6267\u884c\u65f6\u95f4\uff1a", None))
        self.start_import_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u624b\u5de5\u6267\u884c\u5bfc\u5165", None))
        self.import_status_label.setText(QCoreApplication.translate("MainWindow", u"\u8bf7\u52ff\u76d8\u4e2d\u5bfc\u5165\uff01", None))
        self.groupBox_5.setTitle(QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u8fdb\u5c55", None))
        self.hdf5_weight_label.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.label_14.setText(QCoreApplication.translate("MainWindow", u"\u5bfc\u51651\u5206\u949f\u7ebf\uff1a", None))
        self.label_4.setText(QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u5206\u7b14\u6570\u636e\uff1a", None))
        self.label_8.setText(QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u65e5\u7ebf\uff1a", None))
        self.label_5.setText(QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u5206\u65f6\u6570\u636e\uff1a", None))
        self.label_12.setText(QCoreApplication.translate("MainWindow", u"\u5bfc\u51655\u5206\u949f\u7ebf\uff1a", None))
        self.label_9.setText(QCoreApplication.translate("MainWindow", u"\u5bfc\u5165\u6743\u606f\u6570\u636e\uff1a", None))
        self.import_detail_textEdit.setHtml(QCoreApplication.translate("MainWindow", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u4e0a\u8bc1\u65e5\u7ebf\u8bb0\u5f55\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u6df1\u8bc1\u65e5\u7ebf\u8bb0\u5f55\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; "
                        "text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u4e0a\u8bc15\u5206\u949f\u7ebf\u8bb0\u5f55\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u6df1\u8bc15\u5206\u949f\u7ebf\u8bb0\u5f55\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u4e0a\u8bc11\u5206\u949f\u7ebf\u8bb0\u5f55\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u6df1\u8bc11\u5206\u949f\u7ebf\u8bb0\u5f55\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u4e0a\u8bc1\u5206\u7b14\u8bb0\u5f55\uff1a"
                        "</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u6df1\u8bc1\u5206\u7b14\u8bb0\u5f55\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u4e0a\u8bc1\u5206\u65f6\u6570\u636e\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u6df1\u8bc1\u5206\u65f6\u6570\u636e\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u5bfc\u5165\u6743\u606f\u6570\u636e\u6570\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><s"
                        "pan style=\" font-size:9pt;\">\u5bfc\u5165\u5b8c\u6bd5\uff01</span></p></body></html>", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), QCoreApplication.translate("MainWindow", u"\u6267\u884c\u5bfc\u5165", None))
        self.groupBox_6.setTitle(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u8bbe\u7f6e", None))
        self.label_35.setText(QCoreApplication.translate("MainWindow", u"\u6b64\u5904\u4e3a Hikyuu \u8fd0\u884c\u65f6\u7684\u6570\u636e\u9884\u52a0\u8f7d\u8bbe\u7f6e\uff0c\u8bf7\u6839\u636e\u673a\u5668\u5185\u5b58\u5927\u5c0f\u9009\u62e9", None))
        self.label_36.setText(QCoreApplication.translate("MainWindow", u"\uff08\u76ee\u524d\u52a0\u8f7d\u5168\u90e8\u65e5\u7ebf\u6570\u636e\u76ee\u524d\u9700\u8981\u7ea6\u9700900M\u5185\u5b58\uff09", None))
        self.lazy_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528\u61d2\u52a0\u8f7d\u6a21\u5f0f(\u4ec5\u5bf9\u65e5\u7ebf\u4ee5\u4e0b\u7ea7\u522b\u751f\u6548, \u6350\u8d60\u6743\u76ca)", None))
        self.preload_min5_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d5\u5206\u949f\u7ebf", None))
        self.label_33.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.label_26.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.label_27.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.preload_year_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u5e74\u7ebf", None))
        self.preload_min30_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d30\u5206\u949f\u7ebf", None))
        self.label_28.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.save_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u4fdd\u5b58\u8bbe\u7f6e", None))
        self.label_34.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.preload_hour2_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d120\u5206\u949f\u7ebf", None))
        self.preload_halfyear_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u534a\u5e74\u7ebf", None))
        self.preload_day_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u65e5\u7ebf", None))
        self.label_24.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.preload_min1_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d1\u5206\u949f\u7ebf", None))
        self.preload_min15_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d15\u5206\u949f\u7ebf", None))
        self.preload_min60_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d60\u5206\u949f\u7ebf", None))
        self.label_29.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.preload_week_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u5468\u7ebf", None))
        self.preload_quarter_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u5b63\u7ebf", None))
        self.label_31.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.label_25.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.preload_month_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u6708\u7ebf", None))
        self.label_30.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.label_42.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.label_32.setText(QCoreApplication.translate("MainWindow", u"\u6700\u5927\u7f13\u5b58\u6570\u91cf\uff1a", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_6), QCoreApplication.translate("MainWindow", u"\u9884\u52a0\u8f7d\u8bbe\u7f6e", None))
        self.collect_start_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u542f\u52a8\u91c7\u96c6", None))
        self.collect_status_label.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.label_39.setText(QCoreApplication.translate("MainWindow", u"\u884c\u60c5\u6570\u636e\u6e90\uff1a", None))
        self.collect_source_comboBox.setItemText(0, QCoreApplication.translate("MainWindow", u"qq", None))
        self.collect_source_comboBox.setItemText(1, QCoreApplication.translate("MainWindow", u"qmt", None))

        self.label_23.setText(QCoreApplication.translate("MainWindow", u"\u91c7\u96c6\u95f4\u9694\uff08\u79d2\uff09\uff1a", None))
        self.label_21.setText(QCoreApplication.translate("MainWindow", u"\u6267\u884c\u65f6\u95f4\u6bb51\uff1a", None))
        self.label_22.setText(QCoreApplication.translate("MainWindow", u"-", None))
        self.label_37.setText(QCoreApplication.translate("MainWindow", u"\u6267\u884c\u65f6\u95f4\u6bb52\uff1a", None))
        self.label_38.setText(QCoreApplication.translate("MainWindow", u"-", None))
        self.collect_use_zhima_checkBox.setText(QCoreApplication.translate("MainWindow", u"\u4f7f\u7528\u829d\u9ebb\u4ee3\u7406", None))
        self.textBrowser.setHtml(QCoreApplication.translate("MainWindow", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">\u6ce8\uff1a</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt; font-weight:600; color:#ff0000;\">1\u3001\u884c\u60c5\u91c7\u96c6\u670d\u52a1\u4ec5\u5bf9\u9884\u52a0\u8f7d\u6570\u636e\u6709\u6548</span><span style=\" font-size:9pt;\">\uff0c\u5728\u884c\u60c5\u91c7\u96c6\u670d\u52a1"
                        "\u8fd0\u884c\u671f\u95f4\uff0chikyuu.interactive\u8fd0\u884c\u65f6\u5c06\u81ea\u52a8\u8fde\u63a5\u91c7\u96c6\u670d\u52a1\u83b7\u53d6\u884c\u60c5\u6570\u636e\uff0c\u5e76\u66f4\u65b0\u9884\u52a0\u8f7d\u7684\u5185\u5bb9\u6570\u636e\u3002</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">2\u3001\u5982\u4f7f\u7528\u829d\u9ebb\u4ee3\u7406\uff08</span><a href=\"http://h.zhimaruanjian.com/\"><span style=\" font-size:9pt; text-decoration: underline; color:#0000ff;\">http://h.zhimaruanjian.com/</span></a><span style=\" font-size:9pt;\">\uff09\uff0c\u8bf7\u81ea\u884c\u7533\u8bf7\uff08\u9700\u4ed8\u8d39\uff09\uff0c\u5e76\u786e\u4fddip\u4e3a\u5176\u767d\u540d\u5355\u3002</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0"
                        "px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt; font-weight:696; color:#0000ff;\">3\u3001\u6b64\u5904\u91c7\u96c6\u4e3a\u7f51\u7edc\u91c7\u96c6\uff0c\u66f4\u63a8\u8350\u76f4\u63a5\u8fd0\u884c\u5b89\u88c5\u76ee\u5f55\u4e0bgui\u5b50\u76ee\u5f55\u4e0b\u7684 start_qmt.py ,\u4f7f\u7528miniqmt \u5b9e\u65f6\u670d\u52a1\u3002\u8be5\u7a0b\u5e8f\u72ec\u7acb\u8fd0\u884c\uff0c\u4e0d\u7528\u5173\u95ed\uff0c\u548c\u8fd9\u91cc\u7684\u91c7\u96c6\u6548\u679c\u4e00\u6837\u3002\u6ce8\u610f\uff1aminiqmt\u9700\u8981QMT\u4ea4\u6613\u7aef\u914d\u5408\uff0c\u4e14\u5728\u540c\u4e00\u673a\u5668\u4e0a\u6267\u884c\u3002</span></p></body></html>", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), QCoreApplication.translate("MainWindow", u"\u884c\u60c5\u91c7\u96c6\u670d\u52a1", None))
        self.label_46.setText(QCoreApplication.translate("MainWindow", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Arial'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">Hikyuu </span><span style=\" font-size:12pt; font-weight:700;\">\u4e13\u6ce8\u4e8e\u91cf\u5316\u4ea4\u6613\u9886\u57df\u7684\u6838\u5fc3\u6280\u672f\u6784\u5efa\uff0c\u6db5\u76d6\u4ea4\u6613\u6a21\u578b\u5f00\u53d1\u3001\u6781\u901f\u8ba1\u7b97\u5f15\u64ce\u3001\u9ad8\u6548\u56de\u6d4b\u6846\u67b6\u53ca\u5b9e\u76d8\u62d3\u5c55\u80fd\u529b</span><span style=\" font-size"
                        ":12pt;\">\uff0c\u5b9a\u4f4d\u4e3a\u91cf\u5316\u4ea4\u6613\u7684\u57fa\u7840\u8bbe\u65bd\u7ea7\u8ba1\u7b97\u5f15\u64ce\uff0c\u4e3a\u91cf\u5316\u4ea4\u6613\u7231\u597d\u8005\u63d0\u4f9b\u9ad8\u6027\u80fd\u5e95\u5c42\u67b6\u6784\u652f\u6301\u3002\u968f\u7740\u793e\u533a\u89c4\u6a21\u6269\u5927\uff0c\u5206\u6563\u4e86\u4f5c\u8005\u5728\u7b56\u7565\u7814\u7a76\u4e0a\u7684\u7cbe\u529b\u3002\u4e3a\u4fdd\u969c\u9879\u76ee\u7684\u53ef\u6301\u7eed\u6027\uff0c\u73b0\u5bf9\u6350\u8d60\u7528\u6237\u63d0\u4f9b\u90e8\u5206\u989d\u5916\u529f\u80fd\u4f5c\u4e3a\u56de\u9988\uff0c\u611f\u8c22\u793e\u533a\u4f19\u4f34\u7684\u652f\u6301\uff01</span></p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">\u6350\u8d60\u529f\u80fd\u4ee5\u63d2\u4ef6\u7684\u65b9\u5f0f\u63d0\u4f9b\uff0c\u91c7\u7528\u72ec\u7acb\u6388\u6743\u8bb8\u53ef\uff0c\u5b8c\u5168\u5728 hikyuu \u4e4b\u5916\uff0c\u5bf9\u559c\u6b22\u81ea\u884c\u7f16\u8bd1\u6269\u5c55"
                        "\u7684\u670b\u53cb\u6ca1\u6709\u5f71\u54cd\u3002\u56e0\u63d2\u4ef6\u8bb8\u53ef\u6388\u6743\u9700\u8981\u91c7\u96c6\u786c\u4ef6\u4fe1\u606f\uff0c\u5982\u6709\u7591\u8651\u53ea\u8981\u4e0d\u7533\u8bf7\u8bd5\u7528\u8bb8\u53ef\u548c\u6b63\u5f0f\u8bb8\u53ef\u6388\u6743\uff0c\u4e0d\u4f1a\u89e6\u53d1\u786c\u4ef6\u4fe1\u606f\u91c7\u96c6\uff0c\u5982\u7533\u8bf7\uff0c\u89c6\u4e3a\u540c\u610f\u91c7\u96c6\u3002</span></p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt; font-weight:700;\">\u8be6\u60c5\u53c2\u89c1\uff1a</span><a href=\"https://hikyuu.readthedocs.io/zh-cn/latest/vip/vip-plan.html\"><span style=\" font-size:12pt; text-decoration: underline; color:#3586ff;\">\u6350\u8d60\u6743\u76ca</span></a><span style=\" font-size:12pt; font-weight:700;\"> \uff0c\u611f\u8c22\u5927\u5bb6\u7684\u652f\u6301\uff01</span></p></body></html>", None))
        self.label_47.setText(QCoreApplication.translate("MainWindow", u"\u7533\u8bf7\u6350\u8d60\u529f\u80fd\u8bd5\u7528\uff0830\u5929\u8bd5\u7528\uff09", None))
        self.label_45.setText(QCoreApplication.translate("MainWindow", u"\u7535\u5b50\u90ae\u4ef6\u5730\u5740:", None))
        self.fetch_trial_pushButton.setText(QCoreApplication.translate("MainWindow", u"\u7533\u8bf7\u8bd5\u7528\u8bb8\u53ef", None))
        self.label_48.setText(QCoreApplication.translate("MainWindow", u"\u52a0\u5165\u77e5\u8bc6\u661f\u7403\u8fdb\u884c\u6350\u8d60\uff08\u53ef\u540c\u65f6\u57283\u53f0\u8bbe\u5907\u4e0a\u4f7f\u7528\u6350\u8d60\u529f\u80fd\uff09,\u6216\u8005\u8bf7\u4f5c\u8005\u559d\u676f\u2615\ufe0f\uff0c\u8c22\u8c22", None))
        self.label_44.setText("")
        self.label_56.setText("")
        self.label_49.setText(QCoreApplication.translate("MainWindow", u"\u5f53\u524d\u6350\u8d60\u6388\u6743\u4fe1\u606f", None))
        self.label_license.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_star), QCoreApplication.translate("MainWindow", u"\u9879\u76ee\u6350\u8d60", None))
        self.label_41.setText(QCoreApplication.translate("MainWindow", u"\u6267\u884c\u65e5\u5fd7", None))
    # retranslateUi

