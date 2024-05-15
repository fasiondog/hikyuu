#!/usr/bin/env python
# -*- coding:utf-8 -*-

import platform
import os
try:
    from setuptools import find_packages, setup
except ImportError:
    from distutils.core import find_packages, setup


def parse_requirements(filename):
    line_iter = (line.strip() for line in open(filename))
    return [line for line in line_iter if line and not line.startswith('#')]


requirements = parse_requirements('requirements.txt')

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
# hku_version = "1.0.9"
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
with open("./hikyuu/README.rst", encoding='utf-8') as f:
    hku_long_description = f.read()

hku_data_files = []

# packages = [
#     'hikyuu',
#     'hikyuu/analysis',
#     'hikyuu/config',
#     'hikyuu/config/block',
#     'hikyuu/cpp',
#     'hikyuu/data',
#     'hikyuu/data/mysql_upgrade',
#     'hikyuu/data/sqlite_upgrade',
#     'hikyuu/data/sqlite_mem_sql',
#     'hikyuu/data_driver',
#     'hikyuu/examples',
#     'hikyuu/examples/notebook',
#     'hikyuu/examples/notebook/images',
#     'hikyuu/examples/notebook/Demo',
#     'hikyuu/flat',
#     'hikyuu/fetcher',
#     'hikyuu/fetcher/proxy',
#     'hikyuu/fetcher/stock',
#     'hikyuu/gui',
#     'hikyuu/gui/data',
#     'hikyuu/indicator',
#     'hikyuu/draw',
#     'hikyuu/draw/drawplot',
#     'hikyuu/shell',
#     'hikyuu/strategy',
#     'hikyuu/strategy/demo',
#     'hikyuu/test',
#     'hikyuu/tools',
#     'hikyuu/trade_manage',
#     'hikyuu/trade_sys',
#     'hikyuu/util',
#     'hikyuu/include',
# ]

packages = ['hikyuu']
for root, dirs, files in os.walk('hikyuu'):
    for p in dirs:
        if p.find('__pycache__') < 0 and p.find('ipynb_checkpoints') < 0 \
                and p.find('virtual_documents') < 0 and p.find('idea') < 0 and p.find('venv') < 0:
            packages.append(f'{root}/{p}')

setup(
    name=hku_name,
    version=hku_version,
    description=hku_description,
    long_description_content_type="text/x-rst",
    long_description=hku_long_description,
    author=hku_author,
    author_email=hku_author_email,
    license=hku_license,
    keywords=hku_keywords,
    platforms=hku_platforms,
    url=hku_url,
    packages=packages,  # find_packages(),
    zip_safe=False,
    include_package_data=True,
    package_data={
        '': [
            '*.rst', '*.pyd', '*.png', '*.md', '*.ipynb', '*.ini', '*.sql', '*.properties', '*.xml',
            'LICENSE.txt', '*.dll', '*.exe', '*.ico', '*.so', '*.dylib', '*.h',
            '*.so.*', '*.qm', 'libboost_serialization*',
            'libboost_python{}*'.format(py_version)
        ],
    },
    data_files=hku_data_files,
    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 4 - Beta',

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
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
    ],
    entry_points={
        # win11下使用 GUI 方式，会立刻 timeout，导致无法下载
        # 'gui_scripts': [
        #     'HikyuuTDX=hikyuu.gui.HikyuuTDX:start',
        # ],
        'console_scripts': [
            'HikyuuTDX=hikyuu.gui.HikyuuTDX:start',
            'importdata=hikyuu.gui.importdata:main',
        ]
    },
    install_requires=requirements,
)
