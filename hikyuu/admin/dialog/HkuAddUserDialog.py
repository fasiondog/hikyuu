# -*- coding: utf-8 -*-

import resource
from PyQt5 import QtWidgets, QtGui
from .Ui_HkuAddUserDialog import Ui_AddUserDialog
from data import SessionModel
from translate import _translate


class HkuAddUserDialog(QtWidgets.QDialog, Ui_AddUserDialog):
    def __init__(self, session: SessionModel, parent, name=None, password=None):
        super(HkuAddUserDialog, self).__init__(parent)
        self.session = session
        self.setupUi(self)
        self.setWindowIcon(QtGui.QIcon(":/logo/logo_16.png"))
        if name is not None:
            self.name_lineEdit.setText(name)
        if password is not None:
            self.password_lineEdit.setText(password)

    @property
    def name(self):
        return self.name_lineEdit.text()

    @property
    def password(self):
        return self.password_lineEdit.text()

    def accept(self):
        if not self.name:
            QtWidgets.QMessageBox.warning(self, _translate("UserManage", "error"), _translate("UserManage", "The user name cannot be empty."))
            return
        self.done(1)

    def reject(self):
        self.done(-1)


