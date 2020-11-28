#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
import logging
import time
from PyQt5.QtCore import QThread

import mysql.connector
from mysql.connector import errorcode


class CollectThread(QThread):
    def __init__(self, config, market='SH', interval=60):
        super(self.__class__, self).__init__()
        self.logger = logging.getLogger(self.__class__.__name__)
        self.working = True
        self._interval = interval
        self._config = config
        self.market = market
        assert config.getboolean("mysql", "enable", fallback=False)
        self._db_config = {
            'user': config['mysql']['usr'],
            'password': config['mysql']['pwd'],
            'host': config['mysql']['host'],
            'port': config['mysql']['port']
        }
        self._connect = None

    def __del__(self):
        self.working = False
        if self._connect is not None:
            self._connect.close()
        self.wait()

    def run(self):
        self.logger.info("{} 数据采集同步线程启动 ({})".format(self.market, self.currentThreadId()))
        while self.working == True:
            start = time.time()
            self.collect()
            end = time.time()
            x = end - start
            if x < self._interval:
                delta = int(self._interval - x)
                self.logger.info("{} {} 秒后重新采集".format(self.market, delta))
                self.sleep(delta)
        self.logger.info("{} 数据采集同步线程终止 ({})!".format(self.market, self.currentThreadId()))

    def collect(self):
        self.logger.info("{} collect".format(self.market))

    def get_connect(self):
        if self._connect is None:
            try:
                self._connect = mysql.connector.connect(**self._db_config)
            except mysql.connector.Error as err:
                if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
                    self.logger.error("MYSQL密码或用户名错误！")
                elif err.errno == errorcode.ER_BAD_DB_ERROR:
                    self.logger.error("MySQL数据库不存在！")
                else:
                    self.logger.error("连接数据库失败，{}".format(err.msg))
            except:
                self.logger.error("未知原因导致无法连接数据库！")
        return self._connect
