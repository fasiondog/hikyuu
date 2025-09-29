#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import time
from PySide6.QtCore import QThread, Signal

class EscapetimeThread(QThread):
    message = Signal(list)

    def __init__(self):
        super(self.__class__, self).__init__()
        self.working = True
        self.msg_name = 'ESCAPE_TIME'
        self.init_time = time.time()

    def run(self):
        while self.working:
            time.sleep(1)
            current_time = time.time()
            self.message.emit([self.msg_name, round(current_time - self.init_time, 1)])

    def stop(self):
        self.working = False
        self.quit()
        self.wait()
