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

import time
from concurrent import futures
from pytdx.hq import TdxHq_API
from pytdx.config.hosts import hq_hosts

# hq_hosts = [
#     ('上海双线主站1', '47.103.48.45', 7709),
#     ('上海双线主站2', '47.103.86.229', 7709),
#     ('上海双线主站3', '47.103.88.146', 7709),
#     ('上海电信主站z1', '180.153.18.170', 7709),
#     ('上海电信主站z2', '180.153.18.171', 7709),
#     ('上海电信主站z3', '180.153.39.51', 7709),
#     ('上海电信主站z4', '58.34.106.207', 7709),
#     ('上海移动主站z1', '120.253.221.207', 7709),
#     ('上海电信主站z80', '180.153.18.172', 7709),
#     ('深圳双线主站1', '120.79.60.82', 7709),
#     ('深圳双线主站2', '47.112.129.66', 7709),
#     ('北京双线主站1', '39.98.234.173', 7709),
#     ('北京双线主站2', '39.98.198.249', 7709),
#     ('北京双线主站3', '39.100.68.59', 7709),
#     ('北京联通主站z1', '202.108.254.67', 7709),
#     ('北京移动主站z1', '111.13.112.206', 7709),
# ]


def to_pytdx_market(market):
    """转换为pytdx的market"""
    pytdx_market = {'SZ': 0, 'SH': 1, 'BJ': 2}
    return pytdx_market[market.upper()]


def ping(ip, port=7709, multithread=False):
    api = TdxHq_API(multithread=multithread)
    success = False
    starttime = time.time()
    try:
        with api.connect(ip, port, time_out=1):
            # x = api.get_security_count(0)
            # x = api.get_index_bars(7, 1, '000001', 800, 100)
            x = api.get_security_bars(7, 0, '000001', 800, 100)
            if x:
                success = True

    except Exception as e:
        success = False

    endtime = time.time()
    return (success, endtime - starttime, ip, port)


def search_best_tdx():
    def ping2(host):
        return ping(host[0], host[1], host[2])

    hosts = [(host[1], host[2], True) for host in hq_hosts]
    with futures.ThreadPoolExecutor() as executor:
        res = executor.map(ping2, hosts, timeout=2)
    x = [i for i in res if i[0] == True]
    x.sort(key=lambda item: item[1])
    return x


def pytdx_get_day_trans(api, pymarket, code, date):
    buf = []
    for i in range(21):
        x = api.get_history_transaction_data(pymarket, code, i * 2000, 2000,
                                             date)
        # x = api.get_transaction_data(TDXParams.MARKET_SZ, '000001', (9-i)*800, 800)
        if not x:
            break
        buf = x + buf
    return buf


if __name__ == '__main__':
    import time
    starttime = time.time()

    x = search_best_tdx()
    for i in x:
        print(i)
    print(len(x))
    print(len(hq_hosts))

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
