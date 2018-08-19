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

def ask_question(question, default_anwser='N'):
    flag = False
    x = ''
    while not flag:
        x = input(question)
        if x == '':
            x = default_anwser
        flag = True if (x=='' or x=='Y' or x=='y' or x=='N' or x=='n') else False
    return False if (x=='' or x=='N' or x=='n') else True

data_config_file = hku_dir + '/data_dir.ini'    
if os.path.exists(data_config_file):
    os.remove(data_config_file);

default_dir = "c:\stock" if sys.platform == "win32" else usr_dir + "/stock"
data_directory = input("\n请输入股票数据存放路径（默认" + default_dir + "）：")
if data_directory == '':
    data_directory = default_dir
        
with open(data_config_file, 'w', encoding = 'utf-8') as conf:
    conf.writelines(['[data_dir]\n', 'data_dir=' + data_directory])


import_config = """
;db - 指定基础信息数据库
;sql - 当指定的基础信息数据库不存在时，使用该指定的脚本创建数据库
[database]
db = {dir}/stock.db
sql = {dir}/createdb.sql

; ☆☆☆ 注：权息信息目前指支持钱龙数据格式
;sh、sz分别指定上证、深圳的权息数据所在目录
[weight]
sh = {dir}/weight/shase/weight
sz = {dir}/weight/sznse/weight
;sh = D:\LON\QLDATA\history\SHASE\weight
;sz = D:\LON\QLDATA\history\SZNSE\weight

;指定K线数据源，格式为 “类型, 根目录”（类型仅能为 dzh 或 tdx，即大智慧、通达信）
;可多个数据源同时导入，防止信息遗漏
[from]
1 = tdx, {tdx}
;2 = dzh, d:\DZH5

;指定K线导入的目标文件
;选项“all”代表不管基础信息库中是否存在该股票，都将其K线导入
;如果选择all选择为true，将会导入很多不需要的K线数据，一般仅用于备份数据或将来使用
;all默认为false，如目前数据库仅配置导入A股、B股、基金，而不导入国债
[dest]
;all = True
sh_day = {dir}/sh_day.h5
sz_day = {dir}/sz_day.h5
sh_5min = {dir}/sh_5min.h5
sz_5min = {dir}/sz_5min.h5
sh_1min = {dir}/sh_1min.h5
sz_1min = {dir}/sz_1min.h5
"""

hikyuu_config = """
[hikyuu]
tmpdir = {dir}/tmp
logger = {dir}/logger.properties

[block]
type  = qianlong
dir = {dir}/block
指数板块 = zsbk.ini
行业板块 = hybk.ini
地域板块 = dybk.ini
概念板块 = gnbk.ini
self = self.ini

[preload]
day = 1
week = 0
month = 0
quarter = 0
halfyear = 0
year = 0

[baseinfo]
type = sqlite3
db = {dir}/stock.db

[kdata]
;type = tdx
;dir = D:\\TdxW_HuaTai\\vipdoc
type = hdf5
sh_day = {dir}/sh_day.h5
sh_min = {dir}/sh_1min.h5
sh_min5 = {dir}/sh_5min.h5
sz_day = {dir}/sz_day.h5
sz_min = {dir}/sz_1min.h5
sz_min5 = {dir}/sz_5min.h5
"""

tdx_default_dir = "c:\TdxW_HuaTai" if sys.platform == "win32" else usr_dir + "/.wine/driver/c/Tdx_HuaTai"
tdx_input_str = "请输入通达信安装路径（默认从" + tdx_default_dir + "）："
    
if not os.path.lexists(data_directory):
    shutil.copytree('config', data_directory)
    os.remove(data_directory + '/__init__.py')

    if sys.platform == 'win32':
        filename = data_directory + '/hikyuu_win.ini'
    else:
        filename = data_directory + '/hikyuu_linux.ini'

    with open(filename, 'w', encoding = 'utf-8') as f:
        f.write(hikyuu_config.format(dir=data_directory))
    
    tdx_dir = input(tdx_input_str)
    if tdx_dir == '':
        tdx_dir = tdx_default_dir

    with open(data_directory + '/importdata.ini', 'w', encoding = 'utf-8') as f:
        f.write(import_config.format(dir=data_directory, tdx=tdx_dir))
        
    os.mkdir(data_directory + '/tmp')

    
else:
    if os.path.lexists(data_directory + '/block'):
        pass
        x = ask_question("block目录已存在是否覆盖？(Y/[N])：")
        if x:
            shutil.rmtree(data_directory + '/block')
            shutil.copytree('config/block', data_directory + '/block')
            os.remove(data_directory + '/block/__init__.py')
    else:
        shutil.copytree('config/block', data_directory + '/block')
        os.remove(data_directory + '/block/__init__.py')
    
    if os.path.exists(data_directory + '/logger.properties'):
        pass
        x = ask_question("logger.properties文件已存在是否覆盖？(Y/[N])：")
        if x:
            os.remove(data_directory + '/logger.properties')
            shutil.copy('config/logger.properties', data_directory + '/logger.properties')
    else:
        shutil.copy('config/logger.properties', data_directory + '/logger.properties')
        
    if os.path.exists(data_directory + '/importdata.ini'):
        x = ask_question("importdata.ini文件已存在是否覆盖？(Y/[N])：")
        if x:
            os.remove(data_directory + '/importdata.ini')
            tdx_dir = input(tdx_input_str)
            if tdx_dir == '':
                tdx_dir = tdx_default_dir
            with open(data_directory + '/importdata.ini', 'w', encoding = 'utf-8') as f:
                f.write(import_config.format(dir=data_directory, tdx=tdx_dir))
    else:
        tdx_dir = input(tdx_input_str)
        if tdx_dir == '':
            tdx_dir = tdx_default_dir
        with open(data_directory + '/importdata.ini', 'w', encoding = 'utf-8') as f:
            f.write(import_config.format(dir=data_directory, tdx=tdx_dir))
            
    if os.path.exists(data_directory + '/createdb.sql'):
        os.remove(data_directory + '/createdb.sql')
    shutil.copy('config/createdb.sql', data_directory + '/createdb.sql')

    if sys.platform == 'win32':
        hikyuu_ini = '/hikyuu_win.ini'
    else:
        hikyuu_ini = '/hikyuu_linux.ini'
        
    filename = data_directory + hikyuu_ini
    
    if os.path.exists(data_directory + hikyuu_ini):
        x = ask_question(hikyuu_ini[1:] + "文件已存在是否覆盖？(Y/[N])：")
        if x:
            os.remove(filename)
        with open(data_directory + hikyuu_ini, 'w', encoding = 'utf-8') as f:
            f.write(hikyuu_config.format(dir=data_directory))
    else:        
        with open(data_directory + hikyuu_ini, 'w', encoding = 'utf-8') as f:
            f.write(hikyuu_config.format(dir=data_directory))

    if not os.path.lexists(data_directory + '/tmp'):
        os.mkdir(data_directory + '/tmp')
            
#print(import_config.format(dir=data_directory))            
