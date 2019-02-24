# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2019 fasiondog/hikyuu
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

import struct
import pathlib

def qianlong_import_weight(connect, src_dir, market):
    """导入钱龙格式的权息数据"""
    cur = connect.cursor()
    marketid = cur.execute("select marketid from Market where market='%s'" % market)
    marketid = [id[0] for id in marketid]
    marketid = marketid[0]

    src_path = pathlib.Path(src_dir + '/shase/weight') if market == 'SH' else pathlib.Path(src_dir + '/sznse/weight')
    wgt_file_list = list(src_path.glob('*.wgt'))

    total_count = 0
    for wgt_file in wgt_file_list:
        code = wgt_file.stem
        stockid = cur.execute("select stockid from Stock where marketid=%s and code='%s'" % (marketid, code))
        stockid = [id[0] for id in stockid]
        if not stockid:
            continue

        with wgt_file.open('rb') as sourcefile:
            stockid = stockid[0]
            a = cur.execute("select date from stkweight where stockid=%s" % stockid)
            dateDict = dict([(i[0], None) for i in a])

            records = []
            data = sourcefile.read(36)
            while data:
                a = struct.unpack('iiiiiiiii', data)
                date = (a[0] >> 20) * 10000 + (((a[0] << 12) & 4294967295) >> 28) * 100 + ((a[0] & 0xffff) >> 11)
                if date not in dateDict:
                    records.append((stockid, date, a[1], a[2], a[3], a[4], a[5], a[6], a[7]))
                data = sourcefile.read(36)
            sourcefile.close()

            if records:
                cur.executemany("INSERT INTO StkWeight(stockid, date, countAsGift, \
                                 countForSell, priceForSell, bonus, countOfIncreasement, totalCount, freeCount) \
                                 VALUES (?,?,?,?,?,?,?,?,?)",
                                records)
                total_count += len(records)

    connect.commit()
    cur.close()
    return total_count
