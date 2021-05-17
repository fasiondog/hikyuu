# -*- coding: utf-8 -*-

import logging
from PyQt5 import QtWidgets, QtCore

from data import SessionModel
from dialog import HkuAddUserDialog
from service import UserService
from translate import _translate

from .Ui_HkuUserManagerWidget import Ui_UserManagerForm
from .RestDataTableModel import RestDataTableModel


class HkuUserManagerWidget(QtWidgets.QWidget, Ui_UserManagerForm):
    def __init__(
        self,
        session: SessionModel,
        parent=None,
    ):
        super(HkuUserManagerWidget, self).__init__(parent)
        self.session = session
        self.setupUi(self)
        self.remove_pushButton.setEnabled(False)
        self.reset_password_pushButton.setEnabled(False)
        self.on_refresh_pushButton_clicked()

    def on_users_tableView_clicked(self, index: QtCore.QModelIndex):
        if not self.remove_pushButton.isEnabled():
            self.remove_pushButton.setEnabled(True)
        if not self.reset_password_pushButton.isEnabled():
            self.reset_password_pushButton.setEnabled(True)

    @QtCore.pyqtSlot()
    def on_refresh_pushButton_clicked(self):
        try:
            data = RestDataTableModel(
                ['userid', 'name', 'start_time'], [
                    _translate("UserManage", "userid"),
                    _translate("UserManage", "name"),
                    _translate("UserManage", "start_time")
                ], UserService.query_users(self.session)
            )
            self.rest_data_model = data
            self.users_tableView.setModel(self.rest_data_model)
            # self.users_tableView.horizontalHeader().setSectionResizeMode(0, QtWidgets.QHeaderView.ResizeToContents)
            # self.users_tableView.horizontalHeader().setSectionResizeMode(0, QtWidgets.QHeaderView.Interactive)
        except Exception as e:
            logging.error(e)
            self.add_user_pushButton.setEnabled(False)
            QtWidgets.QMessageBox.warning(
                self, _translate("MainWindow", "error"), "{}: {}".format(e.__class__.__name__, e)
            )

    @QtCore.pyqtSlot()
    def on_add_user_pushButton_clicked(self, name=None, password=None):
        add_dialog = HkuAddUserDialog(self.session, self, name, password)
        if add_dialog.exec() > 0:
            try:
                r = UserService.add_user(self.session, add_dialog.name, add_dialog.password)
                user_info = (r["userid"], r["name"], r["start_time"])
                self.rest_data_model.insertRows(0, 1, QtCore.QModelIndex())
                index = self.rest_data_model.index(0, 0, QtCore.QModelIndex())
                self.rest_data_model.setData(index, user_info[0], QtCore.Qt.EditRole)
                index = self.rest_data_model.index(0, 1, QtCore.QModelIndex())
                self.rest_data_model.setData(index, user_info[1], QtCore.Qt.EditRole)
                index = self.rest_data_model.index(0, 2, QtCore.QModelIndex())
                self.rest_data_model.setData(index, user_info[2], QtCore.Qt.EditRole)
            except Exception as e:
                QtWidgets.QMessageBox.warning(self, _translate("UserManage", "error"), str(e))
                self.on_add_user_pushButton_clicked(add_dialog.name, add_dialog.password)

    @QtCore.pyqtSlot()
    def on_remove_pushButton_clicked(self):
        selected = self.users_tableView.selectionModel()
        if selected:
            indexes = selected.selectedRows()
            for index in indexes:
                try:
                    userid_index = self.rest_data_model.index(index.row(), 0, QtCore.QModelIndex())
                    userid = self.rest_data_model.data(userid_index, QtCore.Qt.DisplayRole)
                    name_index = self.rest_data_model.index(index.row(), 1, QtCore.QModelIndex())
                    name = self.rest_data_model.data(name_index, QtCore.Qt.DisplayRole)
                    r = QtWidgets.QMessageBox.information(
                        self, _translate("UserManage", "Confirm"),
                        _translate("UserManage", "Are you sure to remove the user ({})?").format(name),
                        QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.No
                    )
                    if r == QtWidgets.QMessageBox.No:
                        return
                    r = UserService.remove_user(self.session, userid)
                except Exception as e:
                    QtWidgets.QMessageBox.warning(self, _translate("UserManage", "error"), str(e))
                    return
                self.rest_data_model.removeRows(index.row(), 1, QtCore.QModelIndex())

    @QtCore.pyqtSlot()
    def on_reset_password_pushButton_clicked(self):
        selected = self.users_tableView.selectionModel()
        if selected:
            indexes = selected.selectedRows()
            for index in indexes:
                try:
                    userid_index = self.rest_data_model.index(index.row(), 0, QtCore.QModelIndex())
                    userid = self.rest_data_model.data(userid_index, QtCore.Qt.DisplayRole)
                    name_index = self.rest_data_model.index(index.row(), 1, QtCore.QModelIndex())
                    name = self.rest_data_model.data(name_index, QtCore.Qt.DisplayRole)
                    r = UserService.reset_password(self.session, userid)
                    QtWidgets.QMessageBox.about(
                        self, _translate("UserManage", "info"),
                        _translate("UserManage", "The password of user ({}) had be reset to 123456").format(name)
                    )
                except Exception as e:
                    QtWidgets.QMessageBox.warning(self, _translate("UserManage", "error"), str(e))
                    return
