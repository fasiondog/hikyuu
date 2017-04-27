#!/usr/bin/env python
#-*- coding:utf-8 -*-

from setuptools import setup, find_packages
import sys

hku_name = "Hikyuu"
hku_version = "1.0.0"
hku_author = "fasiondog"
hku_author_email = "fasiondog@sina.com"

hku_license = "PSF"
hku_keywords = ("hikyuu", "quant", "trade", "System Trading")
hku_platforms = "Independant"
hku_url = "http://hikyuu.org/"

hku_description = "Hikyuu System Trading Analysis Tools"
hku_long_description = ""

if sys.platform == 'win32':
    hku_data_files = [('Lib/site-packages/hikyuu', ['../bin/hikyuu.dll']),
                      ('Lib/site-packages/hikyuu', ['../bin/hikyuu_utils.dll']),
                      ('Lib/site-packages/hikyuu', ['../bin/sqlite3.dll']),
                      ('Lib/site-packages/hikyuu', ['../bin/importdata.exe']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_chrono-vc100-mt-1_56.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_date_time-vc100-mt-1_56.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_filesystem-vc100-mt-1_56.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_python-vc100-mt-1_56.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_serialization-vc100-mt-1_56.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_system-vc100-mt-1_56.dll']),
                      #('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_thread-vc100-mt-1_56.dll']),
                      #('Lib/site-packages/hikyuu', ['../extern-libs/boost/boost_unit_test_framework-vc100-mt-1_56.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/log4cplus/dll/log4cplus.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/mysql/lib/libmysql.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/hdf5/dll/hdf5dll.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/hdf5/dll/hdf5_hldll.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/hdf5/dll/hdf5_cppdll.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/hdf5/dll/hdf5_hl_cppdll.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/hdf5/dll/szip.dll']),
                      ('Lib/site-packages/hikyuu', ['../extern-libs/hdf5/dll/zlib.dll'])]
else:
    hku_data_files = []

setup(
        name = hku_name,
        version = hku_version,
        description = hku_description,
        long_description = hku_long_description,
        author = hku_author,
        author_email = hku_author_email,

        license = hku_license,
        keywords = hku_keywords,
        platforms = hku_platforms,
        url = hku_url,

        packages = find_packages(),
        zip_safe = False,
        include_package_data=True,
        package_data = {
            '': ['*.rst', '*.pyd', '*.ini', '*.sql', '*.properties', '*.xml'],

        },
        
        data_files = hku_data_files
        )