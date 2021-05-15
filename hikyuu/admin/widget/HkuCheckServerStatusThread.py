# -*- coding: utf-8 -*-

import resource
import service

from PyQt5 import QtCore, QtGui, QtWidgets


class HkuCheckServerStatusThread(QtCore.QThread):
    def __init__(self, session_widget):
        super(HkuCheckServerStatusThread, self).__init__()
        self.session_widget = session_widget
        self.working = True
        self.first = True
        self.icons = {
            "running": QtGui.QIcon(":/icon/circular_green.png"),
            "stop": QtGui.QIcon(":/icon/circular_yellow.png")
        }

    def run(self) -> None:
        while self.working:
            try:
                self._run()
            except Exception as e:
                print(e)
                pass
            if self.first:
                self.sleep(1)
            else:
                self.sleep(30)

    def _run(self):
        items = [self.session_widget.tree.topLevelItem(i) for i in range(self.session_widget.tree.topLevelItemCount())]
        for item in items:
            session = item.data(0, QtCore.Qt.UserRole)
            status, msg = service.getServerStatus(session)
            item.setText(1, msg)
            item.setIcon(1, self.icons[status])
            # 刷新 treewidget 显示界面
            self.session_widget.tree.viewport().update()
        if items:
            self.first = False
