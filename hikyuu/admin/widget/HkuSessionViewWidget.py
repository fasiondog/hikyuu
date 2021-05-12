# -*- coding: utf-8 -*-

import resource
from PyQt5 import QtWidgets, QtCore, QtGui
from .HkuCheckServerStatusThread import HkuCheckServerStatusThread

from translate import _translate
from data import SessionModel


class HkuSessionViewWidget(QtWidgets.QDockWidget):
    status_changed = QtCore.pyqtSignal()

    # 用户管理信号
    user_manage_trigger = QtCore.pyqtSignal(SessionModel)

    def __init__(self, parent=None):
        super(HkuSessionViewWidget, self).__init__(parent)
        self.setObjectName("HKUServerViewWidget")
        self.tree = QtWidgets.QTreeWidget(self)
        self.setWidget(self.tree)
        #self.tree.header().setVisible(False)
        self.tree.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.icons = [QtGui.QIcon(':/icon/server_16.png')]  # 记录每一级别的图标

        self.tree.setColumnCount(2)
        self.tree.setColumnWidth(0, 150)
        self.tree.setColumnWidth(1, 50)

        self.initContextMenu()
        self.retranslateUi()

        # 创建服务器状态检测线程
        self.status_check_thread = HkuCheckServerStatusThread(self)
        self.status_check_thread.start()

        QtCore.QMetaObject.connectSlotsByName(self)

    def initContextMenu(self):
        self.server_menu = QtWidgets.QMenu(self)
        self.server_menu.addAction(self.parent().action_dict['action_file_connect'])
        self.server_menu.addAction(self.parent().action_dict['action_new_file_session'])
        self.server_menu.addAction(self.parent().action_dict['action_edit_file_session'])
        self.server_menu.addAction(self.parent().action_dict['action_del_file_session'])
        self.tree.customContextMenuRequested.connect(self.on_tree_customContextMenuRequested)
        self.tree.itemClicked.connect(self.on_tree_itemClicked)

    def on_tree_itemClicked(self, item, column):
        data = item.data(0, QtCore.Qt.UserRole)
        if data is not None:
            # 一级节点，使能会话编辑与移除菜单
            self.parent().action_dict['action_edit_file_session'].setEnabled(True)
            self.parent().action_dict['action_del_file_session'].setEnabled(True)
            self.parent().action_dict['action_file_connect'].setEnabled(True)
            return

        # 屏蔽会话编辑与移除菜单
        self.parent().action_dict['action_edit_file_session'].setEnabled(False)
        self.parent().action_dict['action_del_file_session'].setEnabled(False)
        self.parent().action_dict['action_file_connect'].setEnabled(False)

        session = item.parent().data(0, QtCore.Qt.UserRole)
        if session.name == "admin":
            self.user_manage_trigger.emit(session)

    def on_tree_customContextMenuRequested(self, pos):
        item = self.tree.itemAt(pos)
        if item is not None:
            self.server_menu.exec(QtGui.QCursor.pos())

    def addSession(self, session):
        item = QtWidgets.QTreeWidgetItem(self.tree)
        item.setText(0, session.name)
        item.setIcon(0, self.icons[0])
        item.setData(0, QtCore.Qt.UserRole, session)
        if (session.name == "admin"):
            subitem = QtWidgets.QTreeWidgetItem(item)
            subitem.setText(0, _translate("HkuSessionViewWidget", "users"))
        else:
            account = _translate("HkuSessionViewWidget", "account")
            funds = _translate("HkuSessionViewWidget", "funds")
            positons = _translate("HkuSessionViewWidget", "positions")
            orders = _translate("HkuSessionViewWidget", "orders")
            fills = _translate("HkuSessionViewWidget", "fills")
            names = [account, funds, positons, orders, fills]
            for name in names:
                subitem = QtWidgets.QTreeWidgetItem(item)
                subitem.setText(0, name)

    def modifySession(self, item, session):
        item.setText(0, session.name)
        item.setData(0, QtCore.Qt.UserRole, session)

    def retranslateUi(self):
        self.tree.headerItem().setText(0, _translate("HkuSessionViewWidget", "name"))
        self.tree.headerItem().setText(1, _translate("HkuSessionViewWidget", "status"))
        __sortingEnabled = self.tree.isSortingEnabled()
        #self.tree.setSortingEnabled(False)
        #self.tree.topLevelItem(0).setText(0, _translate("HkuSessionViewWidget", "local"))
        #self.tree.topLevelItem(0).child(0).setText(0, _translate("HkuSessionViewWidget", "account"))
        #self.tree.topLevelItem(1).setText(0, _translate("HkuSessionViewWidget", "other"))
        self.tree.setSortingEnabled(__sortingEnabled)
