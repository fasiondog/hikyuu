# -*- coding: utf-8 -*-

from PyQt5 import QtWidgets, QtCore

from data import SessionModel
from service import UserService
from translate import _translate
from .Ui_HkuPersonInfoWidget import Ui_PersonInfoForm


class HkuPersonInfoWidget(QtWidgets.QWidget, Ui_PersonInfoForm):
    @classmethod
    def title(cls):
        return _translate("PersonInfo", "Person Info")

    def __init__(self, session: SessionModel, parent):
        super(HkuPersonInfoWidget, self).__init__(parent)
        self.session = session
        self.setupUi(self)

    @QtCore.pyqtSlot()
    def on_change_password_pushButton_clicked(self):
        print("clclcl")
