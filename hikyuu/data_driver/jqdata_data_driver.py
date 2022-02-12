#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 weituo2002
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

####################################################################
#
# 使用 jqdata 作为K线数据驱动，可自行实现自己的K线数据驱动
#
# 感谢网友哥本哈斯根贡献
#
#####################################################################

from ..cpp.core import KDataDriver, DataDriverFactory
from hikyuu import KRecord, Query, Datetime, Parameter, KRecordList

from jqdatasdk import *
from datetime import *


class jqdataKDataDriver(KDataDriver):
    def __init__(self):
        super(jqdataKDataDriver, self).__init__('jqdata')

    def clone(self):
        return jqdataKDataDriver()

    def _init(self):
        """【重载接口】（可选）初始化子类私有变量"""
        self._max = {
            Query.DAY: 10,
            Query.WEEK: 2,
            Query.MONTH: 1,
            Query.QUARTER: 1,
            #Query.HALFYEAR:1,
            Query.YEAR: 1,
            Query.MIN: 25,
            Query.MIN5: 25,
            Query.MIN15: 25,
            Query.MIN30: 25,
            Query.MIN60: 25
        }
        return True

    def isIndexFirst(self):
        return False

    def canParallelLoad(self):
        return False

    def getKRecordList(self, market, code, query):  # ktype, start_ix, end_ix, out_buffer):
        """
        【重载接口】（必须）按指定的位置[start_ix, end_ix)读取K线数据至out_buffer
        
        :param str market: 市场标识
        :param str code: 证券代码
        :param Query.KType ktype: K线类型
        :param int start_ix: 起始位置
        :param int end_ix: 结束位置
        :param KRecordListPtr out_buffer: 传入的数据缓存，读取数据后使用 
                                           out_buffer.append(krecord) 加入数据        
        """
        if query.query_type == Query.DATE:
            print("未实现按日期查询")
            return KRecordList()
        start_ix = query.start
        end_ix = query.end
        if start_ix >= end_ix or start_ix < 0 or end_ix < 0:
            return KRecordList()

        data = self._get_bars(market, code, query.ktype)

        if len(data) < start_ix:
            return KRecordList()

        result = KRecordList()
        total = end_ix if end_ix < len(data) else len(data)
        for i in range(start_ix, total):
            record = KRecord()
            record.datetime = Datetime(data.index[i])
            record.open = data['open'][i]
            record.high = data['high'][i]
            record.low = data['low'][i]
            record.close = data['close'][i]
            record.amount = data['money'][i]
            record.volume = data['volume'][i]
            result.append(record)
        return result

    def getCount(self, market, code, ktype):
        """
        【重载接口】（必须）获取K线数量
        
        :param str market: 市场标识
        :param str code: 证券代码
        :param Query.KType ktype: K线类型        
        """
        data = self._get_bars(market, code, ktype)
        return len(data)

    def _getIndexRangeByDate(self, market, code, query):
        """
        【重载接口】（必须）按日期获取指定的K线数据
        
        :param str market: 市场标识
        :param str code: 证券代码
        :param Query query: 日期查询条件（QueryByDate）        
        """
        print("getIndexRangeByDate")

        if query.query_type != Query.DATE:
            return (0, 0)

        start_datetime = query.startDatetime
        end_datetime = query.endDatetime
        if start_datetime >= end_datetime or start_datetime > Datetime.max():
            return (0, 0)

        data = self._get_bars(market, code, query.kType)
        total = len(data)
        if total == 0:
            return (0, 0)

        mid, low = 0, 0
        high = total - 1
        while low <= high:
            tmp_datetime = Datetime(data.index[high])
            if start_datetime > tmp_datetime:
                mid = high + 1
                break

            tmp_datetime = Datetime(data.index[low])
            if tmp_datetime >= start_datetime:
                mid = low
                break

            mid = (low + high) // 2
            tmp_datetime = Datetime(data.index[mid])
            if start_datetime > tmp_datetime:
                low = mid + 1
            else:
                high = mid - 1

        if mid >= total:
            return (0, 0)

        start_pos = mid
        low = mid
        high = total - 1
        while low <= high:
            tmp_datetime = Datetime(data.index[high])
            if end_datetime > tmp_datetime:
                mid = high + 1
                break

            tmp_datetime = Datetime(data.index[low])
            if tmp_datetime >= end_datetime:
                mid = low
                break

            mid = (low + high) // 2
            tmp_datetime = Datetime(data.index[mid])
            if end_datetime > tmp_datetime:
                low = mid + 1
            else:
                high = mid - 1

        end_pos = total if mid >= total else mid
        if start_pos >= end_pos:
            return (0, 0)

        return (start_pos, end_pos)

    def getKRecord(self, market, code, pos, ktype):
        """
        【重载接口】（必须）获取指定位置的K线记录
        
        :param str market: 市场标识
        :param str code: 证券代码
        :param int pos: 指定位置（大于等于0）
        :param Query.KType ktype: K线类型        
        """
        record = KRecord()
        if pos < 0:
            return record

        data = self._get_bars(market, code, ktype)
        if data is None:
            return record

        if pos < len(data):
            record.datetime = Datetime(data.index[pos])
            record.open = data['open'][pos]
            record.high = data['high'][pos]
            record.low = data['low'][pos]
            record.close = data['close'][pos]
            record.amount = data['money'][pos]
            record.volume = data['volume'][pos]

        return record

    def _trans_ktype(self, ktype):  #此处的周月季年数据只是近似的，目前jqdata未提供聚宽网络平台上的get_bar函数，不能直接取，需要自行用日线数据拼装
        ktype_map = {
            Query.MIN: '1m',
            Query.MIN5: '5m',
            Query.MIN15: '15m',
            Query.MIN30: '30m',
            Query.MIN60: '60m',
            Query.DAY: '1d',
            Query.WEEK: '7d',
            Query.MONTH: '30d',
            Query.QUARTER: '90d',
            Query.YEAR: '365d'
        }
        return ktype_map.get(ktype)

    def _get_bars(self, market, code, ktype):
        data = []
        username = self.getParam('username')
        password = self.getParam('password')
        auth(username, password)

        jqdataCode = normalize_code(code)
        jqdata_ktype = self._trans_ktype(ktype)

        if jqdata_ktype is None:
            print("jqdata_ktype == None")
            return data

        print(jqdataCode)
        security_info = get_security_info(jqdataCode)

        if security_info is None:  #有可能取不到任何信息
            return data
        #print(security_info)

        data = get_price(jqdataCode, security_info.start_date, datetime.now(), jqdata_ktype)

        return data


#DataDriverFactory.regKDataDriver(jqdataKDataDriver())

#jqdata_param = Parameter()
#jqdata_param.set('type', 'jqdata')
#jqdata_param.set('username', '用户名')
#jqdata_param.set('password', '密码')

#base_param = sm.getBaseInfoDriverParameter()
#block_param = sm.getBlockDriverParameter()
#kdata_param = sm.getKDataDriverParameter()
#preload_param = sm.getPreloadParameter()
#hku_param = sm.getHikyuuParameter()

#切换K线数据驱动，重新初始化
#sm.init(base_param, block_param, jqdata_param, preload_param, hku_param)
