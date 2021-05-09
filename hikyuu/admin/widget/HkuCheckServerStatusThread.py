# -*- coding: utf-8 -*-

import sys
#if ".." not in sys.path:
#    sys.path.append("..")

import ServerApi

from PyQt5 import QtCore


class HkuCheckServerStatusThread(QtCore.QThread):
    """定时检测服务器状态"""
    status_changed = QtCore.pyqtSignal(list)

    def __init__(self, session_widget):
        super(HkuCheckServerStatusThread, self).__init__()
        self.session_widget = session_widget
        self.working = True

    def run(self) -> None:
        while self.working:
            try:
                self._run()
            except Exception as e:
                print(e)
                pass
            self.sleep(30)

    def _run(self):
        items = [self.session_widget.tree.topLevelItem(i) for i in range(self.session_widget.tree.topLevelItemCount())]
        sessions = [item.data(0, QtCore.Qt.UserRole) for item in items if item is not None]
        for session in sessions:
            status = ServerApi.getServerStatus(session)
            print(status)