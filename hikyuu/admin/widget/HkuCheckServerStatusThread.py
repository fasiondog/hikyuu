# -*- coding: utf-8 -*-

import resource
from service import AssisService

from PyQt5 import QtCore, QtGui, QtWidgets


class HkuCheckServerStatusThread(QtCore.QThread):
    def __init__(self, session_widget):
        super(HkuCheckServerStatusThread, self).__init__()
        self.session_widget = session_widget
        self.working = True
        self.first = True

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
            status, msg = AssisService.getServerStatus(session)
            if session.running:
                self.session_widget.set_default(item)
            else:
                self.session_widget.set_gray(item)
            # 刷新 treewidget 显示界面
            self.session_widget.tree.viewport().update()
        if items:
            self.first = False
