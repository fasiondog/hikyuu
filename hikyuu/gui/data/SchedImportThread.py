#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import logging
import time
import datetime
from math import ceil
from PyQt5.QtCore import QThread, QWaitCondition, QMutex

from hikyuu.util import *
from hikyuu import Datetime, TimeDelta


class SchedImportThread(QThread):
    def __init__(self, config, parent_win):
        super(self.__class__, self).__init__()
        self.working = True
        self._config = config
        #self._interval = TimeDelta(seconds=config.get('sched', 'time', fallback='18:00'))
        self._win = parent_win
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
        current_date = Datetime(now.year, now.month, now.day)
        next_date = Datetime(now.year, now.month, now.day, 18)
        if current_date.dayOfWeek() == 5:
            next_time = next_date + TimeDelta(3)
        else:
            next_time = next_date + TimeDelta(1)
        return (next_date, next_time - now)

    @hku_catch()
    def run(self):
        next_datetime, delta = self.next_time_delta()
        self.logger.info("下次导入时间：{}".format(next_datetime))
        delta = int(delta.total_milliseconds())
        while self.working and not self.cond.wait(self.mutex, int(delta)):
            self._win.on_collect_start_pushButton_clicked()
            next_datetime, delta = self.next_time_delta()
            self.logger.info("下次导入时间：{}".format(next_datetime))
            delta = int(delta.total_milliseconds())


class_logger(SchedImportThread)