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


from ._data_driver import KDataDriver
from hikyuu import KRecord, Query

from pytdx.hq import TdxHq_API


class PytdxKDataDriver(KDataDriver):
    def __init__(self):
        super(PytdxKDataDriver, self).__init__('pytdx')
        
    def _init(self):
        ip = self.getParam('ip')
        port = self.getParam('port')
        self._tdx = TdxHq_API(heartbeat=True)
        return self._tdx.connect(ip, port)
    
    def loadKData(self, market, code, ktype, start_ix, end_ix, out_buffer):
        new_market = self._trans_market(market)
        pass
    
    def getCount(self, market, code, ktype):
        return 0
    
    def getIndexRangeByDate(self, market, code, query, out_start, out_end):
        pass
    
    def getKRecord(self, market, code, pos, ktype):
        return KRecord()
    
    def _trans_market(self, market):
        market_map = {'SH': 1, 'SZ': 0}
        return market_map[market.upper()]
    
    def _trans_ktype(self, ktype):
        ktype_map = {Query.MIN5: 0,
                     Query.MIN15: 1,
                     Query.MIN30: 2,
                     Query.MIN60: 3,
                     Query.DAY: 9,
                     Query.WEEK: 5,
                     Query.MONTH: 6,
                     Query.MIN: 8,
                     Query.QUARTER: 10,
                     Query.YEAR: 11}
        return ktype_map[ktype]
