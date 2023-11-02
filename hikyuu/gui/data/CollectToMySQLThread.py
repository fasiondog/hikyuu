#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
import logging
import time
import datetime
from math import ceil
from PyQt5.QtCore import QThread, QWaitCondition, QMutex

import mysql.connector
from mysql.connector import errorcode

from hikyuu import Datetime, TimeDelta
from hikyuu.util import *
from hikyuu.data.common_mysql import get_stock_list, get_marketid, get_table
from hikyuu.fetcher.stock.zh_stock_a_sina_qq import get_spot, get_spot_parallel


class CollectToMySQLThread(QThread):
    def __init__(self, config, market='SH'):
        super(self.__class__, self).__init__()
        self.working = True
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

        self._interval = TimeDelta(seconds=config.getint('collect', 'interval', fallback=60 * 60))
        self._phase1_start_time = Datetime(
            datetime.datetime.combine(
                datetime.date.today(),
                datetime.time.fromisoformat((config.get('collect', 'phase1_start', fallback='09:05')))
            )
        )
        self._phase1_end_time = Datetime(
            datetime.datetime.combine(
                datetime.date.today(),
                datetime.time.fromisoformat((config.get('collect', 'phase1_end', fallback='09:05')))
            )
        )
        self._use_zhima_proxy = config.getboolean('collect', 'use_zhima_proxy', fallback=False)

        self.cond = QWaitCondition()
        self.mutex = QMutex()

    def stop(self):
        if self.working:
            self.working = False
            self.cond.wakeAll()
            self.wait()
            if self._connect is not None:
                hku_trace('关闭数据库连接', logger=self.logger)
                self._connect.close()

    def __del__(self):
        self.stop()

    @hku_catch()
    def run(self):
        self.logger.info("{} 数据采集同步线程启动".format(self.market))
        stk_list = self.get_stock_list()
        hku_warn_if(not stk_list, "从数据库中获取的股票列表为空！", logger=self.logger)
        self.mutex.tryLock()

        end_delta = self._phase1_end_time - self._phase1_end_time.start_of_day()
        start_delta = self._phase1_start_time - self._phase1_start_time.start_of_day()

        start = Datetime.now()
        if self._phase1_start_time >= self._phase1_end_time:
            delta = TimeDelta(0)
        elif start >= self._phase1_end_time:
            delta = (self._phase1_start_time + TimeDelta(1) - start)
        elif start < self._phase1_start_time:
            delta = (self._phase1_start_time - start)
        else:
            delta = self._interval * ceil((start - self._phase1_start_time) / self._interval
                                          ) - (start - self._phase1_start_time)

        hku_info('{} 下次采集时间：{}', self.market, start + delta, logger=self.logger)
        delta = int(delta.total_milliseconds())
        while self.working and not self.cond.wait(self.mutex, int(delta)):
            last_time = Datetime.today() + end_delta
            start = Datetime.now()
            if start >= last_time:
                next_time = Datetime.today() + TimeDelta(1) + start_delta
                hku_info('{} 明日采集时间：{}', self.market, next_time, logger=self.logger)
                delta = next_time - start
                delta = int(delta.total_milliseconds())
                continue
            hku_trace("start：{}".format(start))
            self.collect(stk_list)
            end = Datetime.now()
            x = end - start
            if x + TimeDelta(seconds=1) < self._interval:
                delta = self._interval - x - TimeDelta(seconds=1)
                delta = int(delta.total_milliseconds())
                self.logger.info("{} {:<.2f} 秒后重新采集".format(self.market, delta / 1000))
                #self.sleep(delta)

        self.logger.info("{} 数据采集同步线程终止!".format(self.market))

    def record_is_valid(self, record):
        return record['amount'] > 0.0 and record['volume'] > 0.0 \
            and record['high'] >= record['open'] >= record['low'] > 0.0 \
            and record['high'] >= record['close'] >= record['low']

    @hku_catch(ret=False)
    def process_one_record(self, record):
        if not self.record_is_valid(record):
            return (0, 0)
        connect = self.get_connect()
        current_date = Datetime(record['datetime'].date()).number
        table = get_table(connect, self.market, record['code'], 'day')
        sql = 'replace into {} (date, open, high, low, close, amount, count) \
             values ({}, {}, {}, {}, {}, {}, {})'.format(
            table, current_date, record['open'], record['high'], record['low'], record['close'], record['amount'],
            record['volume']
        )
        cur = connect.cursor()
        cur.execute(sql)
        cur.close()
        return

    @hku_catch()
    def collect(self, stk_list):
        record_list = get_spot_parallel(stk_list, source='sina', use_proxy=self._use_zhima_proxy)
        hku_info("{} 网络获取数量：{}".format(self.market, len(record_list)))
        connect = self.get_connect()
        if self.marketid == None:
            self.marketid = get_marketid(connect, self.market)
        update_count = 0
        for record in record_list:
            if not self.working:
                break
            if self.process_one_record(record):
                update_count += 1
        connect.commit()
        hku_info("{} 数据库更新记录数：{}".format(self.market, update_count))

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


class_logger(CollectToMySQLThread)