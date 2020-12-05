#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
import logging
import time
from PyQt5.QtCore import QThread, QWaitCondition, QMutex

import mysql.connector
from mysql.connector import errorcode

from hikyuu import Datetime
from hikyuu.util import *
from hikyuu.data.common_mysql import get_stock_list, get_marketid, get_table
from hikyuu.fetcher.stock.zh_stock_a_sina_qq import get_spot


class CollectThread(QThread):
    def __init__(self, config, market='SH', interval=60):
        super(self.__class__, self).__init__()
        self.working = True
        self._interval = interval
        self._config = config
        self.market = market
        self.marketid = None
        assert config.getboolean("mysql", "enable", fallback=False)
        self._db_config = {
            'user': config['mysql']['usr'],
            'password': config['mysql']['pwd'],
            'host': config['mysql']['host'],
            'port': config['mysql']['port']
        }
        self._connect = None
        self.quotations = [
            'stock',
        ]  # sina 不支持基金，qq支持，此处屏蔽基金
        #if config['quotation']['stock']:
        #    self.quotations.append('stock')
        #if config['quotation']['fund']:
        #    self.quotations.append('fund')
        #self.logger.info(self.quotations)

        self.cond = QWaitCondition()
        self.mutex = QMutex()

    def stop(self):
        if self.working:
            self.working = False
            self.cond.wakeAll()
            self.wait()
            if self._connect is not None:
                hku_trace('关闭数据库连接', self.logger)
                self._connect.close()

    def __del__(self):
        self.stop()

    def run(self):
        self.logger.info("{} 数据采集同步线程启动".format(self.market))
        stk_list = self.get_stock_list()
        hku_warn_if(not stk_list, "从数据库中获取的股票列表为空！", self.logger)
        self.mutex.tryLock()
        while self.working == True:
            start = time.time()
            self.collect(stk_list)
            end = time.time()
            x = end - start
            if x < self._interval:
                delta = self._interval - x
                self.logger.info("{} {:<.2f} 秒后重新采集".format(self.market, delta))
                #self.sleep(delta)
                self.cond.wait(self.mutex, int(delta * 1000))
        self.logger.info("{} 数据采集同步线程终止!".format(self.market))

    @hku_catch()
    def collect(self, stk_list):
        record_list = get_spot(stk_list, source='sina', use_proxy=False)
        hku_trace("{} 获取数量：{}".format(self.market, len(record_list)))
        connect = self.get_connect()
        if self.marketid == None:
            self.marketid = get_marketid(connect, self.market)
        update_list = []
        insert_list = []
        market = self.market.lower()
        for record in record_list:
            current_date = Datetime(record['datetime'].date()).number
            table = get_table(connect, market, record['code'], 'day')
            hku_trace(table, self.logger)
            sql = 'select date from {} where date={}'.format(table, current_date)
            cur = connect.cursor()
            a = cur.execute(sql)
            hku_trace('a: {}'.format(a))
            #hku_trace(
            #    '({}, {}, {}, {}, {}, {}, {}'.format(
            #        current_date, record['open'], record['high'], record['low'],
            #        record['last_price'], record['amount'], record['volumn']
            #    )
            #)
            hku_trace(record)
            try:
                if a:
                    insert_list.append(
                        (
                            current_date, record['open'], record['high'], record['low'],
                            record['last_price'], record['amount'], record['volumn']
                        )
                    )
                else:
                    update_list.append(
                        (
                            record['open'], record['high'], record['low'], record['last_price'],
                            record['amount'], record['volumn'], current_date
                        )
                    )
            except:
                hku_trace(record)
            cur.close()

        if insert_list:
            cur = connect.cursor()
            cur.executemany(
                "insert into {} (date, open, high, low, close, amount, count) \
                 values (%s, %s, %s, %s, %s, %s, %s)".format(table), insert_list
            )
            connect.commit()
            cur.close()
        if update_list:
            cur = connect.cursor()
            cur.executemany(
                "update {} set open=%s, high=%s, low=%s, close=%s, amount=%s, count=%s \
                 where date=%s".format(table), update_list
            )
            connect.commit()
            cur.close()

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
        stk_list = get_stock_list(connect, self.market, self.quotations)
        return ["{}{}".format(self.market.lower(), item[2]) for item in stk_list if item[3] == 1]


class_logger(CollectThread)