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
        self.market = market.lower()
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
        delta = 0
        while not self.cond.wait(self.mutex, int(delta * 1000)) and self.working:
            start = time.time()
            hku_trace("start：{}".format(Datetime.now()))
            self.collect(stk_list)
            end = time.time()
            x = end - start
            if x + 1.0 < self._interval:
                delta = self._interval - x - 1.0
                self.logger.info("{} {:<.2f} 秒后重新采集".format(self.market, delta))
                #self.sleep(delta)

        self.logger.info("{} 数据采集同步线程终止!".format(self.market))

    def record_is_valid(self, record):
        return record['amount'] > 0.0 and record['volumn'] > 0.0 \
            and record['high'] >= record['open'] >= record['low'] > 0.0 \
            and record['high'] >= record['last_price'] >= record['low']

    @hku_catch(ret=[])
    def process_one_record(self, record):
        if not self.record_is_valid(record):
            return (0, 0)
        connect = self.get_connect()
        current_date = Datetime(record['datetime'].date()).number
        table = get_table(connect, self.market, record['code'], 'day')
        sql = 'select date,open,high,low,close,amount,count from {} where date={}'.format(
            table, current_date
        )
        cur = connect.cursor()
        cur.execute(sql)
        a = [x for x in cur]
        insert_count = 0
        update_count = 0
        sql = None
        if not a:
            sql = 'insert into {} (date, open, high, low, close, amount, count) \
                 values ({}, {}, {}, {}, {}, {}, {})'.format(
                table, current_date, record['open'], record['high'], record['low'],
                record['last_price'], record['amount'], record['volumn']
            )
            insert_count += 1
        else:
            old = a[0]
            if old[1] != record['open'] or old[2] != record['high'] or old[3] != record['low'] \
                    or old[4] != record['last_price'] or old[5] != record['amount'] \
                    or old[6] != record['volumn']:
                sql = "update {} set open={}, high={}, low={}, close={}, amount={}, count={} \
                    where date={}".format(
                    table, record['open'], record['high'], record['low'], record['last_price'],
                    record['amount'], record['volumn'], current_date
                )
                update_count += 1
        if sql is not None:
            cur.execute(sql)
        cur.close()
        return (insert_count, update_count)

    @hku_catch()
    def collect(self, stk_list):
        record_list = get_spot(stk_list, source='sina', use_proxy=False)
        hku_info("{} 网络获取数量：{}".format(self.market, len(record_list)))
        connect = self.get_connect()
        if self.marketid == None:
            self.marketid = get_marketid(connect, self.market)
        insert_count = 0
        update_count = 0
        for record in record_list:
            if not self.working:
                break
            insert, update = self.process_one_record(record)
            insert_count += insert
            update_count += update
        connect.commit()
        hku_info("{} 数据库插入记录数：{}，更新记录数：{}".format(self.market, insert_count, update_count))

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