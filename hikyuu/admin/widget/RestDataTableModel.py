# -*- coding: utf-8 -*-

import typing
from PyQt5 import QtCore


class RestDataTableModel(QtCore.QAbstractTableModel):
    def __init__(self, head: list, head_tr: list, data: list = [], parent=None):
        """ Restful接口返回表数据模型

        :param head: 表头字段名称列表（按顺序）
        :param head_tr: 表头字段名称国际化翻译列表（与head对应）
        :param data: restful接口返回的 data 段，一个有 dict 组成的列表
        :param parent: Qt上级对象
        """
        super(RestDataTableModel, self).__init__(parent)
        self.rest_head = head
        self.rest_head_tr = head_tr
        self.rest_data = data

    def rowCount(self, parent: QtCore.QModelIndex):
        return 0 if parent.isValid() else len(self.rest_data)

    def columnCount(self, parent: QtCore.QModelIndex):
        return 0 if parent.isValid() else len(self.rest_head)

    def data(self, index: QtCore.QModelIndex, role: int):
        if not index.isValid() or index.row() >= len(self.rest_data) or index.row() < 0:
            return None
        if role == QtCore.Qt.DisplayRole:
            record = self.rest_data[index.row()]
            return record[self.rest_head[index.column()]]
        return None

    def headerData(self, section: int, orientation: QtCore.Qt.Orientation, role: int):
        if role != QtCore.Qt.DisplayRole:
            return None
        if orientation == QtCore.Qt.Horizontal:
            return self.rest_head_tr[section]
        return None

    def insertRows(self, row: int, count: int, parent: QtCore.QModelIndex) -> bool:
        self.beginInsertRows(QtCore.QModelIndex(), row, row + count - 1)
        for i in range(count):
            self.rest_data.insert(i, dict([(k, None) for k in self.rest_head]))
        self.endInsertRows()
        return True

    def removeRows(self, row: int, count: int, parent: QtCore.QModelIndex = ...) -> bool:
        self.beginRemoveRows(QtCore.QModelIndex(), row, row + count - 1)
        for i in range(count):
            self.rest_data.pop(row + count - 1 - i)
        self.endRemoveRows()
        return True

    def setData(self, index: QtCore.QModelIndex, value: typing.Any, role: int = ...) -> bool:
        if index.isValid() and role == QtCore.Qt.EditRole:
            row = index.row()
            self.rest_data[row][self.rest_head[index.column()]] = value
            self.dataChanged.emit(index, index, [QtCore.Qt.DisplayRole, QtCore.Qt.EditRole])
            return True
        return False

    def flags(self, index: QtCore.QModelIndex) -> QtCore.Qt.ItemFlags:
        if not index.isValid():
            return QtCore.Qt.ItemIsEnabled
        return QtCore.Qt.ItemIsEditable

    def getAllData(self):
        return self.rest_data
