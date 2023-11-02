#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
import logging
import time
import datetime
from math import ceil
from PyQt5.QtCore import QThread, QWaitCondition, QMutex

from hikyuu import Datetime, TimeDelta, hikyuu_init, StockManager, constant
from hikyuu.util import *
from hikyuu.fetcher.stock.zh_stock_a_sina_qq import get_spot, get_spot_parallel
from hikyuu.data.sqlite_mem import SqliteMemDriver


class CollectToMemThread(QThread):
    def __init__(self, config, hku_config_file, market='SH'):
        super(self.__class__, self).__init__()
        self.working = True
        self._config = config
        self.hku_config_file = hku_config_file
        self.market = market.lower()
        self.marketid = None
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

    def __del__(self):
        self.stop()

    @hku_catch()
    def run(self):
        self.logger.info("{} 数据采集同步线程启动".format(self.market))
        self.db = SqliteMemDriver()

        hikyuu_init(self.hku_config_file, ignore_preload=True)

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

        self.db.close()
        self.logger.info("{} 数据采集同步线程终止!".format(self.market))

    @hku_catch(retry=2, ret=[])
    def get_stock_list(self):
        sm = StockManager.instance()
        return [
            stk.market_code.lower() for stk in sm
            if stk.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX,
                            constant.STOCKTYPE_GEM) and stk.valid and stk.market.lower() == self.market.lower()
        ]

    def record_is_valid(self, record):
        return record['amount'] > 0.0 and record['volume'] > 0.0 \
            and record['high'] >= record['open'] >= record['low'] > 0.0 \
            and record['high'] >= record['close'] >= record['low']

    @hku_catch()
    def collect(self, stk_list):
        record_list = get_spot_parallel(stk_list, source='sina', use_proxy=self._use_zhima_proxy)
        hku_info("{} 网络获取数量：{}".format(self.market, len(record_list)))
        record_list = [r for r in record_list if self.record_is_valid(r)]
        self.db.insert_bars(self.market, record_list, 'day')
        self.db.insert_bars(self.market, record_list, 'min')


class_logger(CollectToMemThread)