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

try:
    # Try to get the hosts configuration in the user directory
    import os
    import sys
    import importlib
    config_path = "{}/.hikyuu".format(os.path.expanduser('~'))
    host_file = f"{config_path}/hosts.py"
    if os.path.exists(host_file):
        if config_path not in sys.path:
            sys.path.append(config_path)
        tmp = importlib.import_module(f'hosts')
        hq_hosts = tmp.hq_hosts
except:
    pass


def to_pytdx_market(market):
    """转换为pytdx的market"""
    pytdx_market = {'SZ': 0, 'SH': 1, 'BJ': 2}
    return pytdx_market[market.upper()]


def ping(ip, port=7709, multithread=False, timeout=1):
    api = TdxHq_API(multithread=multithread)
    success = False
    starttime = time.time()
    success = False
    try:
        if api.connect(ip, port, time_out=timeout):
            # x = api.get_security_count(0)
            # x = api.get_index_bars(7, 1, '000001', 800, 100)
            x = api.get_security_bars(7, 0, '000001', 800, 100)
            if x:
                success = True
    except Exception as e:
        print(e)
        pass
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

    # 校验 host 是否存在错误，取同样数据，相同结果最多的集合
    values = {}
    for host in x:
        api = TdxHq_API(multithread=False)
        if api.connect(host[2], host[3]):
            x = api.get_security_bars(9, 0, '159915', 0, 1)
            if x and len(x) > 0:
                if x[0]['close'] not in values:
                    values[x[0]['close']] = [host]
                else:
                    values[x[0]['close']].append(host)
    ret = []
    for _, host in values.items():
        if len(host) > len(ret):
            ret = host
    return ret


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
