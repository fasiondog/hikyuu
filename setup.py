#!/usr/bin/env python
#-*- coding:utf-8 -*-

from setuptools import setup, find_packages
import sys
import os
import shutil
import platform

print("checking xmake ...")
xmake = os.system("xmake --version")

if xmake != 0 or 'BOOST_ROOT' not in os.environ or 'BOOST_LIB' not in os.environ:
    print("Please read https://hikyuu.readthedocs.io/en/latest/developer.html")

if len(sys.argv) == 1:
    os.system("xmake")
    install_dir = sys.base_prefix + "\\lib\\site-packages\\hikyuu"
    os.system("xmake install -o " + install_dir)
    sys.exit(0)
    
os.system("xmake")
os.system("xmake install -o hikyuu")

py_version = platform.python_version_tuple() 
py_version = int(py_version[0])*10 + int(py_version[1])
if py_version < 35:
    print("Python version must > 3.5 !")
    sys.exit(0)

if sys.argv[-1] == 'bdist_wheel':
    sys.argv.append("--python-tag")
    sys.argv.append("cp{}".format(py_version))
    sys.argv.append("-p")
    sys.argv.append("win-amd64")
    
shutil.rmtree('build/lib')
shutil.rmtree('build/bdist.win-amd64')
#shutil.rmtree('dist')
for x in os.listdir('hikyuu'):
    if x[:12] == 'boost_python':
        if x[12:14] != str(py_version):
            os.remove('hikyuu/{}'.format(x))

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

hku_data_files = []

packages = ['hikyuu',
            'hikyuu/config',
            'hikyuu/data_driver',
            'hikyuu/examples',
            'hikyuu/indicator',
            'hikyuu/interactive',
            'hikyuu/interactive/drawplot',
            #'hikyuu/test',
            #'hikyuu/tools',
            'hikyuu/trade_instance',
            'hikyuu/trade_manage',
            'hikyuu/trade_sys',
            'hikyuu/util',
           ]

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

        packages = packages, #find_packages(),
        zip_safe = False,
        include_package_data=True,
        package_data = {
            '': ['*.rst', '*.pyd', '*.ini', '*.sql', '*.properties', '*.xml', 'LICENSE.txt', '*.dll', '*.exe'],
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
            'pandas>=0.17.1',
            #'tushare>=0.8.2',
            'pytdx',
            'pyperclip>=1.5.27',
            'pyecharts>=0.2.7',
            'pyecharts_snapshot'
        ],        
        )
