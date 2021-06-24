# -*- coding: utf-8 -*-

import resource
from PyQt5 import QtWidgets, QtGui
from .Ui_HkuEditXueqiuAccountDialog import Ui_XueqiuAccountDialog
from data import SessionModel
from translate import _translate

class HkuEditXueqiuAccountDialog(QtWidgets.QDialog, Ui_XueqiuAccountDialog):
    def __init__(self, session: SessionModel, parent, account_info=None):
        super(HkuEditXueqiuAccountDialog, self).__init__(parent)
        self.session = session
        self.setupUi(self)
        self.setWindowIcon(QtGui.QIcon(":/logo/logo_16.png"))
        if account_info is not None:
            self.td_id_lineEdit.setText(str(account_info["td_id"]))
            self.name_lineEdit.setText(account_info["name"])
            self.portfolio_code_lineEdit.setText(account_info["portfolio_code"])
            self.portfolio_market_lineEdit.setText(account_info["portfolio_market"])
            self.cookies_textEdit.setText(account_info["cookies"])

    @property
    def td_id(self):
        return self.td_id_lineEdit.text()

    @property
    def name(self):
        return self.name_lineEdit.text()

    @property
    def cookies(self):
        return self.cookies_textEdit.toPlainText()

    @property
    def portfolio_code(self):
        return self.portfolio_code_lineEdit.text()

    @property
    def portfolio_market(self):
        return self.portfolio_market_lineEdit.text()

    def accept(self):
        if not self.name:
            QtWidgets.QMessageBox.warning(
                self, _translate("trade", "error"), _translate("trade", "The account name cannot be empty.")
            )
            return
        self.done(1)

    def reject(self):
        self.done(-1)
