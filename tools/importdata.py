#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import urllib.request
import subprocess
import time
import sys
 
if __name__ == "__main__":

    starttime = time.time()
    
    net_filename = 'http://www.qianlong.com.cn/download/history/weight.rar'

    if sys.platform == 'win32':
        dest_filename = "c:\\stock\\weight\\weight.rar"
        dest_dir = "c:\\stock\\weight"
        subprocess.call(['del', dest_filename], shell=True)
    else:
        dest_filename = "/home/fasiondog/stock/weight/weight.rar"
        dest_dir = "/home/fasiondog/stock/weight"
        subprocess.call(['rm', dest_filename])

    #subprocess.call(['rmdir', '/S', '/Q', "d:\\stock\\weight\\WEIGHT_sh"], shell=True)
    #subprocess.call(['rmdir', '/S', '/Q', "d:\\stock\\weight\\WEIGHT_sz"], shell=True)
    urllib.request.urlretrieve(net_filename, dest_filename)
    subprocess.call(['unrar', 'x', '-o+', dest_filename, dest_dir])
    subprocess.call(['importdata'])
    
    endtime = time.time()
    print("%.2fs" % (endtime-starttime))
    print("%.2fm" % ((endtime-starttime)/60))
    
    
