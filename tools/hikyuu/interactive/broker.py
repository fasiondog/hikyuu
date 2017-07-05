#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20170704, Added by fasiondog
#===============================================================================

from hikyuu.trade_manage import *
    
class OrderBrokerWrap(OrderBrokerBase):
    """
    订单代理包装类，用户可以参考自定义自己的订单代理，加入额外的处理
    :param bool real: 下单前是否重新实时获取实时分笔数据
    :param float slip: 如果当前的卖一价格和指示买入的价格绝对差值不超过slip则下单，
                        否则忽略; 对卖出操作无效，立即以当前价卖出
    """
    def __init__(self, broker, real=True, slip=0.03):
        super(OrderBrokerWrap, self).__init__()
        self._broker = broker
        self._real = real
        self._slip=slip
        
    def _buy(self, code, price, num):
        if self._real:
            import tushare as ts
            df = ts.get_realtime_quotes(code)
            new_price = float(df.ix[0]['ask'])
            if (abs(new_price - price) <= self._slip):
                self._broker.buy(code, new_price, num)
            else:
                print("out of slip, not buy!!!!!!!!!!")
        else:
            self._broker.buy(code, price, num)
        
    def _sell(self, code, price, num):
        if self._real:
            import tushare as ts
            df = ts.get_realtime_quotes(code)
            new_price = float(df.ix[0]['bid'])
            self._broker.sell(code, new_price, num)
        else:
            self._broker.sell(code, price, num)
        
class TestOrderBroker:
    def __init__(self):
        pass
    
    def buy(self, code, price, num):
        print("买入：%s  %.3f  %i" % (code, price, num))
    
    def sell(self, code, price, num):
        print("卖出：%s  %.3f  %i" % (code, price, num))
    
        
def crtRB(broker, real=True, slip=0.03):
    """
    快速生成订单代理包装对象
    :param broker: 订单代理示例，必须拥有buy和sell方法，并且参数为 code, price, num
    :param bool real: 下单前是否重新实时获取实时分笔数据
    :param float slip: 如果当前的卖一价格和指示买入的价格绝对差值不超过slip则下单，
                        否则忽略; 对卖出操作无效，立即以当前价卖出
    """
    return OrderBrokerWrap(broker, real, slip)