# -*- coding: utf-8 -*-

from PyQt5 import QtWidgets, QtCore
from .Ui_HkuUserManagerWidget import Ui_UserManagerForm

from data import SessionModel
from dialog import HkuAddUserDialog


class HkuUserManagerWidget(QtWidgets.QWidget, Ui_UserManagerForm):
    def __init__(
        self,
        session: SessionModel,
        parent=None,
    ):
        super(HkuUserManagerWidget, self).__init__(parent)
        self.session = session
        self.setupUi(self)

    @QtCore.pyqtSlot()
    def on_add_user_pushButton_clicked(self):
        add_dialog = HkuAddUserDialog()
        if add_dialog.exec() > 0:
            print("aaaaaaa")

