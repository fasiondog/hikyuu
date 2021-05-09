# -*- coding: utf-8 -*-

import resource
import ServerApi

import sqlalchemy
from PyQt5 import QtWidgets, QtCore, QtGui
from .Ui_HkuEditSessionDialog import Ui_HkuEditSessionDialog
from .HkuWaitingDialog import HkuWaitingDialog

_translate = QtCore.QCoreApplication.translate


class HkuEditSessionDialog(QtWidgets.QDialog, Ui_HkuEditSessionDialog):
    def __init__(self, parent):
        super(HkuEditSessionDialog, self).__init__(parent)
        self.setWindowIcon(QtGui.QIcon(":/icon/server_16.png"))
        self.setupUi(self)
        self.session_model = None

    @property
    def name(self):
        return self.name_lineEdit.text().strip()

    @property
    def host(self):
        return self.host_lineEdit.text().strip()

    @property
    def port(self):
        return self.port_spinBox.value()

    @property
    def user(self):
        return self.user_lineEdit.text().strip()

    @property
    def password(self):
        return self.password_lineEdit.text().strip()

    @property
    def remark(self):
        return self.remark_textEdit.toPlainText()

    def setData(self, session_model):
        self.session_model = session_model
        self.name_lineEdit.setText(session_model.name)
        self.host_lineEdit.setText(session_model.host)
        self.port_spinBox.setValue(session_model.port)
        self.user_lineEdit.setText(session_model.user)
        self.password_lineEdit.setText(session_model.password)
        self.remark_textEdit.setText(session_model.remark)

    def getData(self):
        self.session_model.name = self.name
        self.session_model.host = self.host
        self.session_model.port = self.port
        self.session_model.user = self.user
        self.session_model.password = self.password
        self.session_model.remark = self.remark
        return self.session_model

    def accept(self):
        name = self.name_lineEdit.text().strip()
        if len(name) == 0:
            QtWidgets.QMessageBox.warning(
                self, _translate("HkuEditSessionDialog", "Error"),
                _translate("HkuEditSessionDialog", "The name cannot be empty!")
            )
            self.name_lineEdit.setFocus()
            return
        self.session_model.name = name
        self.session_model.host = self.host
        self.session_model.port = self.port
        self.session_model.user = self.user
        self.session_model.password = self.password_lineEdit.text()
        self.session_model.remark = self.remark_textEdit.toPlainText()
        session = self.parent().session
        try:
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
        try:
            r = ServerApi.login("{}:{}".format(self.host, self.port), self.user, self.password)
            if r["result"]:
                QtWidgets.QMessageBox.about(
                    self, _translate("HkuEditSessionDialog", "success"),
                    _translate("HkuEditSessionDialog", "Connect successfully!")
                )
            else:
                QtWidgets.QMessageBox.about(self, _translate("HkuEditSessionDialog", "Failed"), ret["errmsg"])
        except Exception as e:
            QtWidgets.QMessageBox.about(
                self, _translate("HkuEditSessionDialog", "Failed"),
                _translate("HkuEditSessionDialog", "Failed connect! Please check the host/ip and port\n%s") % e
            )

    @QtCore.pyqtSlot()
    def on_remark_textEdit_textChanged(self):
        text = self.remark_textEdit.toPlainText()
        max_length = 256
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
