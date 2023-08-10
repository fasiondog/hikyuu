#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130128, Added by fasiondog
#===============================================================================
from hikyuu import *

import os

config_file = os.path.expanduser('~') + "/.hikyuu/hikyuu.ini"
if not os.path.exists(config_file):
    #检查老版本配置是否存在，如果存在可继续使用，否则异常终止
    data_config_file = os.path.expanduser('~') + "/.hikyuu/data_dir.ini"
    data_config = configparser.ConfigParser()
    data_config.read(data_config_file)
    data_dir = data_config['data_dir']['data_dir']
    if sys.platform == 'win32':
        config_file = data_dir + "\\hikyuu_win.ini"
    else:
        config_file = data_dir + "/hikyuu_linux.ini"
    if not os.path.exists(config_file):
        raise("未找到配置文件，请先使用数据导入工具导入数据（将自动生成配置文件）！！!")
    
    
#starttime = time.time()
#print "Loading Day Data ..."
hikyuu_init(config_file)
sm = StockManager.instance()
#endtime = time.time()
#print "%.2fs" % (endtime-starttime)