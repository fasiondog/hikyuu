# -*- coding: utf-8 -*-

import resource
from PyQt5 import QtWidgets, QtCore, QtGui
from .HkuCheckServerStatusThread import HkuCheckServerStatusThread

from translate import _translate
from data import SessionModel
from .HkuPersonInfoWidget import HkuPersonInfoWidget
from .HkuUserManagerWidget import HkuUserManagerWidget
from .HkuXueQiuAccountWidget import HkuXueqiuAccountWidget


class HkuSessionViewWidget(QtWidgets.QDockWidget):
    status_changed = QtCore.pyqtSignal()

    # 打开功能 tab 信号， 参数 object 为指定的 Widget
    open_tab_tigger = QtCore.pyqtSignal(SessionModel, object)

    def __init__(self, parent):
        super(HkuSessionViewWidget, self).__init__(parent)
        self.setObjectName("HKUServerViewWidget")
        self.tree = QtWidgets.QTreeWidget(self)
        self.setWidget(self.tree)
        #self.tree.header().setVisible(False)
        self.tree.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.icons = [QtGui.QIcon(':/icon/server_16.png')]  # 记录每一级别的图标

        # self.tree.setColumnCount(2)
        # self.tree.setColumnWidth(0, 150)
        # self.tree.setColumnWidth(1, 50)

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

    def set_gray(self, item: QtWidgets.QTreeWidgetItem):
        """  将item置为灰色，不用 setDisabled 的原因是会导致 currentItem() 失效 """
        item.setForeground(0, QtCore.Qt.gray)
        data = item.data(0, QtCore.Qt.UserRole)
        if data is not None:
            item.setIcon(0, QtGui.QIcon(':/icon/not_connect_16.png'))
        for i in range(item.childCount()):
            self.set_gray(item.child(i))

    def set_default(self, item: QtWidgets.QTreeWidgetItem):
        """ 将 item 恢复为默认颜色 """
        style = self.parent().ui_config.get('main_window', 'style', fallback='normal_style')
        data = item.data(0, QtCore.Qt.UserRole)
        if data is not None:
            item.setIcon(0, self.icons[0])
        if style == "dark_style":
            item.setForeground(0, QtCore.Qt.white)
        else:
            item.setForeground(0, QtCore.Qt.black)
        for i in range(item.childCount()):
            self.set_default(item.child(i))

    def on_tree_itemClicked(self, item: QtWidgets.QTreeWidgetItem, column):
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
            item_text = item.text(0)
            if item_text == _translate("HkuSessionViewWidget", "person info"):
                self.open_tab_tigger.emit(session, HkuPersonInfoWidget)
            elif item_text == _translate("HkuSessionViewWidget", "users manage"):
                self.open_tab_tigger.emit(session, HkuUserManagerWidget)
            else:
                pass
        else:
            item_text = item.text(0)
            if item_text == _translate("HkuSessionViewWidget", "person info"):
                self.open_tab_tigger.emit(session, HkuPersonInfoWidget)
            elif item_text == _translate("HkuSessionViewWidget", "Xueqiu account"):
                self.open_tab_tigger.emit(session, HkuXueqiuAccountWidget)


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
            subitem.setText(0, _translate("HkuSessionViewWidget", "users manage"))
            subitem = QtWidgets.QTreeWidgetItem(item)
            subitem.setText(0, _translate("HkuSessionViewWidget", "person info"))
        else:
            info = _translate("HkuSessionViewWidget", "person info")
            xq_account = _translate("HkuSessionViewWidget", "Xueqiu account")
            funds = _translate("HkuSessionViewWidget", "funds")
            positons = _translate("HkuSessionViewWidget", "positions")
            orders = _translate("HkuSessionViewWidget", "orders")
            fills = _translate("HkuSessionViewWidget", "fills")
            names = [xq_account, funds, positons, orders, fills, info]
            for name in names:
                subitem = QtWidgets.QTreeWidgetItem(item)
                subitem.setText(0, name)
        self.set_gray(item)

    def modifySession(self, item, session):
        item.setText(0, session.name)
        item.setData(0, QtCore.Qt.UserRole, session)

    def retranslateUi(self):
        self.tree.headerItem().setText(0, _translate("HkuSessionViewWidget", "name"))
        __sortingEnabled = self.tree.isSortingEnabled()
        #self.tree.setSortingEnabled(False)
        #self.tree.topLevelItem(0).setText(0, _translate("HkuSessionViewWidget", "local"))
        #self.tree.topLevelItem(0).child(0).setText(0, _translate("HkuSessionViewWidget", "account"))
        #self.tree.topLevelItem(1).setText(0, _translate("HkuSessionViewWidget", "other"))
        self.tree.setSortingEnabled(__sortingEnabled)
