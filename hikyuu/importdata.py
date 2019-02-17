#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import urllib.request
import subprocess
import time
import sys
import os
import shutil
import configparser
 
if __name__ == "__main__":

    starttime = time.time()

    data_config_file = os.path.expanduser('~') + "/.hikyuu/data_dir.ini"
    data_config = configparser.ConfigParser()
    data_config.read(data_config_file)
    data_dir = data_config['data_dir']['data_dir']

    dest_filename = data_dir + "/weight/weight.rar"
    dest_dir = data_dir + "/weight"

    if os.path.lexists(dest_dir):
        shutil.rmtree(dest_dir)
    os.mkdir(dest_dir)

    print("下载权限信息...")
    net_filename = 'http://www.qianlong.com.cn/download/history/weight.rar'
    urllib.request.urlretrieve(net_filename, dest_filename)
    
    print("下载完毕，开始解压...")
    
    subprocess.call(['unrar', 'x', '-o+', dest_filename, dest_dir])

    endtime = time.time()
    print("%.2fs" % (endtime-starttime))
    print("%.2fm" % ((endtime-starttime)/60))
    
    print("\n")

    try:
        subprocess.call(['importdata'])
    except FileNotFoundError:
        subprocess.call(['./importdata'])

    
    
    
