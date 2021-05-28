# -*- coding: utf-8 -*-

import resource
from PyQt5 import QtWidgets, QtGui
from data import SessionModel
from translate import _translate
from .Ui_HkuChangePasswordDialog import Ui_ChangePasswordDialog

class HkuChangePasswordDialog(QtWidgets.QDialog, Ui_ChangePasswordDialog):
    def __init__(self, session: SessionModel, parent):
        super(HkuChangePasswordDialog, self).__init__(parent)
        self.session = session
        self.setupUi(self)
        self.setWindowIcon(QtGui.QIcon(":/logo/logo_16.png"))

    @property
    def old_password(self):
        return self.old_password_lineEdit.text().strip()

    @property
    def new_password(self):
        return self.new_password_lineEdit.text().strip()

    @property
    def confirm_password(self):
        return self.confirm_password_lineEdit.text().strip()

    def reject(self):
        self.done(-1)

    def accept(self) -> None:
        if not self.new_password:
            QtWidgets.QMessageBox.warning(self, _translate("PersonInfo", "error"),
                                          _translate("PersonInfo", "Please input new password"))
            return
        if self.new_password == self.old_password:
            QtWidgets.QMessageBox.warning(self, _translate("PersonInfo", "error"),
                                          _translate("PersonInfo", "The old and new passwords are the same"))
            return
        if self.new_password != self.confirm_password:
            QtWidgets.QMessageBox.warning(self, _translate("PersonInfo", "error"),
                                          _translate("PersonInfo", "The two passwords are different"))
            return
        self.done(1)
