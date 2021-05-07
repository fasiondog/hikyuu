# -*- coding: utf-8 -*-

import sys
sys.path.append("..")
from ServerApi import getServerStatus

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
            self.sleep(60)

    def _run(self):
        print("run...")
