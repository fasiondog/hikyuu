#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import logging
import time
import datetime
from math import ceil
from PyQt5.QtCore import QThread, QWaitCondition, QMutex, pyqtSignal

from hikyuu.util import *
from hikyuu import Datetime, TimeDelta


class SchedImportThread(QThread):
    message = pyqtSignal()

    def __init__(self, config):
        super(self.__class__, self).__init__()
        self.working = True
        self._config = config
        hh_mm = config.get('sched', 'time', fallback='18:00')
        x = hh_mm.split(':')
        self.hour = int(x[0])
        self.minute = int(x[1])
        self.cond = QWaitCondition()
        self.mutex = QMutex()

    def stop(self):
        if self.working:
            self.working = False
            self.cond.wakeAll()
            self.wait()

    def __del__(self):
        self.stop()

    def next_time_delta(self):
        now = Datetime.now()
        next_time = Datetime(now.year, now.month, now.day, self.hour, self.minute)
        if next_time < now:
            current_date = Datetime(now.year, now.month, now.day)
            if current_date.day_of_week() == 5:
                next_time = next_time + TimeDelta(3)
            else:
                next_time = next_time + TimeDelta(1)
        return (next_time, next_time - now)

    @hku_catch()
    def run(self):
        self.mutex.tryLock()
        next_datetime, delta = self.next_time_delta()
        self.logger.info("下次导入时间：{}".format(next_datetime))
        delta = int(delta.total_milliseconds())
        while self.working and not self.cond.wait(self.mutex, int(delta)):
            self.message.emit()
            next_datetime, delta = self.next_time_delta()
            self.logger.info("下次导入时间：{}".format(next_datetime))
            delta = int(delta.total_milliseconds())


class_logger(SchedImportThread)