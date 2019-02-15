#!/usr/bin/env python
#-*- coding:utf-8 -*-

from setuptools import setup, find_packages
import sys
import os
import shutil
import platform

print("checking xmake ...")
xmake = os.system("xmake --version")

if xmake != 0: # or 'BOOST_ROOT' not in os.environ or 'BOOST_LIB' not in os.environ:
    print("Please read https://hikyuu.readthedocs.io/en/latest/developer.html")
    exit(0)
    
current_boost_root = None
current_boost_lib = None
if 'BOOST_ROOT' not in os.environ or 'BOOST_LIB' not in os.environ:
    for dir in os.listdir():
        if len(dir) >= 5 and dir[:5] == 'boost' and os.path.isdir(dir):
            current_boost_root = dir
            current_boost_lib = dir + '/stage/lib'
            os.environ['BOOST_ROOT'] = current_boost_root
            os.environ['BOOST_LIB'] = current_boost_lib
else:
    current_boost_root = os.environ['BOOST_ROOT']
    current_boost_lib = os.environ['BOOST_LIB']
    
if current_boost_root is None or current_boost_lib is None:
    print("Please read https://hikyuu.readthedocs.io/en/latest/developer.html")
    exit(0)

py_version = platform.python_version_tuple() 
py_version = int(py_version[0])*10 + int(py_version[1])
if py_version < 31:
    print("Python version must >= 3.1 !")
    sys.exit(0)

current_plat = sys.platform
current_bits = 64 if sys.maxsize > 2**32 else 32

py_version_changed = True
if os.path.exists('py_version'):
    with open('py_version', 'r+') as f:
        old_py_version = f.read()
        if old_py_version == str(py_version):
            py_version_changed = False
        else:
            f.seek(0)
            f.write(str(py_version))
else:
    with open('py_version', 'w') as f:
        f.write(str(py_version))

print('py_version_changed', py_version_changed)
if py_version_changed:
    if current_plat == 'win32' and current_bits == 64:
        build_hikyuu_pywrap_dir = 'build\\release\\windows\\x64\\.objs\\windows\\x64\\release\\hikyuu_pywrap'
    elif current_plat == 'win32' and current_bits == 32:
        build_hikyuu_pywrap_dir = 'build\\release\\windows\\x64\\.objs\\windows\\x64\\release\\hikyuu_pywrap'
    elif current_plat == 'linux' and current_bits == 64:
        build_hikyuu_pywrap_dir = 'build/release/linux/x64/.objs/windows/x64/release/hikyuu_pywrap'
    else:
        print("************不支持的平台打包**************")
        exit(0)
        
    if os.path.lexists(build_hikyuu_pywrap_dir):
        shutil.rmtree(build_hikyuu_pywrap_dir)
    
    current_dir = os.getcwd()
    os.chdir(current_boost_root)
    if os.path.lexists('bin.v2/libs/python'):
        shutil.rmtree('bin.v2/libs/python')
    if not os.path.exists('b2.exe'):
        os.system('bootstrap.bat')
    os.system('b2 release link=shared address-model=64 -j 4 --with-python --with-date_time --with-filesystem --with-system --with-serialization --with-test')
    
    os.chdir(current_dir)
    

if len(sys.argv) == 1:
    os.system("xmake f -c")
    #os.system("xmake f --with-unit-test=y")
    install_dir = sys.base_prefix + "/lib/site-packages/hikyuu"
    os.system("xmake install -o " + install_dir)
    sys.exit(0)

if sys.argv[-1] == 'bdist_wheel':
    os.system("xmake f -c")
    os.system("xmake install -o hikyuu")
    sys.argv.append("--python-tag")
    sys.argv.append("cp{}".format(py_version))
    sys.argv.append("-p")
    if current_plat == 'win32' and current_bits == 64:
        sys.argv.append("win-amd64")
    elif current_plat == 'win32' and current_bits == 32:
        sys.argv.append("win32")
    elif current_plat == 'linux' and current_bits == 64:
        sys.argv.append("manylinux1_x86_64")
    elif current_plat == 'linux' and current_bits == 32:
        sys.argv.append("manylinux1_i386")
    else:
        print("*********尚未实现*******")
        
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

if os.path.exists('hikyuu'):
    shutil.rmtree('hikyuu')