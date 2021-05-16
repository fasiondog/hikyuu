# -*- coding: utf-8 -*-

import resource
from PyQt5 import QtWidgets, QtGui
from .Ui_HkuAddUserDialog import Ui_AddUserDialog
from data import SessionModel
from service import UserService
from translate import _translate


class HkuAddUserDialog(QtWidgets.QDialog, Ui_AddUserDialog):
    def __init__(self, session: SessionModel, parent=None):
        super(HkuAddUserDialog, self).__init__(parent)
        self.session = session
        self.setupUi(self)
        self.setWindowIcon(QtGui.QIcon(":/logo/logo_16.png"))

    @property
    def name(self):
        return self.name_lineEdit.text()

    @property
    def password(self):
        return self.password_lineEdit.text()

    def accept(self):
        if not self.name:
            QtWidgets.QMessageBox.warning(self, "error", _translate("UserManage", "The user name cannot be empty."))
            return

        r= UserService.add_user(self.session, self.name, self.password)
        print(r)
        self.done(1)

    def reject(self):
        self.done(-1)


