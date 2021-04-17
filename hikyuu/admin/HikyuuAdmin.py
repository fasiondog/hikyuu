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

import sys, logging, os

from PyQt5.QtWidgets import QApplication, QMainWindow, QAction
from PyQt5.QtGui import QIcon
import resource  # 引入资源文件

from PyQt5.QtCore import QCoreApplication, QTranslator, QLocale
translate = QCoreApplication.translate

from UiConfig import UiConfig


class MyMainWindow(QMainWindow):
    def __init__(self, capture_output=True, use_dark_style=False):
        super().__init__()
        loc = QLocale()
        if loc.language() == QLocale.Chinese:
            self.trans = QTranslator()  # 实例翻译者
            self.trans.load("language/zh_CN.qm")  # 读取qm语言包
            _app = QApplication.instance()  # 应用实例
            _app.installTranslator(self.trans)  # 将翻译者安装到实例中

        self.ui_config = UiConfig()
        self.setObjectName("HikyuuAdminMainWindow")
        self.setWindowTitle(translate("MyMainWindow", "Hikyuu Admin"))
        self.setWindowIcon(QIcon(':/ico/hikyuu.ico'))
        if self.ui_config.getboolean('main_window', 'maximized', fallback=False):
            self.showMaximized()
        else:
            self.resize(
                self.ui_config.getint('main_window', 'width', fallback=800),
                self.ui_config.getint('main_window', 'height', fallback=600)
            )

        self.initAction()
        self.initMenuBar()
        self.initMenu()
        self.initActionConnect()

    def closeEvent(self, event):
        self.ui_config.save(self)
        event.accept()

    def initAction(self):
        self.action_dict = dict(action_quit=QAction(translate('MyMainWindow', '&Quit'), self))

    def initMenuBar(self):
        self.menubar_dict = dict(
            menu_file=self.menuBar().addMenu(translate("MyMainWindow", "&File(F)")),
            menu_about=self.menuBar().addMenu(translate("MyMainWindow", "&Help(H)"))
        )

    def initMenu(self):
        self.menu_dict = dict(menu_quit=self.menubar_dict['menu_file'].addAction(self.action_dict['action_quit']))

    def initActionConnect(self):
        self.action_dict['action_quit'].triggered.connect(self.close)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--dark', action='store_true', help='Use dark style')
    args = parser.parse_args()

    app = QApplication(sys.argv)
    use_dark_style = False  # 使用暗黑主题
    if use_dark_style:
        import qdarkstyle
        app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
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
