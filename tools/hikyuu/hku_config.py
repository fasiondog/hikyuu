#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20100220, Added by fasiondog
#===============================================================================

import os
import sys
import shutil

usr_dir = os.path.expanduser('~')
hku_dir = usr_dir + '/.hikyuu'

if not os.path.lexists(hku_dir):
    os.mkdir(hku_dir)

data_config_file = hku_dir + '/data_dir.ini'    
if os.path.exists(data_config_file):
    os.remove(data_config_file);

data_directory = input("\n请输入股票数据存放路径（默认c:\stock）：")
if data_directory == '':
    data_directory = 'c:\stock'

with open(data_config_file, 'w') as conf:
    conf.writelines(['[data_dir]\n', 'data_dir=' + data_directory])

def ask_question(question):
    flag = False
    x = ''
    while not flag:
        x = input(question)
        flag = True if (x=='' or x=='Y' or x=='y' or x=='N' or x=='n') else False
    return False if (x=='' or x=='N' or x=='n') else True
    
if not os.path.lexists(data_directory):
    shutil.copytree('config', data_directory)
    os.remove(data_directory + '/__init__.py')
    if sys.platform == 'win32':
        os.remove(data_directory + '/hikyuu_linux.ini')
    else:
        os.remove(data_directory + '/hikyuu_win.ini')
        
else:
    if os.path.lexists(data_directory + '/block'):
        x = ask_question("block目录已存在是否覆盖？(Y/[N])：")
        if x:
            shutil.rmtree(data_directory + '/block')
            shutil.copytree('config/block', data_directory + '/block')
            os.remove(data_directory + '/block/__init__.py')
    if os.path.exists(data_directory + '/logger.properties'):
        x = ask_question("logger.properties文件已存在是否覆盖？(Y/[N])：")
        if x:
            os.remove(data_directory + '/logger.properties')
            shutil.copy('config/logger.properties', data_directory + '/logger.properties')
    if os.path.exists(data_directory + '/importdata.ini'):
        x = ask_question("importdata.ini文件已存在是否覆盖？(Y/[N])：")
        if x:
            os.remove(data_directory + '/importdata.ini')
            shutil.copy('config/importdata.ini', data_directory + '/importdata.ini')
    if os.path.exists(data_directory + '/createdb.sql'):
        x = ask_question("createdb.sql文件已存在是否覆盖？(Y/[N])：")
        if x:
            os.remove(data_directory + '/logger.properties')
            shutil.copy('config/createdb.sql', data_directory + '/createdb.sql')

    if sys.platform == 'win32':
        hikyuu_ini = '/hikyuu_win.ini'
    else:
        hikyuu_ini = '/hikyuu_linux.ini'
    
    if os.path.exists(data_directory + hikyuu_ini):
        x = ask_question(hikyuu_ini[1:] + "文件已存在是否覆盖？(Y/[N])：")
        if x:
            os.remove(data_directory + hikyuu_ini)
            shutil.copy('config' + hikyuu_ini, data_directory + hikyuu_ini)
