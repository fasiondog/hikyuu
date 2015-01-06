#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20120926, Added by fasiondog
#===============================================================================

from hikyuu import *
from hikyuu.indicator import *
from hikyuu.trade_manage import *

from hikyuu.trade_sys.system import *
from hikyuu.trade_sys.environment import *
from hikyuu.trade_sys.condition import *
from hikyuu.trade_sys.moneymanager import *
from hikyuu.trade_sys.signal import *
from hikyuu.trade_sys.stoploss import *
from hikyuu.trade_sys.profitgoal import *
from hikyuu.trade_sys.slippage import *

from .drawplot import *
#import time

from . import volume as vl
from . import elder as el
from . import kaufman as kf

import sys
if sys.platform == 'win32':
    #config_file = "d:\\workspace\\hikyuu\\test\\data\\hikyuu_win.ini"
    config_file = "c:\\stock\\hikyuu_win.ini"
else:
    config_file = "/home/fasiondog/stock/hikyuu_linux.ini"
    
#starttime = time.time()
#print "Loading Day Data ..."
hikyuu_init(config_file)
sm = StockManager.instance()
#endtime = time.time()
#print "%.2fs" % (endtime-starttime)

KData.plot = kplot
KData.kplot = kplot
KData.mkplot = mkplot

Indicator.plot = iplot