#!/usr/bin/env python
#-*- coding:utf-8 -*-

from setuptools import setup, find_packages

setup(
        name = "Hikyuu",
        version="0.1.0",
        packages = find_packages(),
        zip_safe = False,

        description = "egg test demo.",
        long_description = "egg test demo, haha.",
        author = "fasiondog",
        author_email = "fasiondog@sina.com",

        license = "GPL",
        keywords = ("hikyuu", "egg"),
        platforms = "Independant",
        url = "",
        
        data_files = [('Lib/site-packages/hikyuu', ['./_hikyuu.pyd']),
                      ('Lib/site-packages/hikyuu/indicator', ['./indicator/_indicator.pyd']),
                      ('Lib/site-packages/hikyuu/trade_instance', ['./trade_instance/_trade_instance.pyd']),
                      ('Lib/site-packages/hikyuu/trade_manage', ['./trade_manage/_trade_manage.pyd']),
                      ('Lib/site-packages/hikyuu/trade_sys', ['./trade_sys/_trade_sys.pyd']),
                      ('DLLs', ['../../bin/hikyuu.dll']),
                      ('DLLs', ['../../bin/hikyuu_utils.dll']),
                      ('DLLs', ['../../bin/boost_date_time-vc100-mt-1_56.dll']),
                      ('DLLs', ['../../bin/boost_filesystem-vc100-mt-1_56.dll']),
                      ('DLLs', ['../../bin/boost_serialization-vc100-mt-1_56.dll']),
                      ('DLLs', ['../../bin/boost_thread-vc100-mt-1_56.dll']),
                      ('DLLs', ['../../extern-libs/log4cplus/dll/log4cplus.dll']),
                      ('DLLs', ['../../extern-libs/mysql/lib/libmysql.dll']),
                      ('DLLs', ['../../extern-libs/hdf5/dll/hdf5dll.dll']),
                      ('DLLs', ['../../extern-libs/hdf5/dll/hdf5_hldll.dll']),
                      ('DLLs', ['../../extern-libs/hdf5/dll/hdf5_cppdll.dll']),
                      ('DLLs', ['../../extern-libs/hdf5/dll/hdf5_hl_cppdll.dll']),
                      ('DLLs', ['../../extern-libs/hdf5/dll/szip.dll']),
                      ('DLLs', ['../../extern-libs/hdf5/dll/zlib.dll'])]
        )