# -*- coding: utf-8 -*-

import sys
sys.path.append("..")
import resource

import sqlalchemy
from PyQt5 import QtWidgets, QtCore, QtGui
from .Ui_HkuEditSessionDialog import Ui_HkuEditSessionDialog


class HkuEditSessionDialog(QtWidgets.QDialog, Ui_HkuEditSessionDialog):
    def __init__(self, parent):
        super(HkuEditSessionDialog, self).__init__(parent)
        self.setWindowIcon(QtGui.QIcon(":/icon/server.png"))
        self.setupUi(self)
        self.session_model = None

    def setData(self, session_model):
        self.session_model = session_model
        self.name_lineEdit.setText(session_model.name)
        self.host_lineEdit.setText(session_model.host)
        self.port_spinBox.setValue(session_model.port)
        self.user_lineEdit.setText(session_model.user)
        self.password_lineEdit.setText(session_model.password)
        self.remark_textEdit.setText(session_model.remark)

    def accept(self):
        _translate = QtCore.QCoreApplication.translate
        name = self.name_lineEdit.text().strip()
        if len(name) == 0:
            QtWidgets.QMessageBox.warning(
                self, _translate("HkuEditSessionDialog", "Error"),
                _translate("HkuEditSessionDialog", "The name cannot be empty!")
            )
            self.name_lineEdit.setFocus()
            return
        self.session_model.name = name
        self.session_model.host = self.host_lineEdit.text().strip()
        self.session_model.port = self.port_spinBox.value()
        self.session_model.password = self.password_lineEdit.text()
        self.session_model.remark = self.remark_textEdit.toPlainText()
        session = self.parent().session
        try:
            print(self.parent())
            session.add(self.session_model)
            session.commit()
        except sqlalchemy.exc.IntegrityError as e:
            session.rollback()
            QtWidgets.QMessageBox.warning(
                self, _translate("HkuEditSessionDialog", "Error"),
                _translate("HkuEditSessionDialog", "A duplicate name already exists!")
            )
            return
        except Exception as e:
            session.rollback()
            QtWidgets.QMessageBox.warning(self, _translate("HkuEditSessionDialog", "Error"), str(e))
            return
        self.done(0)

    def reject(self):
        self.done(-1)

    @QtCore.pyqtSlot()
    def on_test_pushButton_clicked(self):
        print("jjjj")

    @QtCore.pyqtSlot()
    def on_remark_textEdit_textChanged(self):
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
