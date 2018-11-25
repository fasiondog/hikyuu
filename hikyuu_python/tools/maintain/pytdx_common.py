# coding:utf-8
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog/hikyuu
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

def ping(ip, port=7709, multithread=False):
    #print(ip, port, multithread)
    api = TdxHq_API(multithread=multithread)
    success = False
    starttime = time.time()
    try:
        with api.connect(ip, port, time_out=1):
            #x = api.get_security_count(0)
            x = api.get_index_bars(7, 1, '000001', 1, 800)
            if x:
                success = True
            y = api.get_security_bars(7, 0, '000001', 800, 800)
            if x and y:
                success = True

    except Exception as e:
        success = False
        #print(e)

    endtime = time.time()
    #print(success, endtime - starttime)
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


if __name__ == '__main__':
    import os
    import time
    starttime = time.time()

    #x = ping('119.147.212.81', 7709)
    x = search_best_tdx()
    #x = [i for i in x if i[0] == True]
    #x.sort(key=lambda item: item[1])
    #print(x)
    for i in x:
        print(i)
    print(len(x))
    print(len(hq_hosts))

    endtime = time.time()
    print("\nTotal time:")
    print("%.2fs" % (endtime - starttime))
    print("%.2fm" % ((endtime - starttime) / 60))
