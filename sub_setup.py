#!/usr/bin/env python
# -*- coding:utf-8 -*-

import platform
import os
import sys
try:
    from setuptools import find_packages, setup
except ImportError:
    from distutils.core import find_packages, setup


def parse_requirements(filename):
    line_iter = (line.strip() for line in open(filename))
    return [line for line in line_iter if line and not line.startswith('#')]


requirements = parse_requirements('requirements.txt')

current_plat = sys.platform
# Remove the special handling of PyQt5
# if current_plat == 'linux':
#     requirements.remove('PyQt5')

hku_version = ''
with open('xmake.lua', 'r', encoding='utf-8') as f:
    for line in f:
        if len(line) > 15 and line[:11] == 'set_version':
            pre_pos = line.find('"') + 1
            end_pos = line.find('"', pre_pos)
            hku_version = line[pre_pos:end_pos]
            break

if not hku_version:
    print("Cannot find the set_version statement in xmake.lua, failed to get the version number!")
    exit(0)

print('current hikyuu version:', hku_version)

py_version = platform.python_version_tuple()
py_version = int(py_version[0]) * 10 + int(py_version[1])

hku_name = "hikyuu"  # "hikyuu-noarrow"
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
with open("./readme.md", encoding='utf-8') as f:
    hku_long_description = f.read()

hku_data_files = []

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
    # long_description_content_type="text/x-rst",
    long_description_content_type='text/markdown',
    long_description=hku_long_description,
    author=hku_author,
    author_email=hku_author_email,
    license=hku_license,
    license_files=['LICENSE.txt'],
    keywords=hku_keywords,
    platforms=hku_platforms,
    url=hku_url,
    packages=packages,  # find_packages(),
    zip_safe=False,
    include_package_data=True,
    package_data={
        '': [
            '*.rst', '*.pyd', '*.png', '*.md', '*.ipynb', '*.ini', '*.sql', '*.properties', '*.xml',
            'LICENSE.txt', '*.dll', '*.exe', '*.ico', '*.so', '*.dylib', '*.h', '*.lib', '*.mo',
            '*.so.*', '*.qm', 'libboost_serialization*', 'libboost_python{}*'.format(py_version),
        ],
    },
    data_files=hku_data_files,
    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 5 - Production/Stable',

        # Indicate who your project is intended for
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Office/Business :: Financial',
        'Topic :: Office/Business :: Financial :: Investment',
        'Topic :: Scientific/Engineering :: Mathematics',

        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: MIT License',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX :: Linux',
        'Operating System :: MacOS :: MacOS X',

        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Programming Language :: Python :: 3.13',
    ],
    entry_points={
        # On win11, using the GUI mode times out immediately, so the download fails
        # 'gui_scripts': [
        #     'HikyuuTDX=hikyuu.gui.HikyuuTDX:start',
        # ],
        'console_scripts': [
            'HikyuuTDX=hikyuu.gui.HikyuuTDX:start',
            'importdata=hikyuu.gui.importdata:main',
            'dataserver=hikyuu.gui.dataserver:main',
            'shmserver=hikyuu.gui.shmserver:main',
        ]
    },
    install_requires=requirements,
)
