# -*- coding: utf-8 -*-

import logging
from PyQt5 import QtWidgets, QtCore

from data import SessionModel
from dialog import HkuAddUserDialog
from service import UserService
from translate import _translate

from .Ui_HkuXueqiuAccountWidget import Ui_HkuXueqiuAccountForm
from .RestDataTableModel import RestDataTableModel


class HkuXueqiuAccountWidget(QtWidgets.QWidget, Ui_HkuXueqiuAccountForm):
    @classmethod
    def title(cls):
        return _translate("XueqiuAccount", "Xueqiu Account Setting")

    def __init__(self, session: SessionModel, parent):
        super(HkuXueqiuAccountWidget, self).__init__(parent)
        self.session = session
        self.setupUi(self)
