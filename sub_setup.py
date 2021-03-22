#!/usr/bin/env python
#-*- coding:utf-8 -*-

import platform
from setuptools import setup, find_packages

hku_version = ''
with open('xmake.lua', 'r', encoding='utf-8') as f:
    for line in f:
        if len(line) > 15 and line[:11] == 'set_version':
            pre_pos = line.find('"') + 1
            end_pos = line.find('"', pre_pos)
            hku_version = line[pre_pos:end_pos]
            break

if not hku_version:
    print("无法在 xmake.lua 中找到 set_version 语句，获取版本号失败！")
    exit(0)

print('current hikyuu version:', hku_version)

py_version = platform.python_version_tuple()
py_version = int(py_version[0]) * 10 + int(py_version[1])

hku_name = "hikyuu"
#hku_version = "1.0.9"
hku_author = "fasiondog"
hku_author_email = "fasiondog@sina.com"

hku_license = "MIT"
hku_keywords = [
    "quant", "trade", "System Trading", "backtester", "量化", "程序化交易", "量化交易",
    "系统交易"
]
hku_platforms = "Independant"
hku_url = "http://hikyuu.org/"

hku_description = "Hikyuu Quant Framework for System Trading Analysis and backtester"
with open("README.rst", encoding='utf-8') as f:
    hku_long_description = f.read()

hku_data_files = []

packages = [
    'hikyuu',
    'hikyuu/config',
    'hikyuu/config/block',
    'hikyuu/cpp',
    'hikyuu/data',
    'hikyuu/data/mysql_upgrade',
    'hikyuu/data/sqlite_upgrade',
    'hikyuu/data/sqlite_mem_sql',
    'hikyuu/data_driver',
    'hikyuu/examples',
    'hikyuu/flat',
    'hikyuu/fetcher',
    'hikyuu/fetcher/proxy',
    'hikyuu/fetcher/stock',
    'hikyuu/gui',
    'hikyuu/gui/data',
    'hikyuu/indicator',
    'hikyuu/draw',
    'hikyuu/draw/drawplot',
    'hikyuu/shell',
    'hikyuu/strategy',
    'hikyuu/strategy/demo',
    #'hikyuu/test',
    'hikyuu/tools',
    'hikyuu/trade_manage',
    'hikyuu/trade_sys',
    'hikyuu/util',
]

setup(
    name=hku_name,
    version=hku_version,
    description=hku_description,
    long_description=hku_long_description,
    author=hku_author,
    author_email=hku_author_email,
    license=hku_license,
    keywords=hku_keywords,
    platforms=hku_platforms,
    url=hku_url,
    packages=packages,  #find_packages(),
    zip_safe=False,
    include_package_data=True,
    package_data={
        '': [
            '*.rst', '*.pyd', '*.ini', '*.sql', '*.properties', '*.xml',
            'LICENSE.txt', '*.dll', '*.exe', '*.ico', '*.so', '*.dylib',
            'libboost_serialization*', 'libboost_python{}*'.format(py_version)
        ],
    },
    data_files=hku_data_files,
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
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
    ],
    entry_points={
        'gui_scripts': [
            'HikyuuTDX=hikyuu.gui.HikyuuTDX:start',
        ],
        'console_scripts': [
            'importdata=hikyuu.gui.importdata:main',
        ]
    },
    install_requires=[
        'matplotlib',
        'pandas>=0.17.1',
        #'tushare>=0.8.2',
        'pytdx',
        'PyQt5',
        'tables',
        'bokeh',
        'gitpython',
        'SQLAlchemy',
        'mysql-connector-python',
        'pyperclip',
        'requests',
        'qdarkstyle',
        'flatbuffers',
        'pynng',
        'requests'
    ],
)