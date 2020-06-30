#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130128, Added by fasiondog
#===============================================================================
from hikyuu import *

import os
#curdir = os.path.dirname(os.path.realpath(__file__))
#head, tail = os.path.split(curdir)
#head, tail = os.path.split(head)
#head, tail = os.path.split(head)

import sys
if sys.platform == 'win32':
    #config_file = os.path.join(head, "test_data/hikyuu_win.ini")
    config_file = "test_data/hikyuu_win.ini"
else:
    #config_file = os.path.join(head, "test_data/hikyuu_linux.ini")
    config_file = "test_data/hikyuu_linux.ini"
    #print(config_file)

#tmp_dir = os.path.join(head, "test/data/tmp")
tmp_dir = "test_data/tmp"
if not os.path.lexists(tmp_dir):
    os.mkdir(tmp_dir)

#starttime = time.time()
#print "Loading Day Data ..."
hikyuu_init(config_file)
sm = StockManager.instance()
#endtime = time.time()
#print "%.2fs" % (endtime-starttime)
