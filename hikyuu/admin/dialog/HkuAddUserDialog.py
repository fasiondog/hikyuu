# -*- coding: utf-8 -*-

from PyQt5 import QtWidgets
from .Ui_HkuAddUserDialog import Ui_AddUserDialog


class HkuAddUserDialog(QtWidgets.QDialog, Ui_AddUserDialog):
    def __init__(self, parent=None):
        super(HkuAddUserDialog, self).__init__(parent)
        self.setupUi(self)

    def accept(self):
        self.done(1)

    def reject(self):
        self.done(-1)


