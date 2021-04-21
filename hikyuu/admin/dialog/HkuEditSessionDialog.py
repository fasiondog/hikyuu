# -*- coding: utf-8 -*-

import sys
sys.path.append("..")
import resource

from PyQt5 import QtWidgets, QtCore, QtGui
from .Ui_HkuEditSessionDialog import Ui_HkuEditSessionDialog


class HkuEditSessionDialog(QtWidgets.QDialog, Ui_HkuEditSessionDialog):
    def __init__(self, parent=None):
        super(HkuEditSessionDialog, self).__init__(parent)
        self.setWindowIcon(QtGui.QIcon(":/icon/server.png"))
        self.setupUi(self)
        self.count = 1

    def accept(self):
        print(self.count)
        self.count += 1
        self.close()

    def rejected(self):
        pass

    @QtCore.pyqtSlot()
    def on_test_pushButton_clicked(self):
        print("jjjj")


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    win = HkuEditSessionDialog()
    win.show()
    sys.exit(win.exec_())
