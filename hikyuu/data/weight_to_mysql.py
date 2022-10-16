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

from .common_mysql import get_marketid

def qianlong_import_weight(connect, src_dir, market):
    """导入钱龙格式的权息数据"""
    cur = connect.cursor()

    marketid = get_marketid(connect, market)

    src_path = pathlib.Path(src_dir + '/shase/weight') if market == 'SH' else pathlib.Path(src_dir + '/sznse/weight')
    wgt_file_list = list(src_path.glob('*.wgt'))

    records = []
    for wgt_file in wgt_file_list:
        code = wgt_file.stem
        cur.execute("select stockid from `hku_base`.`stock` where marketid=%s and code='%s'" % (marketid, code))
        stockid = [id[0] for id in cur.fetchall()]
        if not stockid:
            continue

        with wgt_file.open('rb') as sourcefile:
            stockid = stockid[0]
            cur.execute("select date from `hku_base`.`stkweight` where stockid=%s" % stockid)
            dateDict = dict([(i[0], None) for i in cur.fetchall()])

            #records = []
            data = sourcefile.read(36)
            while data:
                a = struct.unpack('iiiiiiiii', data)
                date = (a[0] >> 20) * 10000 + (((a[0] << 12) & 4294967295) >> 28) * 100 + ((a[0] & 0xffff) >> 11)
                if date not in dateDict\
                        and a[0] >= 0 and a[1] >= 0 and a[2] >= 0 and a[3] >= 0 and a[4] >= 0\
                        and a[5] >= 0 and a[6] >= 0 and a[7] >= 0:
                    records.append((stockid, date, a[1], a[2], a[3], a[4], a[5], a[6], a[7]))
                data = sourcefile.read(36)
            sourcefile.close()

    if records:
        cur.executemany("INSERT INTO `hku_base`.`stkweight` (stockid, date, countAsGift, \
                         countForSell, priceForSell, bonus, countOfIncreasement, totalCount, freeCount) \
                         VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s)",
                        records)

    connect.commit()
    cur.close()
    return len(records)
