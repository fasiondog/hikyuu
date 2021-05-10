# -*- coding: utf-8 -*-

import resource
import ServerApi

from PyQt5 import QtCore, QtGui


class HkuCheckServerStatusThread(QtCore.QThread):
    """定时检测服务器状态"""
    status_changed = QtCore.pyqtSignal(list)

    def __init__(self, session_widget):
        super(HkuCheckServerStatusThread, self).__init__()
        self.session_widget = session_widget
        self.working = True
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
            self.sleep(5)

    def _run(self):
        items = [self.session_widget.tree.topLevelItem(i) for i in range(self.session_widget.tree.topLevelItemCount())]
        for item in items:
            session = item.data(0, QtCore.Qt.UserRole)
            status, msg = ServerApi.getServerStatus(session)
            item.setText(1, msg)
            item.setIcon(1, self.icons[status])
