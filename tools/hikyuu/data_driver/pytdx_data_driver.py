#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
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


from ._data_driver import KDataDriver, DataDriverFactory
from hikyuu import KRecord, Query, Datetime, Parameter

from pytdx.hq import TdxHq_API
from pytdx.params import TDXParams


class PytdxKDataDriver(KDataDriver):
    
    def __init__(self):
        super(PytdxKDataDriver, self).__init__('pytdx')
        
    def _init(self):
        self._max = {Query.DAY:10,
                     Query.WEEK:2,
                     Query.MONTH:1,
                     Query.QUARTER:1,
                     #Query.HALFYEAR:1,
                     Query.YEAR:1,
                     Query.MIN:25,
                     Query.MIN5:25,
                     Query.MIN15:25,
                     Query.MIN30:25,
                     Query.MIN60:25}
        return 
    
    def loadKData(self, market, code, ktype, start_ix, end_ix, out_buffer):
        if start_ix >= end_ix or start_ix <0 or end_ix <0:
            return
        
        data = self._get_bars(market, code, ktype)
                    
        if len(data) < start_ix:
            return
        
        total = end_ix if end_ix < len(data) else len(data)
        for i in range(start_ix, total):
            record = KRecord()
            record.datetime = Datetime(data[i].get('datetime'))
            record.openPrice = data[i].get('open')
            record.highPrice = data[i].get('high')
            record.lowPrice = data[i].get('low')
            record.closePrice = data[i].get('close')
            record.transAmount = data[i].get('amount')
            record.transCount = data[i].get('vol')
            out_buffer.append(record)
        
    
    def getCount(self, market, code, ktype):
        data = self._get_bars(market, code, ktype)
        return len(data) if data else 0
    
    def _getIndexRangeByDate(self, market, code, query):
        print("getIndexRangeByDate")

        if query.queryType != Query.DATE:
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
        high = total-1
        while low <= high:
            tmp_datetime = Datetime(data[high].get('datetime'))
            if start_datetime > tmp_datetime:
                mid = high + 1
                break
            
            tmp_datetime = Datetime(data[low].get('datetime'))
            if tmp_datetime >= start_datetime:
                mid = low
                break
            
            mid = (low + high) // 2
            tmp_datetime = Datetime(data[mid].get('datetime'))
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
            tmp_datetime = Datetime(data[high].get('datetime'))
            if end_datetime > tmp_datetime:
                mid = high + 1
                break
            
            tmp_datetime = Datetime(data[low].get('datetime'))
            if tmp_datetime >= end_datetime:
                mid = low
                break
            
            mid = (low + high) // 2
            tmp_datetime = Datetime(data[mid].get('datetime'))
            if end_datetime > tmp_datetime:
                low = mid + 1
            else:
                high = mid - 1
                
        end_pos = total if mid >= total else mid
        if start_pos >= end_pos:
            return (0,0)
        
        return (start_pos, end_pos)
        
        
    
    def getKRecord(self, market, code, pos, ktype):
        record = KRecord()
        if pos < 0:
            return record
        
        data = self._get_bars(market, code, ktype)
        if data is None:
            return record
        
        if pos < len(data):
            record.datetime = Datetime(data[pos].get('datetime'))
            record.openPrice = data[pos].get('open')
            record.highPrice = data[pos].get('high')
            record.lowPrice = data[pos].get('low')
            record.closePrice = data[pos].get('close')
            record.transAmount = data[pos].get('amount')
            record.transCount = data[pos].get('vol')
            
        return record
    
    def _trans_market(self, market):
        market_map = {'SH': TDXParams.MARKET_SH, 
                      'SZ': TDXParams.MARKET_SZ}
        return market_map.get(market)
    
    def _trans_ktype(self, ktype):
        ktype_map = {Query.MIN: TDXParams.KLINE_TYPE_1MIN,
                     Query.MIN5: TDXParams.KLINE_TYPE_5MIN,
                     Query.MIN15: TDXParams.KLINE_TYPE_15MIN,
                     Query.MIN30: TDXParams.KLINE_TYPE_30MIN,
                     Query.MIN60: TDXParams.KLINE_TYPE_1HOUR,
                     Query.DAY: TDXParams.KLINE_TYPE_RI_K,
                     Query.WEEK: TDXParams.KLINE_TYPE_WEEKLY,
                     Query.MONTH: TDXParams.KLINE_TYPE_MONTHLY,
                     Query.QUARTER: TDXParams.KLINE_TYPE_3MONTH,
                     Query.YEAR: TDXParams.KLINE_TYPE_YEARLY}
        return ktype_map.get(ktype)
    
    def _get_bars(self, market, code, ktype):
        data = []
        tdx_market = self._trans_market(market)
        if tdx_market is None:
            print("tdx_market == None")
            return data
        
        tdx_ktype = self._trans_ktype(ktype)
        if tdx_ktype is None:
            print("tdx_ktype == None")
            return data
        
        try:
            ip = self.getParam('ip')
            port = self.getParam('port')
        except:
            ip = '119.147.212.81'
            port = 7709
            
        api = TdxHq_API(raise_exception=True)
        
        with api.connect(ip, port):
            if (market == 'SH' and code[:3] == '000') \
                  or (market == 'SZ'  and code[:2] == '39'):
                for i in range(self._max[ktype]):
                    data += api.get_index_bars(tdx_ktype, tdx_market, code,
                                           (self._max[ktype]-1-i)*800,800)
            else:
                for i in range(self._max[ktype]):
                    data += api.get_security_bars(tdx_ktype, tdx_market, code,
                                                (self._max[ktype]-1-i)*800,800)
                
        return data
    
    
DataDriverFactory.regKDataDriver(PytdxKDataDriver())

tdx_param = Parameter()
tdx_param.set('type', 'pytdx')
tdx_param.set('ip', '119.147.212.81')
tdx_param.set('port', 7709)
