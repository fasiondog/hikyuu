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
import os

from PyQt5.QtWidgets import QVBoxLayout

cur_dir = os.path.dirname(__file__)

# 将当前目录加入 sys.path 以便其下子模块可以互相引用
sys.path.append(cur_dir)

# 将hikyuu目录加入 sys.path 以便直接引用 utils 包
sys.path.append(os.path.split(cur_dir)[0])

from PyQt5 import QtCore, QtGui, QtWidgets
import qdarkstyle

from UiConfig import UiConfig
from translate import _translate
from dialog import *
from widget import *
from data import (get_local_db, SessionModel)
from service import AssisService


class MyMainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        appid = 'HikyuuAdmin'
        QtWidgets.QApplication.setApplicationName(appid)
        QtWidgets.QApplication.setOrganizationName("org.hikyuu")

        # 国际化支持
        loc = QtCore.QLocale()
        if loc.language() == QtCore.QLocale.Chinese:
            self.trans = QtCore.QTranslator()
            self.trans.load("{}/language/zh_CN.qm".format(os.path.dirname(__file__)))  # 读取qm语言包
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

        # 绑定本地数据库，辅助使用，尽量直接使用 Model 中的方法
        self.db = get_local_db()

        self.initAction()
        self.initMenuBar()
        self.initMenu()
        self.initToolBar()
        self.initActionConnect()
        self.initMainTabWidget()
        self.initDockWidgets()
        self.statusBar().showMessage(_translate('MainWindow', 'Running'))

        # 在窗口初始化完毕后，根据历史信息对窗口风格和大小进行重置
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

        QtCore.QMetaObject.connectSlotsByName(self)

    @property
    def session(self):
        return self.db.session

    def closeEvent(self, event):
        self.ui_config.save(self)
        event.accept()

    def initAction(self):
        self.action_dict = dict(
            action_new_file_session=QtWidgets.QAction(
                QtGui.QIcon(":/icon/new_32.png"), _translate("MainWindow", "&New Session"), self
            ),
            action_edit_file_session=QtWidgets.QAction(
                QtGui.QIcon(":/icon/edit_32.png"), _translate("MainWindow", "&Edit Session"), self
            ),
            action_del_file_session=QtWidgets.QAction(
                QtGui.QIcon(":/icon/cancel_32.png"), _translate("MainWindow", "&Remove Session"), self
            ),
            action_file_connect=QtWidgets.QAction(
                QtGui.QIcon(":/icon/connect_32.png"), _translate('MainWindow', '&Connect Now')
            ),
            action_file_quit=QtWidgets.QAction(
                QtGui.QIcon(":/icon/quit_32.png"), _translate('MainWindow', '&Quit'), self
            ),
            action_view_normal_style=QtWidgets.QAction(_translate('MainWindow', 'Normal style'), self),
            action_view_dark_style=QtWidgets.QAction(_translate('MainWindow', 'Dark style'), self),
            action_about=QtWidgets.QAction(_translate('MainWindow', 'About'), self),
            action_about_qt=QtWidgets.QAction(_translate('MainWindow', 'About Qt'), self),
        )
        self.action_dict['action_new_file_session'].setStatusTip(_translate('MainWindow', 'New Session'))
        self.action_dict['action_file_connect'].setStatusTip(_translate('MainWindow', 'Connect Now'))
        self.action_dict['action_file_quit'].setStatusTip(_translate('MainWindow', 'Quit Application'))
        self.action_dict['action_about_qt'].setStatusTip(_translate('MainWindow', "Show the Qt library's About box"))
        self.action_dict['action_view_normal_style'].setObjectName('normal_style')
        self.action_dict['action_view_normal_style'].setStatusTip(_translate('MainWindow', 'Switch to normal style'))
        self.action_dict['action_view_dark_style'].setObjectName('dark_style')
        self.action_dict['action_view_dark_style'].setStatusTip(_translate('MainWindow', 'Switch to dark style'))
        self.action_dict['action_edit_file_session'].setEnabled(False)
        self.action_dict['action_del_file_session'].setEnabled(False)

    def initMenuBar(self):
        self.menubar_dict = dict(
            menu_file=self.menuBar().addMenu(_translate('MainWindow', "&File(F)")),
            menu_view=self.menuBar().addMenu(_translate('MainWindow', "&View(V)")),
            menu_help=self.menuBar().addMenu(_translate('MainWindow', "&Help(H)"))
        )

    def initMenu(self):
        file_session_menu = self.menubar_dict['menu_file'].addMenu(
            QtGui.QIcon(":/icon/server_16.png"), _translate('MainWindow', '&Session Manager')
        )
        style_menu = self.menubar_dict['menu_view'].addMenu(_translate('MainWindow', 'Skin style'))
        self.menu_dict = dict(
            menu_file_new_session=file_session_menu.addAction(self.action_dict['action_new_file_session']),
            menu_file_edit_session=file_session_menu.addAction(self.action_dict['action_edit_file_session']),
            menu_file_del_session=file_session_menu.addAction(self.action_dict['action_del_file_session']),
            menu_file_connect=self.menubar_dict['menu_file'].addAction(self.action_dict['action_file_connect']),
            menu_file_quit=self.menubar_dict['menu_file'].addAction(self.action_dict['action_file_quit']),
            menu_view_normal_style=style_menu.addAction(self.action_dict['action_view_normal_style']),
            menu_view_dark_style=style_menu.addAction(self.action_dict['action_view_dark_style']),
            menu_about=self.menubar_dict['menu_help'].addAction(self.action_dict['action_about']),
            menu_about_qt=self.menubar_dict['menu_help'].addAction(self.action_dict['action_about_qt']),
        )

    def initToolBar(self):
        self.setUnifiedTitleAndToolBarOnMac(True)
        file_toolbar = self.addToolBar('File')
        file_toolbar.addAction(self.action_dict['action_new_file_session'])
        file_toolbar.addAction(self.action_dict['action_edit_file_session'])
        file_toolbar.addAction(self.action_dict['action_del_file_session'])
        file_toolbar.addAction(self.action_dict['action_file_connect'])
        file_toolbar.addAction(self.action_dict['action_file_quit'])

    def initActionConnect(self):
        self.action_dict['action_new_file_session'].triggered.connect(self.actionNewSession)
        self.action_dict['action_edit_file_session'].triggered.connect(self.actionEditSession)
        self.action_dict['action_del_file_session'].triggered.connect(self.actionDeleteSession)
        self.action_dict['action_file_connect'].triggered.connect(self.actionConnect)
        self.action_dict['action_file_quit'].triggered.connect(self.close)
        self.action_dict['action_about'].triggered.connect(self.actionAbout)
        self.action_dict['action_about_qt'].triggered.connect(QtWidgets.QApplication.aboutQt)
        self.action_dict['action_view_normal_style'].triggered.connect(self.actionChangStyle)
        self.action_dict['action_view_dark_style'].triggered.connect(self.actionChangStyle)

    def initMainTabWidget(self):
        self.main_tab = QtWidgets.QTabWidget()
        self.setCentralWidget(self.main_tab)

        # 设置为可关闭，并连接信号
        self.main_tab.setTabsClosable(True)
        self.main_tab.tabCloseRequested.connect(self.closeTab)

        # 保存打开过的 tab，防止重复打开
        self.tabs = {}

    def initDockWidgets(self):
        self.server_view_dock = HkuSessionViewWidget(self)
        self.server_view_dock.setFeatures(QtWidgets.QDockWidget.DockWidgetMovable)  # 禁止关闭
        self.server_view_dock.setMinimumWidth(200)
        # 消除 docker window 的顶部按钮
        title_bar = self.server_view_dock.titleBarWidget()
        self.server_view_dock.setTitleBarWidget(QtWidgets.QWidget())
        del title_bar
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.server_view_dock)
        servers = self.db.session.query(SessionModel).order_by(SessionModel.name.asc()).all()
        for server in servers:
            server.running = False  # SQLalchemy query 出来的对象并没有添加非数据库外的属性，此处手工添加保护
            self.server_view_dock.addSession(server)
        self.server_view_dock.open_tab_tigger.connect(self.openTab)

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

    def actionNewSession(self):
        server_session = SessionModel()
        session_dialog = HkuEditSessionDialog(self)
        session_dialog.setWindowTitle(_translate("MainWindow", "New Session"))
        session_dialog.setData(server_session)
        if session_dialog.exec() >= 0:
            session_data = session_dialog.getData()
            session_data.save()
            self.server_view_dock.addSession(session_data)
        session_dialog.destroy()

    def actionEditSession(self):
        item = self.server_view_dock.tree.currentItem()
        server_session = self.db.session.query(SessionModel).filter_by(name=item.text(0)).first() if item else None
        if server_session is None:
            QtWidgets.QMessageBox.about(
                self, _translate("MainWindow", "info"), _translate("MainWindow", "Please select a session to execute")
            )
            return
        edit_session_dialog = HkuEditSessionDialog(self)
        edit_session_dialog.setWindowTitle(_translate("MainWindow", "Edit Session"))
        edit_session_dialog.setData(server_session)
        if edit_session_dialog.exec() >= 0:
            session_data = edit_session_dialog.getData()
            session_data.save()
            self.server_view_dock.modifySession(item, session_data)
        edit_session_dialog.destroy()

    def actionDeleteSession(self):
        item = self.server_view_dock.tree.currentItem()
        data = item.data(0, QtCore.Qt.UserRole) if item is not None else None
        if data is None:
            QtWidgets.QMessageBox.about(
                self, _translate("MainWindow", "info"), _translate("MainWindow", "Please select a session to execute")
            )
            return
        ret = QtWidgets.QMessageBox.question(
            self, _translate("MainWindow", "Confirm removal"),
            _translate("MainWindow", "Confirm to remove the session (%s)?") % item.text(0),
            QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.No
        )
        if ret == QtWidgets.QMessageBox.Yes:
            root_index = self.server_view_dock.tree.indexOfTopLevelItem(item)
            self.server_view_dock.tree.takeTopLevelItem(root_index)
            data.delete()

    def actionConnect(self):
        item = self.server_view_dock.tree.currentItem()
        if item is None:
            logging.error("Can't get currentItem.")
            return
        session = item.data(0, QtCore.Qt.UserRole)
        status, msg = AssisService.getServerStatus(session)
        if not session.running:
            self.server_view_dock.set_gray(item)
            QtWidgets.QMessageBox.warning(
                self, _translate("MainWindow", "info"),
                "{}: {}".format(_translate("MainWindow", "connection failed"), msg)
            )
        else:
            self.server_view_dock.set_default(item)
        self.server_view_dock.tree.viewport().update()

    def closeTab(self, index):
        title = self.main_tab.tabText(index)
        self.main_tab.removeTab(index)
        self.tabs[title] = None

    def openTab(self, session, widget):
        title = "{}({})".format(widget.title(), session.name)
        if title not in self.tabs or self.tabs[title] is None:
            if not session.running:
                QtWidgets.QMessageBox.warning(
                    self, _translate("MainWindow", "info"),
                    _translate("MainWindow", "The server is disconnected. Please connect first!")
                )
            else:
                tab = widget(session, self.main_tab)
                self.main_tab.addTab(tab, title)
                self.tabs[title] = tab
                self.main_tab.setCurrentWidget(tab)
        else:
            self.main_tab.setCurrentWidget(self.tabs[title])


def main_core():
    FORMAT = '%(asctime)-15s [%(levelname)s]: %(message)s [%(name)s::%(funcName)s]'
    logging.basicConfig(format=FORMAT, level=logging.INFO, handlers=[
        logging.StreamHandler(),
    ])

    # 自适应分辨率，防止字体显示不全
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_UseHighDpiPixmaps)
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling)

    app = QtWidgets.QApplication(sys.argv)
    main_win = MyMainWindow()
    main_win.show()
    exit_code = app.exec()
    if exit_code == 888:
        # 应用中使用 qApp.exit(888) 指示重启
        del main_win
        del app  # 必须，否则最终无法正常退出应用
        main_core()
    else:
        sys.exit()


if __name__ == "__main__":
    main_core()
