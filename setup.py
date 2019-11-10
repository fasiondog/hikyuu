#!/usr/bin/env python
#-*- coding:utf-8 -*-

from setuptools import setup, find_packages
import sys
import os
import shutil
import platform
import click


#------------------------------------------------------------------------------
# 前置检查
#------------------------------------------------------------------------------
def check_xmake():
    """检查是否按照了编译工具 xmake"""
    print("checking xmake ...")
    xmake = os.system("xmake --version")
    return False if xmake != 0 else True


def get_boost_envrionment():
    """ 
    获取 BOOST 环境变量设置
        @return (current_boost_root, current_boost_lib)
    """
    current_dir = os.getcwd()
    current_boost_root = None
    current_boost_lib = None
    if 'BOOST_ROOT' in os.environ:
        current_boost_root = os.environ['BOOST_ROOT']
        if 'BOOST_LIB' in os.environ:
            current_boost_lib = os.environ['BOOST_LIB']
        else:
            current_boost_lib = current_boost_root + '/stage/lib'
            os.environ['BOOST_LIB'] = current_boost_lib
    else:
        for dir in os.listdir():
            if len(dir) >= 5 and dir[:5] == 'boost' and os.path.isdir(dir):
                current_boost_root = current_dir + '/' + dir
                current_boost_lib = current_dir + '/' + dir + '/stage/lib'
                os.environ['BOOST_ROOT'] = current_boost_root
                os.environ['BOOST_LIB'] = current_boost_lib
    return (current_boost_root, current_boost_lib)


def get_python_version():
    """
    获取 python版本，并判断当前 python 版本是否发生了变化
        @return (py_version, py_version_changed)
    """
    py_version = platform.python_version_tuple()
    py_version = int(py_version[0])*10 + int(py_version[1])
    py_version_changed = True
    if os.path.exists('py_version'):
        with open('py_version', 'r+') as f:
            old_py_version = f.read()
            if old_py_version == str(py_version):
                py_version_changed = False
            else:
                f.seek(0)
                f.write(str(py_version))
                print('old python version:', int(old_py_version)*0.1)
    else:
        with open('py_version', 'w') as f:
            f.write(str(py_version))
    print('current python version:', int(py_version)*0.1, '\n')
    return (py_version, py_version_changed)


def build_boost():
    """ 编译依赖的 boost 库 """
    current_boost_root, current_boost_lib = get_boost_envrionment()
    if current_boost_root is None or current_boost_lib is None:
        print("Can't get boost environment!")
        return
    current_dir = os.getcwd()
    if sys.platform == 'win32':
        os.chdir(current_boost_root)
        if not os.path.exists('b2.exe'):
            os.system('bootstrap.bat')
        os.system('b2 release link=static runtime-link=shared address-model=64 -j 4 --with-date_time'
                  ' --with-filesystem --with-system --with-test')
        os.system('b2 release link=shared runtime-link=shared address-model=64 -j 4 --with-python'
                  ' --with-serialization')
        os.chdir(current_dir)
    else:
        cmd = 'cd {boost} ; if [ ! -f "b2" ]; then ./bootstrap.sh ; fi; '\
              './b2 release link=shared address-model=64 -j 4 --with-python --with-serialization; '\
              './b2 release link=static address-model=64 cxxflags=-fPIC -j 4 --with-date_time '\
              '--with-filesystem --with-system --with-test; '\
              'cd {current}'.format(boost=current_boost_root, current=current_dir)
        os.system(cmd)


def clear_with_python_changed():
    """
    python版本发生变化时，清理之前的python编译结果
    应该仅在 pyhon 版本发生变化时被调用
    """
    current_plat = sys.platform
    current_bits = 64 if sys.maxsize > 2**32 else 32
    if current_plat == 'win32' and current_bits == 64:
        build_pywrap_dir = 'build\\release\\windows\\x64\\.objs\\windows\\x64\\release\\hikyuu_pywrap'
    elif current_plat == 'win32' and current_bits == 32:
        build_pywrap_dir = 'build\\release\\windows\\x86\\.objs\\windows\\x64\\release\\hikyuu_pywrap'
    elif current_plat == 'linux' and current_bits == 64:
        build_pywrap_dir = 'build/release/linux/x86_64/.objs/linux/x86_64/release/hikyuu_pywrap'
    elif current_plat == "darwin" and current_bits == 64:
        build_pywrap_dir = 'build/release/macosx/x86_64/.objs/macosx/x86_64/release/hikyuu_pywrap'
    else:
        print("************不支持的平台**************")
        exit(0)
    if os.path.lexists(build_pywrap_dir):
        shutil.rmtree(build_pywrap_dir)
    current_boost_root, _ = get_boost_envrionment()
    if os.path.lexists('{}/bin.v2/libs/python'.format(current_boost_root)):
        shutil.rmtree('{}/bin.v2/libs/python'.format(current_boost_root))


#------------------------------------------------------------------------------
# 执行构建
#------------------------------------------------------------------------------
def start_build(verbose=False):
    """ 执行编译 """
    global g_verbose
    g_verbose = verbose
    if not check_xmake():
        print("Please install xmake")
        return

    print("\nchecking python version ...")
    py_version, py_version_changed = get_python_version()
    if py_version < 31:
        print("Python version must >= 3.1 !")
        return

    print("checking boost ...")
    current_boost_root, current_boost_lib = get_boost_envrionment()
    if current_boost_root is None or current_boost_lib is None:
        print("Please configure BOOST")
        exit(0)
    print('BOOST_ROOT:', current_boost_root)
    print('BOOST_LIB:', current_boost_lib)

    #如果 python 发生变化，则编译依赖的 boost 库（boost.python)
    if py_version_changed:
        clear_with_python_changed()
        print('\ncompile boost ...')
        build_boost()

    if py_version_changed:
        os.system("xmake f -c -y")
    else:
        os.system("xmake f -y")
    os.system("xmake -b {} hikyuu".format("-v -D" if verbose else ""))
    os.system("xmake -b {} _hikyuu".format("-v -D" if verbose else ""))
    os.system("xmake -b {} _indicator".format("-v -D" if verbose else ""))
    os.system("xmake -b {} _trade_manage".format("-v -D" if verbose else ""))
    os.system("xmake -b {} _trade_sys".format("-v -D" if verbose else ""))
    os.system("xmake -b {} _trade_instance".format("-v -D" if verbose else ""))
    os.system("xmake -b {} _data_driver".format("-v -D" if verbose else ""))


#------------------------------------------------------------------------------
# 控制台命令
#------------------------------------------------------------------------------

@click.group()
def cli():
    pass


@click.command()
@click.option('-v', '--verbose', is_flag=True, help='显示详细的编译信息')
def build(verbose):
    """ 执行编译 """
    start_build(verbose)


@click.command()
@click.option('-all', "--all", is_flag=True, help="执行全部测试, 否则仅仅进行最小范围测试）")
@click.option("-compile", "--compile", is_flag=True, help='强制重新编译')
@click.option('-v', '--verbose', is_flag=True, help='显示详细的编译信息')
def test(all, compile, verbose):
    """ 执行单元测试 """
    # 先取 BOOST 路径，避免为设置 BOOST_LIB 的情况
    current_boost_root, current_boost_lib = get_boost_envrionment()
    if compile:
        start_build(verbose)
    if all:
        os.system("xmake f --test=all")
        os.system("xmake -b unit-test")
        os.system("xmake r unit-test")
    else:
        os.system("xmake f --test=small")
        os.system("xmake -b small-test")
        os.system("xmake r small-test")


@click.command()    
@click.option("-with_boost", "--with_boost", is_flag=True, help='清除相应的BOOST库')
def clear(with_boost):
    """ 清除当前编译设置及结果 """
    if os.path.lexists('.xmake'):
        print('delete .xmake')
        shutil.rmtree('.xmake')
    if os.path.lexists('build'):
        print('delete build')
        shutil.rmtree('build')
    if os.path.lexists('Hikyuu.egg-info'):
        print('delete Hikyuu.egg-info')
        shutil.rmtree('Hikyuu.egg-info')
    if with_boost and os.path.exists('py_version'):
        print('delete py_version')
        os.remove('py_version')
    for r, _, f_list in os.walk('hikyuu'):
        for name in f_list:
            if (name != 'UnRAR.exe' and len(name) > 4 and name[-4:] in ('.dll','.exe','.pyd')) \
                   or (len(name) > 8 and name[:9] == 'libboost_')  \
                   or (len(name) > 6 and name[-6:] == '.dylib'):
                print('delete', r + '/' + name)
                os.remove(os.path.join(r, name))
    print('clear finished!')


@click.command()
def uninstall():
    """ 卸载已安装的 python 包 """
    if sys.platform == 'win32':
        site_lib_dir = sys.base_prefix + "/lib/site-packages"
    else:
        usr_dir = os.path.expanduser('~')
        py_version, _ = get_python_version()
        site_lib_dir = '{}/.local/lib/python{:>.1f}/site-packages'.format(usr_dir, py_version*0.1)
    for dir in os.listdir(site_lib_dir):
        if dir == 'hikyuu' or (len(dir) > 6 and dir[:6] == 'Hikyuu'):
            print('delete', site_lib_dir + '/' + dir)
            shutil.rmtree(site_lib_dir + '/' + dir)
    print("Uninstall finished!")


@click.command()
def install():
    """ 编译并安装 Hikyuu python 库 """
    start_build()
    py_version, py_version_changed = get_python_version()
    if py_version_changed:
        os.system("xmake f -c -y")
    if sys.platform == 'win32':
        install_dir = sys.base_prefix + "\\Lib\\site-packages\\Hikyuu"
    else:
        usr_dir = os.path.expanduser('~')
        install_dir = '{}/.local/lib/python{:>.1f}/site-packages/Hikyuu'.format(usr_dir, py_version*0.1)
        try:
            shutil.rmtree(install_dir)
        except:
            pass
        os.makedirs(install_dir)
    os.system('xmake install -o "{}"'.format(install_dir))


@click.command()
def wheel():
    """ 生成 python 的 wheel 安装包 """
    # 尝试编译
    start_build()

    # 清理之前遗留的打包产物
    print("Clean up the before papackaging outputs ...")
    py_version, _ = get_python_version()
    if os.path.lexists('Hikyuu.egg-info'):
        shutil.rmtree('Hikyuu.egg-info')
    if os.path.lexists('build/lib'):
        shutil.rmtree('build/lib')
    if os.path.lexists('build'):
        for bdist in os.listdir('build'):
            if len(bdist) >= 5 and bdist[:5] == 'bdist' and os.path.lexists(bdist):
                shutil.rmtree(bdist)
    for x in os.listdir('hikyuu'):
        if x[:12] == 'boost_python':
            if x[12:14] != str(py_version):
                os.remove('hikyuu/{}'.format(x))

    # 构建打包命令
    print("start pacakaging bdist_wheel ...")
    current_plat = sys.platform
    current_bits = 64 if sys.maxsize > 2**32 else 32
    if current_plat == 'win32' and current_bits == 64:
        plat = "win-amd64"
    elif current_plat == 'win32' and current_bits == 32:
        plat = "win32"
    elif current_plat == 'linux' and current_bits == 64:
        plat = "manylinux1_x86_64"
    elif current_plat == 'linux' and current_bits == 32:
        plat = "manylinux1_i386"
    else:
        print("*********尚未实现该平台的支持*******")
        return
    cmd = 'python sub_setup.py bdist_wheel --python-tag cp{} -p {}'.format(py_version, plat)
    os.system(cmd)


@click.command()
def upload():
    """ 发布上传至 pypi，仅供发布者使用！！！ """
    if not os.path.lexists('dist'):
        print("Not found wheel package! Pleae wheel first")
        return
    print("current wheel:")
    for bdist in os.listdir('dist'):
        print(bdist)
    print("")
    val = input('Are you sure upload now (y/n)? (deault: n) ')
    if val == 'y':
        os.system("twine upload dist/*")


#------------------------------------------------------------------------------
# 添加 click 命令
#------------------------------------------------------------------------------
cli.add_command(build)
cli.add_command(test)
cli.add_command(clear)
cli.add_command(install)
cli.add_command(uninstall)
cli.add_command(wheel)
cli.add_command(upload)


if __name__ == "__main__":
    cli()
