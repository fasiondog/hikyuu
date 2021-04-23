# -*- coding: utf-8 -*-

import sys
sys.path.append("..")
import resource

from PyQt5 import QtWidgets, QtCore, QtGui


class HkuSessionViewWidget(QtWidgets.QDockWidget):
    def __init__(self, parent=None):
        super(HkuSessionViewWidget, self).__init__(parent)
        self.setObjectName("HKUServerViewWidget")
        self.tree = QtWidgets.QTreeWidget(self)
        self.setWidget(self.tree)
        self.tree.header().setVisible(False)
        self.tree.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        item_0 = QtWidgets.QTreeWidgetItem(self.tree)
        item_0.setIcon(0, QtGui.QIcon(':/icon/server.png'))
        item_1 = QtWidgets.QTreeWidgetItem(item_0)
        item_0 = QtWidgets.QTreeWidgetItem(self.tree)

        self.initContextMenu()
        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def initContextMenu(self):
        _translate = QtCore.QCoreApplication.translate
        self.server_menu = QtWidgets.QMenu(self)
        self.server_menu.addAction(self.parent().action_dict['action_file_session'])
        self.tree.customContextMenuRequested.connect(self.on_tree_customContextMenuRequested)

    def on_tree_customContextMenuRequested(self, pos):
        print("on_tree_customContextMenuRequested")
        item = self.tree.itemAt(pos)
        if item:
            print(item.parent)
            self.server_menu.exec(QtGui.QCursor.pos())

    def addSession(self):
        pass

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.tree.headerItem().setText(0, _translate("HkuSessionViewWidget", "name"))
        __sortingEnabled = self.tree.isSortingEnabled()
        self.tree.setSortingEnabled(False)
        self.tree.topLevelItem(0).setText(0, _translate("HkuSessionViewWidget", "local"))
        self.tree.topLevelItem(0).child(0).setText(0, _translate("HkuSessionViewWidget", "account"))
        self.tree.topLevelItem(1).setText(0, _translate("HkuSessionViewWidget", "other"))
        self.tree.setSortingEnabled(__sortingEnabled)
