# -*- coding: utf-8 -*-

import sys
sys.path.append("..")
from ServerApi import getServerStatus

from concurrent import futures
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
        params = [(session.host) for session in sessions]
        #with futures.ThreadPoolExecutor() as executor:
        #    res = executor.map(ping2, hosts, timeout=2)

        for session in sessions:
            print(session)