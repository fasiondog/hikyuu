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
        self.remark_textEdit.textChanged.connect(self.limitRemarkMaxLength)
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

    def limitRemarkMaxLength(self):
        text = self.remark_textEdit.toPlainText()
        print(len(text))
        max_length = 3
        if len(text) > max_length:
            self.remark_textEdit.setText(text[:max_length])
            cursor = self.remark_textEdit.textCursor()
            cursor.movePosition(QtGui.QTextCursor.End)
            self.remark_textEdit.setTextCursor(cursor)


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    win = HkuEditSessionDialog()
    win.show()
    sys.exit(win.exec_())
