#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# Aothor: fasiondog
# History: 20130128, Added by fasiondog
#===============================================================================

from hikyuu.trade_sys.signal import SignalBase
from hikyuu.indicator import HHV, LLV, CLOSE, REF

class TurtleSignal(SignalBase):
    def __init__(self, n = 20):
        super(TurtleSignal, self).__init__("TurtleSignal")
        self.setParam("n", 20)
        
    def _clone(self):
        return TurtleSignal()

    def _calculate(self):
        n = self.getParam("n")
        k = self.getTO()
        c = CLOSE(k)
        h = REF(HHV(c, n), 1)
        L = REF(LLV(c, n), 1)
        for i in range(h.discard, len(k)):
            if (c[i] >= h[i]):
                self._addBuySignal(k[i].datetime)
            elif (c[i] <= L[i]):
                self._addSellSignal(k[i].datetime)

if __name__ == "__main__":
    from examples_init import *
    
    sg = TurtleSignal()
    s = getStock("sh000001")
    k = s.getKData(Query(-500))
    sg.setTO(k)
    dates = k.getDatetimeList()
    for d in dates:
        if (sg.shouldBuy(d)):
            print("买入：%s" % str(d))
        elif (sg.shouldSell(d)):
            print("卖出: %s" % str(d))
