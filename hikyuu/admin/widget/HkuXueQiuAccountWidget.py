# -*- coding: utf-8 -*-

import logging
from PyQt5 import QtWidgets, QtCore

from data import SessionModel
from dialog import HkuEditXueqiuAccountDialog
from service import TradeService
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
        self.head = ['td_id', 'name', 'portfolio_code', 'portfolio_market', 'cookies']
        self.head_tr = [
            _translate("trade", "td_id"),
            _translate("trade", "name"),
            _translate("trade", "portfolio_code"),
            _translate("trade", "portfolio_market"),
            _translate("trade", "cookies"),
        ]
        self.edit_pushButton.setEnabled(False)
        self.remove_pushButton.setEnabled(False)
        self.on_refresh_pushButton_clicked()

    @QtCore.pyqtSlot()
    def on_refresh_pushButton_clicked(self):
        try:
            data = RestDataTableModel(self.head, self.head_tr, TradeService.query_xq_accounts(self.session))
            self.rest_data_model = data
            self.tableView.setModel(self.rest_data_model)
        except Exception as e:
            logging.error(e)
            #self.add_user_pushButton.setEnabled(False)
            QtWidgets.QMessageBox.warning(self, _translate("trade", "error"), "{}: {}".format(e.__class__.__name__, e))

    @QtCore.pyqtSlot()
    def on_add_pushButton_clicked(self):
        dialog = HkuEditXueqiuAccountDialog(self.session, self)
        if dialog.exec() > 0:
            try:
                r = TradeService.add_xq_account(
                    self.session, dialog.name, dialog.cookies, dialog.portfolio_code, dialog.portfolio_market
                )
                info = (
                    r["td_id"],
                    dialog.name,
                    dialog.portfolio_code,
                    dialog.portfolio_market,
                    dialog.cookies,
                )
                self.rest_data_model.insertRows(0, 1, QtCore.QModelIndex())
                for i in range(len(info)):
                    index = self.rest_data_model.index(0, i, QtCore.QModelIndex())
                    self.rest_data_model.setData(index, info[i], QtCore.Qt.EditRole)
            except Exception as e:
                QtWidgets.QMessageBox.warning(self, _translate("trade", "error"), str(e))

    @QtCore.pyqtSlot()
    def on_remove_pushButton_clicked(self):
        indexes = self.tableView.selectedIndexes()
        for index in indexes:
            row = self.rest_data_model.row(index)
            if row is None:
                continue
            try:
                td_id, name = row["td_id"], row["name"]
                r = QtWidgets.QMessageBox.information(
                    self, _translate("trade", "Confirm"),
                    _translate("trade", "Are you sure to remove the trade account ({})?").format(name),
                    QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.No
                )
                if r == QtWidgets.QMessageBox.No:
                    return
                r = TradeService.remove_xq_account(self.session, td_id)
            except Exception as e:
                QtWidgets.QMessageBox.warning(self, _translate("trade", "error"), str(e))
                return
            self.rest_data_model.removeRows(index.row(), 1, QtCore.QModelIndex())

    @QtCore.pyqtSlot()
    def on_edit_pushButton_clicked(self):
        indexes = self.tableView.selectedIndexes()
        for index in indexes:
            row = self.rest_data_model.row(index)
            if row is None:
                continue
            try:
                dialog = HkuEditXueqiuAccountDialog(self.session, self, row)
                if dialog.exec() > 0:
                    info = dict(
                        td_id=int(dialog.td_id),
                        name=dialog.name,
                        cookies=dialog.cookies,
                        portfolio_code=dialog.portfolio_code,
                        portfolio_market=dialog.portfolio_market
                    )
                    TradeService.modify_xq_account(self.session, info)
                    row["name"] = info["name"]
                    row["cookies"] = info["cookies"]
                    row["portfolio_code"] = info["portfolio_code"]
                    row["portfolio_market"] = info["portfolio_market"]
            except Exception as e:
                QtWidgets.QMessageBox.warning(self, _translate("trade", "error"), str(e))
                return

    def on_tableView_clicked(self, index: QtCore.QModelIndex):
        if not self.remove_pushButton.isEnabled():
            self.remove_pushButton.setEnabled(True)
        if not self.edit_pushButton.isEnabled():
            self.edit_pushButton.setEnabled(True)

    def keyPressEvent(self, event):
        # ctrl-c 负责选中的单元格
        if event.modifiers() == QtCore.Qt.ControlModifier and event.key() == QtCore.Qt.Key_C:
            indexes = self.tableView.selectedIndexes()
            for index in indexes:
                QtWidgets.qApp.clipboard().setText(str(self.rest_data_model.data(index, QtCore.Qt.DisplayRole)))
