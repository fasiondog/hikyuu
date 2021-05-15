# -*- coding: utf-8 -*-

from PyQt5 import QtWidgets, QtCore

from data import SessionModel
from dialog import HkuAddUserDialog
from translate import _translate

from .Ui_HkuUserManagerWidget import Ui_UserManagerForm
from .RestDataTableModel import RestDataTableModel


class HkuUserManagerWidget(QtWidgets.QWidget, Ui_UserManagerForm):
    def __init__(
        self,
        session: SessionModel,
        data: RestDataTableModel,
        parent=None,
    ):
        super(HkuUserManagerWidget, self).__init__(parent)
        self.session = session
        self.setupUi(self)
        self.rest_data_model = data
        self.users_tableView.setModel(self.rest_data_model)
        #self.users_tableView.horizontalHeader().setSectionResizeMode(0, QtWidgets.QHeaderView.ResizeToContents)
        #self.users_tableView.horizontalHeader().setSectionResizeMode(0, QtWidgets.QHeaderView.Interactive)

    @QtCore.pyqtSlot()
    def on_add_user_pushButton_clicked(self):
        add_dialog = HkuAddUserDialog()
        if add_dialog.exec() > 0:
            print("aaaaaaa")
