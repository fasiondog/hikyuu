#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# 作者：fasiondog
# 历史：1）20130128, Added by fasiondog
# ===============================================================================
from hikyuu import *
from configparser import ConfigParser
from functools import lru_cache

import os
# curdir = os.path.dirname(os.path.realpath(__file__))
# head, tail = os.path.split(curdir)
# head, tail = os.path.split(head)
# head, tail = os.path.split(head)

import sys
if sys.platform == 'win32':
    # config_file = os.path.join(head, "test_data/hikyuu_win.ini")
    config_file = "test_data/hikyuu_win.ini"
else:
    # config_file = os.path.join(head, "test_data/hikyuu_linux.ini")
    config_file = "test_data/hikyuu_linux.ini"
    # print(config_file)

# tmp_dir = os.path.join(head, "test/data/tmp")
tmp_dir = "test_data/tmp"
if not os.path.lexists(tmp_dir):
    os.mkdir(tmp_dir)

# starttime = time.time()
# print "Loading Day Data ..."
hikyuu_init(config_file)
sm = StockManager.instance()
# endtime = time.time()
# print "%.2fs" % (endtime-starttime)

# 仅在模块加载时读取一次配置
_config = ConfigParser()
_config.read(config_file, encoding='utf-8')
_tdx_dir = _config.get('tdx', 'dir', fallback='/mnt/tdx')


@lru_cache(maxsize=None)
def get_real_tdx_filepath(code, market='sh'):
    """
    获取真实的通达信.day文件路径。

    该函数按以下顺序确定文件路径:
    1. 检查环境变量 `HKU_real_tdx_file_path` 是否被设置。
    2. 如果未设置, 则检查配置文件中 "tdx"->"dir" 指定的路径。
    3. 如果未配置, 则使用 /mnt/tdx/vipdoc/。
    4. 如果上述路径不存在, 则检查 test_data/vipdoc/ 是否存在, 如果存在, 则使用该路径。
    5. 如果上述路径都不存在, 则自动回退到在项目根目录下的 `test_data` 文件夹中寻找同名文件作为备用。

    这使得测试既能适应有权访问生产数据环境的开发者, 也能让其他贡献者使用
    项目内提供的样本数据来运行测试, 避免了因路径问题导致的测试挂起或失败。

    :param str code: 股票代码 (如 '600000', '000001')
    :param str market: 市场简称 (如 'sh', 'sz')
    :return: 最终确定的文件路径 (可能不存在, 由调用方处理)
    """
    # 检查环境变量
    env_path = os.environ.get('HKU_real_tdx_file_path')
    if env_path and os.path.exists(env_path):
        print(f"get_real_tdx_filepath return: {env_path}")
        return env_path

    market_subdir = f"{market.lower()}/lday"
    filename = f"{market.lower()}{code}.day"

    # 检查配置文件指定的路径
    base_path = os.path.join(_tdx_dir, 'vipdoc')
    if os.path.exists(base_path):
        primary_path = os.path.join(base_path, market_subdir, filename)
        if os.path.exists(primary_path):
            print(f"get_real_tdx_filepath return: {primary_path}")
            return primary_path

    # 检查 test_data/vipdoc/
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
    test_data_vipdoc_path = os.path.join(project_root, 'test_data', 'vipdoc')
    if os.path.exists(test_data_vipdoc_path):
        primary_path = os.path.join(test_data_vipdoc_path, market_subdir, filename)
        if os.path.exists(primary_path):
            print(f"get_real_tdx_filepath return: {primary_path}")
            return primary_path

    # 如果主路径文件不存在, 则尝试从 test_data 目录寻找同名文件作为备用
    test_data_dir = os.path.join(project_root, 'test_data')
    fallback_path = os.path.join(test_data_dir, filename)
    print(f"get_real_tdx_filepath return: {fallback_path}")
    return fallback_path
