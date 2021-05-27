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
    @classmethod
    def title(cls):
        # 用类属性无法获取翻译字符串，只能用类方法或静态方法
        return _translate("UserManage", "User Manage")

    def __init__(
        self,
        session: SessionModel,
        parent=None,
    ):
        super(HkuUserManagerWidget, self).__init__(parent)
        self.session = session
        self.head = ['userid', 'name', 'start_time']
        self.head_tr = [
            _translate("UserManage", "userid"),
            _translate("UserManage", "name"),
            _translate("UserManage", "start_time")
        ]
        self.setupUi(self)
        self.remove_pushButton.setEnabled(False)
        self.reset_password_pushButton.setEnabled(False)
        self.on_refresh_pushButton_clicked()

    def on_users_tableView_clicked(self, index: QtCore.QModelIndex):
        if not self.remove_pushButton.isEnabled():
            self.remove_pushButton.setEnabled(True)
        if not self.reset_password_pushButton.isEnabled():
            self.reset_password_pushButton.setEnabled(True)

    def keyPressEvent(self, event):
        # ctrl-c 负责选中的单元格
        if event.modifiers() == QtCore.Qt.ControlModifier and event.key() == QtCore.Qt.Key_C:
            indexes = self.users_tableView.selectedIndexes()
            for index in indexes:
                QtWidgets.qApp.clipboard().setText(str(self.rest_data_model.data(index, QtCore.Qt.DisplayRole)))

    @QtCore.pyqtSlot()
    def on_reset_pushButton_clicked(self):
        # 重置查询条件
        self.userid_lineEdit.setText(None)
        self.name_lineEdit.setText(None)

    @QtCore.pyqtSlot()
    def on_query_pushButton_clicked(self):
        userid = self.userid_lineEdit.text()
        if userid:
            try:
                userid = int(self.userid_lineEdit.text())
            except:
                self.rest_data_model = RestDataTableModel(self.head, self.head_tr, [])
                self.users_tableView.setModel(self.rest_data_model)
                return
        name = self.name_lineEdit.text()
        try:
            data = RestDataTableModel(self.head, self.head_tr, UserService.query_users(self.session, userid, name))
            self.rest_data_model = data
            self.users_tableView.setModel(self.rest_data_model)
        except Exception as e:
            logging.error(e)
            self.add_user_pushButton.setEnabled(False)
            QtWidgets.QMessageBox.warning(
                self, _translate("MainWindow", "error"), "{}: {}".format(e.__class__.__name__, e)
            )

    @QtCore.pyqtSlot()
    def on_refresh_pushButton_clicked(self):
        try:
            data = RestDataTableModel(self.head, self.head_tr, UserService.query_users(self.session))
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
        indexes = self.users_tableView.selectedIndexes()
        for index in indexes:
            row = self.rest_data_model.row(index)
            if row is None:
                continue
            try:
                userid, name = row["userid"], row["name"]
                if name == "admin":
                    raise Exception(_translate("UserManage", "The admin count can't to be deleted"))
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
        indexes = self.users_tableView.selectedIndexes()
        for index in indexes:
            row = self.rest_data_model.row(index)
            if row is None:
                continue
            try:
                userid, name = row["userid"], row["name"]
                r = UserService.reset_password(self.session, userid)
                QtWidgets.QMessageBox.about(
                    self, _translate("UserManage", "info"),
                    _translate("UserManage", "The password of user ({}) had be reset to 123456").format(name)
                )
            except Exception as e:
                QtWidgets.QMessageBox.warning(self, _translate("UserManage", "error"), str(e))
                return
