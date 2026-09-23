#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# Author: fasiondog
# History: 1) 20130128, Added by fasiondog
#===============================================================================
from hikyuu import *

import os

config_file = os.path.expanduser('~') + "/.hikyuu/hikyuu.ini"
if not os.path.exists(config_file):
    # Check whether the old version configuration exists; if it exists, it can be used continuously, otherwise terminate with an exception
    data_config_file = os.path.expanduser('~') + "/.hikyuu/data_dir.ini"
    data_config = configparser.ConfigParser()
    data_config.read(data_config_file)
    data_dir = data_config['data_dir']['data_dir']
    if sys.platform == 'win32':
        config_file = data_dir + "\\hikyuu_win.ini"
    else:
        config_file = data_dir + "/hikyuu_linux.ini"
    if not os.path.exists(config_file):
        raise("The configuration file cannot be found, please import the data with the data import tool first (the configuration file will be generated automatically)!!!")
    
    
#starttime = time.time()
#print "Loading Day Data ..."
hikyuu_init(config_file)
sm = StockManager.instance()
#endtime = time.time()
#print "%.2fs" % (endtime-starttime)