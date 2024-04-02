#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# Aothor: fasiondog
# History: 20160407, Added by fasiondog
# ===============================================================================

from hikyuu import *


def TurtleSG(self, k):
    n = self.get_param("n")
    c = CLOSE(k)
    h = REF(HHV(c, n), 1)  # 前n日高点
    L = REF(LLV(c, n), 1)  # 前n日低点
    for i in range(h.discard, len(k)):
        if (c[i] >= h[i]):
            self._add_buy_signal(k[i].datetime)
        elif (c[i] <= L[i]):
            self._add_sell_signal(k[i].datetime)


if __name__ == "__main__":
    from examples_init import *

    sg = crtSG(TurtleSG, {'n': 20}, 'TurtleSG')
    s = get_stock("sh000001")
    k = s.get_kdata(Query(-500))

    # 只有设置交易对象时，才会开始实际计算
    sg.to = k
    dates = k.get_datetime_list()
    for d in dates:
        if (sg.should_buy(d)):
            print("买入：%s" % d)
        elif (sg.should_sell(d)):
            print("卖出: %s" % d)
