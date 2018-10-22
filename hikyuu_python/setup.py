#!/usr/bin/env python
#-*- coding:utf-8 -*-

from setuptools import setup, find_packages
import sys
import os

if sys.argv[-1] == 'publish':
    os.system("python setup.py bdist_wheel --python-tag cp35 -p win-amd64")
    #python setup.py bdist_wheel --python-tag cp35 -p linux_x86_64
    os.system("twine upload dist/*")
    sys.exit()

hku_name = "Hikyuu"
hku_version = "1.0.9"
hku_author = "fasiondog"
hku_author_email = "fasiondog@sina.com"

hku_license = "MIT"
hku_keywords = ("quant", "trade", "System Trading", "backtester", "量化", "程序化交易", "量化交易", "系统交易")
hku_platforms = "Independant"
hku_url = "http://hikyuu.org/"

hku_description = "Hikyuu Quant Framework for System Trading Analysis and backtester"
with open("README.rst", encoding='utf-8') as f:
    hku_long_description = f.read()

if sys.platform == 'win32':
    hku_data_files = [('Lib/site-packages/hikyuu', ['hikyuu/hikyuu.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/hikyuu_utils.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/sqlite3.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/importdata.exe']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/boost_date_time-vc141-mt-x64-1_68.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/boost_filesystem-vc141-mt-x64-1_68.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/boost_python35-vc141-mt-x64-1_68.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/boost_serialization-vc141-mt-x64-1_68.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/boost_system-vc141-mt-x64-1_68.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/libmysql.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/hdf5.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/hdf5_hl.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/hdf5_cpp.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/hdf5_hl_cpp.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/szip.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/zlib.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/concrt140.dll']),
                      ('Lib/site-packages/hikyuu', ['hikyuu/vcruntime140.dll'])]
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
            '': ['*.rst', '*.pyd', '*.ini', '*.sql', '*.properties', '*.xml', 'LICENSE.txt', 'lib*.so.*'],
        },
        
        data_files = hku_data_files,
        
        classifiers=[
            # How mature is this project? Common values are
            #   3 - Alpha
            #   4 - Beta
            #   5 - Production/Stable
            'Development Status :: 3 - Alpha',

            # Indicate who your project is intended for
            'Intended Audience :: Developers',
            'Topic :: Software Development :: Libraries :: Python Modules',
            'Topic :: Office/Business :: Financial',
            'Topic :: Office/Business :: Financial :: Investment',
            'Topic :: Scientific/Engineering :: Mathematics',

            # Pick your license as you wish (should match "license" above)
             'License :: OSI Approved :: MIT License',

            'Operating System :: Microsoft :: Windows',
             
            # Specify the Python versions you support here. In particular, ensure
            # that you indicate whether you support Python 2, Python 3 or both.
            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3.5',
        ],

        install_requires=[
            'matplotlib>=1.5.0',
            #'pandas>=0.17.1',
            #'tushare>=0.8.2',
            #'pyperclip>=1.5.27',
            'pyecharts>=0.2.7',
        ],        
        )