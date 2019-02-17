#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# Aothor: fasiondog
# History: 20160407, Added by fasiondog
#===============================================================================

from hikyuu.trade_sys.signal import crtSG
from hikyuu.indicator import HHV, LLV, CLOSE, REF

def TurtleSG(self):
     n = self.getParam("n")
     k = self.getTO()
     c = CLOSE(k)
     h = REF(HHV(c, n), 1) #前n日高点
     L = REF(LLV(c, n), 1) #前n日低点
     for i in range(h.discard, len(k)):
         if (c[i] >= h[i]):
             self._addBuySignal(k[i].datetime)
         elif (c[i] <= L[i]):
             self._addSellSignal(k[i].datetime)

if __name__ == "__main__":
    from examples_init import *
    
    sg = crtSG(TurtleSG, {'n': 20}, 'TurtleSG')
    s = getStock("sh000001")
    k = s.getKData(Query(-500))
    
    #只有设置交易对象时，才会开始实际计算
    sg.setTO(k)
    dates = k.getDatetimeList()
    for d in dates:
        if (sg.shouldBuy(d)):
            print("买入：%s" % d)
        elif (sg.shouldSell(d)):
            print("卖出: %s" % d)
