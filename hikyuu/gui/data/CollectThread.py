#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
import logging
import time
from PyQt5.QtCore import QThread

import mysql.connector
from mysql.connector import errorcode

from hikyuu.util import *
from hikyuu.data.common_mysql import get_stock_list
from hikyuu.fetcher.stock.zh_stock_a_sina_qq import get_spot


class CollectThread(QThread):
    def __init__(self, config, market='SH', interval=60):
        super(self.__class__, self).__init__()
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
        self.quotations = []
        if config['quotation']['stock']:
            self.quotations.append('stock')
        if config['quotation']['fund']:
            self.quotations.append('fund')
        self.logger.info(self.quotations)

    def stop(self):
        self.working = False
        if self._connect is not None:
            hku_info('关闭数据库连接', self.logger)
            self._connect.close()
        self.wait()

    def __del__(self):
        self.working = False
        if self._connect is not None:
            self._connect.close()
        self.wait()

    def run(self):
        self.logger.info("{} 数据采集同步线程启动 ({})".format(self.market, self.currentThreadId()))
        stk_list = self.get_stock_list()
        hku_warn_if(not stk_list, "从数据库中获取的股票列表为空！", self.logger)
        while self.working == True:
            start = time.time()
            self.collect(stk_list)
            end = time.time()
            x = end - start
            if x < self._interval:
                delta = int(self._interval - x)
                self.logger.info("{} {} 秒后重新采集".format(self.market, delta))
                self.sleep(delta)
        self.logger.info("{} 数据采集同步线程终止 ({})!".format(self.market, self.currentThreadId()))

    @hku_catch()
    def collect(self, stk_list):
        self.logger.info("{} collect".format(self.market))
        self.logger.info("{}".format(len(self.get_stock_list())))

    def get_connect(self):
        if self._connect:
            return self._connect
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

    @hku_catch(retry=2, ret=[])
    def get_stock_list(self):
        connect = self.get_connect()
        stk_list = get_stock_list(connect, self.market.upper(), self.quotations)
        return ["{}{}".format(self.market.lower(), item[2]) for item in stk_list if item[3] == 1]


class_logger(CollectThread)