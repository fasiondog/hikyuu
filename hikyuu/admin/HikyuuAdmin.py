# -*- coding: utf-8 -*-
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
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

import logging
import sys
sys.path.append('.')

from PyQt5 import QtCore, QtGui, QtWidgets
import qdarkstyle

from UiConfig import UiConfig
from widget.HkuSessionViewWidget import HkuSessionViewWidget
from dialog import *
from widget import *
from data import *

_translate = QtCore.QCoreApplication.translate


class MyMainWindow(QtWidgets.QMainWindow):
    def __init__(self, capture_output=True, use_dark_style=False):
        super().__init__()
        appid = 'HikyuuAdmin'
        QtWidgets.QApplication.setApplicationName(appid)
        QtWidgets.QApplication.setOrganizationName("org.hikyuu")

        # 国际化支持
        loc = QtCore.QLocale()
        if loc.language() == QtCore.QLocale.Chinese:
            self.trans = QtCore.QTranslator()
            self.trans.load("language/zh_CN.qm")  # 读取qm语言包
            _app = QtWidgets.QApplication.instance()  # 应用实例
            _app.installTranslator(self.trans)  # 将翻译者安装到实例中

        # 设置程序图标资源
        # 如未能正常显示图标，请检查 "import resource" 是否
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/logo/logo_16.png"))
        icon.addPixmap(QtGui.QPixmap(":/logo/logo_32.png"))
        icon.addPixmap(QtGui.QPixmap(":/logo/logo_48.png"))
        icon.addPixmap(QtGui.QPixmap(":/logo/logo_64.png"))
        icon.addPixmap(QtGui.QPixmap(":/logo/logo_128.png"))
        icon.addPixmap(QtGui.QPixmap(":/logo/logo_256.png"))
        self.setWindowIcon(icon)
        if sys.platform == 'win32':
            # window下设置任务栏图片
            import ctypes
            ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(appid)

        self.ui_config = UiConfig()
        self.setObjectName("HikyuuAdminMainWindow")
        self.setWindowTitle(_translate("MainWindow", "Hikyuu Strategy Server Manager"))

        # 必须放在 resize 窗口大小之前
        style = self.ui_config.get('main_window', 'style', fallback='normal_style')
        if style == 'dark_style':
            QtWidgets.qApp.setStyleSheet(qdarkstyle.load_stylesheet(qt_api='pyqt5'))

        if self.ui_config.getboolean('main_window', 'maximized', fallback=False):
            self.showMaximized()
        else:
            self.resize(
                self.ui_config.getint('main_window', 'width', fallback=800),
                self.ui_config.getint('main_window', 'height', fallback=500)
            )


        self.initAction()
        self.initMenuBar()
        self.initMenu()
        self.initToolBar()
        self.initActionConnect()
        self.initMainTabWidget()
        self.initDockWidgets()
        self.statusBar().showMessage(_translate('MainWindow', 'Running'))
        QtCore.QMetaObject.connectSlotsByName(self)

        self.db = LocalDatabase()

    def closeEvent(self, event):
        self.ui_config.save(self)
        event.accept()

    def initAction(self):
        self.action_dict = dict(
            action_file_session=QtWidgets.QAction(
                QtGui.QIcon(":/icon/server.png"), _translate("MainWindow", "&Session"), self
            ),
            action_file_quit=QtWidgets.QAction(QtGui.QIcon(":/icon/quit.png"), _translate('MainWindow', '&Quit'), self),
            action_view_normal_style=QtWidgets.QAction(_translate('MainWindow', 'Normal style'), self),
            action_view_dark_style=QtWidgets.QAction(_translate('MainWindow', 'Dark style'), self),
            action_about=QtWidgets.QAction(_translate('MainWindow', 'About'), self),
            action_about_qt=QtWidgets.QAction(_translate('MainWindow', 'About Qt'), self),
        )
        self.action_dict['action_file_session'].setStatusTip(_translate('MainWindow', 'Session Manager'))
        self.action_dict['action_file_quit'].setStatusTip(_translate('MainWindow', 'Quit Application'))
        self.action_dict['action_about_qt'].setStatusTip(_translate('MainWindow', "Show the Qt library's About box"))
        self.action_dict['action_view_normal_style'].setObjectName('normal_style')
        self.action_dict['action_view_normal_style'].setStatusTip(_translate('MainWindow', 'Switch to normal style'))
        self.action_dict['action_view_dark_style'].setObjectName('dark_style')
        self.action_dict['action_view_dark_style'].setStatusTip(_translate('MainWindow', 'Switch to dark style'))

    def initMenuBar(self):
        self.menubar_dict = dict(
            menu_file=self.menuBar().addMenu(_translate('MainWindow', "&File(F)")),
            menu_view=self.menuBar().addMenu(_translate('MainWindow', "&View(V)")),
            menu_help=self.menuBar().addMenu(_translate('MainWindow', "&Help(H)"))
        )

    def initMenu(self):
        style_menu = self.menubar_dict['menu_view'].addMenu(_translate('MainWindow', 'Skin style'))
        self.menu_dict = dict(
            menu_file_session=self.menubar_dict['menu_file'].addAction(self.action_dict['action_file_session']),
            menu_file_quit=self.menubar_dict['menu_file'].addAction(self.action_dict['action_file_quit']),
            menu_view_normal_style=style_menu.addAction(self.action_dict['action_view_normal_style']),
            menu_view_dark_style=style_menu.addAction(self.action_dict['action_view_dark_style']),
            menu_about=self.menubar_dict['menu_help'].addAction(self.action_dict['action_about']),
            menu_about_qt=self.menubar_dict['menu_help'].addAction(self.action_dict['action_about_qt']),
        )

    def initToolBar(self):
        self.setUnifiedTitleAndToolBarOnMac(True)
        file_toolbar = self.addToolBar('File')
        file_toolbar.addAction(self.action_dict['action_file_session'])
        file_toolbar.addAction(self.action_dict['action_file_quit'])

    def initActionConnect(self):
        self.action_dict['action_file_session'].triggered.connect(self.actionEditSession)
        self.action_dict['action_file_quit'].triggered.connect(self.close)
        self.action_dict['action_about'].triggered.connect(self.actionAbout)
        self.action_dict['action_about_qt'].triggered.connect(QtWidgets.QApplication.aboutQt)
        self.action_dict['action_view_normal_style'].triggered.connect(self.actionChangStyle)
        self.action_dict['action_view_dark_style'].triggered.connect(self.actionChangStyle)

    def initMainTabWidget(self):
        self.main_tab = QtWidgets.QTabWidget(self)
        self.setCentralWidget(self.main_tab)

    def initDockWidgets(self):
        self.server_view_dock = HkuSessionViewWidget(self)
        self.server_view_dock.setFeatures(QtWidgets.QDockWidget.DockWidgetMovable)  # 禁止关闭
        self.server_view_dock.setMinimumWidth(200)
        title_bar = self.server_view_dock.titleBarWidget()
        self.server_view_dock.setTitleBarWidget(QtWidgets.QWidget())
        del title_bar
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.server_view_dock)

    def actionAbout(self):
        msg = _translate(
            'MainWindow', "<p><b>Hikyuu Strategy Server Manager</b><p>"
            "<p>Hikyuu strategy server management is used to "
            "manage quant trading strategies based on hikyuu "
            "quant framework</p>"
            "<p><b>Hikyuu Quant Framework</b></p>"
            "It is a high performance open source quantitative "
            "trading research framework based on C++/Python, "
            "which is used for stratgy analysis and back testing."
            "Now it only used in Chinese stock market)</p>"
            '<p>see more: <a href="https://hikyuu.org">https://hikyuu.org<a></p>'
        )
        QtWidgets.QMessageBox.about(self, _translate('MainWindow', 'About Hikyuu Strategy Server Manager'), msg)

    def actionChangStyle(self):
        QtWidgets.qApp.setStyleSheet('')
        style_name = self.sender().objectName()
        if style_name == 'dark_style':
            QtWidgets.qApp.setStyleSheet(qdarkstyle.load_stylesheet(qt_api='pyqt5'))
        self.ui_config.set('main_window', 'style', style_name)

    def actionEditSession(self):
        item = self.server_view_dock.tree.currentItem()
        print(item.text(0) if item else 'None')
        edit_session_dialog = HkuEditSessionDialog()
        if edit_session_dialog.exec():
            print("ok")
        edit_session_dialog.destroy()

def main_core():
    # 自适应分辨率，防止字体显示不全
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_UseHighDpiPixmaps)
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling)

    app = QtWidgets.QApplication(sys.argv)
    use_dark_style = False  # 使用暗黑主题
    if use_dark_style:
        import qdarkstyle
        app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
    if (len(sys.argv) > 1 and sys.argv[1] == '0'):
        FORMAT = '%(asctime)-15s [%(levelname)s]: %(message)s [%(name)s::%(funcName)s]'
        logging.basicConfig(format=FORMAT, level=logging.INFO, handlers=[
            logging.StreamHandler(),
        ])
        main_win = MyMainWindow(capture_output=False, use_dark_style=use_dark_style)
    else:
        main_win = MyMainWindow(capture_output=True, use_dark_style=use_dark_style)

    main_win.show()
    exit_code = app.exec()
    if exit_code == 888:
        # 应用中使用 qApp.exit(888) 指示重启
        del main_win
        del app  #必须，否则最终无法正常退出应用
        main_core()
    else:
        sys.exit()


if __name__ == "__main__":
    main_core()
