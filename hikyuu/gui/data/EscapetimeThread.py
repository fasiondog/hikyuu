#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import time
from PyQt5.QtCore import QThread, pyqtSignal

class EscapetimeThread(QThread):
    message = pyqtSignal(list)

    def __init__(self):
        super(self.__class__, self).__init__()
        self.working = True
        self.msg_name = 'ESCAPE_TIME'
        self.init_time = time.time()

    def __del__(self):
        self.working = False
        #print("EscapetimeThread", "__del__", time.time() - self.init_time)
        self.wait()

    def send_message(self, msg):
        self.message.emit([self.msg_name] + msg)

    def stop(self):
        self.working = False
        self.wait()

    def run(self):
        while self.working:
            self.msleep(10)
            self.send_message([(time.time() - self.init_time),])
