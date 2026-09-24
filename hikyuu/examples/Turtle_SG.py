#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# Aothor: fasiondog
# History: 20160407, Added by fasiondog
# ===============================================================================

from hikyuu import *


class TurtleSignal(SignalBase):
    def __init__(self, n=20):
        super(TurtleSignal, self).__init__("TurtleSignal")
        self.set_param("n", 20)

    def _clone(self):
        return TurtleSignal()

    def _calculate(self, k):
        n = self.get_param("n")
        c = CLOSE(k)
        h = REF(HHV(c, n), 1)  # the high of the previous n days
        L = REF(LLV(c, n), 1)  # the low of the previous n days
        for i in range(h.discard, len(k)):
            if (c[i] >= h[i]):
                self._add_buy_signal(k[i].datetime)
            elif (c[i] <= L[i]):
                self._add_sell_signal(k[i].datetime)


if __name__ == "__main__":
    from examples_init import *

    sg = TurtleSignal()
    s = get_stock("sh000001")
    k = s.get_kdata(Query(-500))

    # The actual calculation starts only when the trading object is set
    sg.to = k
    dates = k.get_datetime_list()
    for d in dates:
        if (sg.should_buy(d)):
            print("Buy: %s" % d)
        elif (sg.should_sell(d)):
            print("Sell: %s" % d)
