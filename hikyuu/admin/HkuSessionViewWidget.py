# -*- coding: utf-8 -*-

from PyQt5 import QtWidgets, QtCore, QtGui
import resource


class HkuSessionViewWidget(QtWidgets.QDockWidget):
    def __init__(self, parent=None):
        super(HkuSessionViewWidget, self).__init__(parent)
        self.setObjectName("HKUServerViewWidget")
        self.tree = QtWidgets.QTreeWidget(self)
        self.setWidget(self.tree)
        self.tree.header().setVisible(False)
        item_0 = QtWidgets.QTreeWidgetItem(self.tree)
        item_0.setIcon(0, QtGui.QIcon(':/icon/server.png'))
        item_1 = QtWidgets.QTreeWidgetItem(item_0)
        item_0 = QtWidgets.QTreeWidgetItem(self.tree)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def addSession(self):
        pass

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.tree.headerItem().setText(0, _translate("Form", "name"))
        __sortingEnabled = self.tree.isSortingEnabled()
        self.tree.setSortingEnabled(False)
        self.tree.topLevelItem(0).setText(0, _translate("Form", "local"))
        self.tree.topLevelItem(0).child(0).setText(0, _translate("Form", "account"))
        self.tree.topLevelItem(1).setText(0, _translate("Form", "other"))
        self.tree.setSortingEnabled(__sortingEnabled)
