#!/usr/bin/env python
# -*- coding: utf8 -*-
# cp936
#
# ===============================================================================
# History
# 1. 20200816, Added by fasiondog
# ===============================================================================

from sqlalchemy.orm import sessionmaker, scoped_session, declarative_base
from sqlalchemy import (create_engine, Sequence, Column, Integer, String, and_, UniqueConstraint)
from hikyuu.util.singleton import SingletonType
from hikyuu.util.check import checkif
import os
import stat
import errno
import sys
import shutil
import pathlib
import logging
import importlib
from configparser import ConfigParser

# 引入 git 前需设置环境变量，否则某些情况下会报错失败
os.environ['GIT_PYTHON_REFRESH'] = 'quiet'
try:
    import git
except Exception as e:
    print(e)
    print("You need install git! see: https://git-scm.com/downloads")


Base = declarative_base()


class ConfigModel(Base):
    __tablename__ = 'hub_config'
    id = Column(Integer, Sequence('config_id_seq'), primary_key=True)
    key = Column(String, index=True)  # 参数名
    value = Column(String)  # 参数值

    __table_args__ = (UniqueConstraint('key'), )

    def __str__(self):
        return "ConfigModel(id={}, key={}, value={})".format(self.id, self.key, self.value)

    def __repr__(self):
        return "<{}>".format(self.__str__())


class HubModel(Base):
    __tablename__ = 'hub_repo'
    id = Column(Integer, Sequence('remote_id_seq'), primary_key=True)
    name = Column(String, index=True)  # 本地仓库名
    hub_type = Column(String)  # 'remote' (远程仓库) | 'local' （本地仓库）
    local_base = Column(String)  # 本地路径的基础名称
    local = Column(String)  # 本地路径
    url = Column(String)  # git 仓库地址
    branch = Column(String)  # 远程仓库分支

    __table_args__ = (UniqueConstraint('name'), )

    def __str__(self):
        return "HubModel(id={}, name={}, hub_type={}, local={}, url={}, branch={})".format(
            self.id, self.name, self.hub_type, self.local, self.url, self.branch
        )

    def __repr__(self):
        return "<{}>".format(self.__str__())


class PartModel(Base):
    __tablename__ = 'hub_part'
    id = Column(Integer, Sequence('part_id_seq'), primary_key=True)
    hub_name = Column(String)  # 所属仓库标识
    part = Column(String)  # 部件类型
    name = Column(String)  # 策略名称
    author = Column(String)  # 策略作者
    version = Column(String)  # 版本
    doc = Column(String)  # 帮助说明
    module_name = Column(String)  # 实际策略导入模块名

    def __str__(self):
        return 'PartModel(id={}, hub_name={}, part={}, name={}, author={}, module_name={})'.format(
            self.id, self.hub_name, self.part, self.name, self.author, self.module_name
        )

    def __repr__(self):
        return '<{}>'.format(self.__str__())


class HubNameRepeatError(Exception):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return "已存在相同名称的仓库（{}），请更换仓库名！".format(self.name)


class HubNotFoundError(Exception):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return '找不到指定的仓库（"{}"）'.format(self.name)


class ModuleConflictError(Exception):
    def __init__(self, hub_name, conflict_module, hub_path):
        self.hub_name = hub_name
        self.conflict_module = conflict_module
        self.hub_path = hub_path

    def __str__(self):
        return '该仓库（{}）路径名与其他 python 模块（"{}"）冲突，请更改目录名称！（"{}"）'.format(
            self.hub_name, self.conflict_module, self.hub_path
        )


class PartNotFoundError(Exception):
    def __init__(self, name, cause):
        self.name = name
        self.cause = cause

    def __str__(self):
        return '未找到指定的策略部件: "{}", {}!'.format(self.name, self.cause)


class PartNameError(Exception):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return '无效的策略部件名称: "{}"!'.format(self.name)


# Windows下 shutil.rmtree 删除的目录中如有存在只读文件或目录会导致失败，需要此函数辅助处理
# 可参见：https://blog.csdn.net/Tri_C/article/details/99862201
def handle_remove_read_only(func, path, exc):
    excvalue = exc[1]
    if func in (os.rmdir, os.remove, os.unlink) and excvalue.errno == errno.EACCES:
        os.chmod(path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)  # 0777
        func(path)
    else:
        raise RuntimeError('无法移除目录 "{}"，请手工删除'.format(path))


def dbsession(func):
    def wrapfunc(*args, **kwargs):
        x = args[0]
        old_session = x._session
        if x._session is None:
            x._session = x._scoped_Session()
        result = func(*args, **kwargs)
        x._session.commit()
        if old_session is not x._session:
            x._session.close()
            x._session = old_session
        return result

    return wrapfunc


class HubManager(metaclass=SingletonType):
    """策略库管理"""

    def __init__(self):
        self.logger = logging.getLogger(self.__class__.__name__)
        usr_dir = os.path.expanduser('~')
        hku_dir = '{}/.hikyuu'.format(usr_dir)
        if not os.path.lexists(hku_dir):
            os.mkdir(hku_dir)

        # 创建仓库数据库
        engine = create_engine("sqlite:///{}/.hikyuu/hub.db".format(usr_dir))
        Base.metadata.create_all(engine)
        self._scoped_Session = scoped_session(sessionmaker(autocommit=False, autoflush=False, bind=engine))
        self._session = None

    @dbsession
    def setup_hub(self):
        """初始化 hikyuu 默认策略仓库"""
        usr_dir = os.path.expanduser('~')

        # 检查并建立远端仓库的本地缓存目录
        self.remote_cache_dir = self._session.query(ConfigModel.value).filter(ConfigModel.key == 'remote_cache_dir'
                                                                              ).first()
        if self.remote_cache_dir is None:
            self.remote_cache_dir = "{}/.hikyuu/hub_cache".format(usr_dir)
            record = ConfigModel(key='remote_cache_dir', value=self.remote_cache_dir)
            self._session.add(record)
        else:
            self.remote_cache_dir = self.remote_cache_dir[0]

        if not os.path.lexists(self.remote_cache_dir):
            os.makedirs(self.remote_cache_dir)

        # 将远程仓库本地缓存地址加入系统路径
        sys.path.append(self.remote_cache_dir)

        # 将所有本地仓库的上层路径加入系统路径
        hub_models = self._session.query(HubModel).filter_by(hub_type='local').all()
        for model in hub_models:
            sys.path.append(os.path.dirname(model.local))

        # 检查并下载 hikyuu 默认策略仓库, hikyuu_hub 避免导入时模块和 hikyuu 重名
        hikyuu_hub_path = self._session.query(HubModel.local).filter(HubModel.name == 'default').first()
        if hikyuu_hub_path is None:
            self.add_remote_hub('default', 'https://gitee.com/fasiondog/hikyuu_hub.git', 'main')

    def download_remote_hub(self, local_dir, url, branch):
        print('正在下载 hikyuu 策略仓库至："{}"'.format(local_dir))

        # 如果存在同名缓存目录，则强制删除
        if os.path.lexists(local_dir):
            shutil.rmtree(local_dir, onerror=handle_remove_read_only)

        try:
            git.Repo.clone_from(url, local_dir, branch=branch)
        except:
            raise RuntimeError("需要安装git（https://git-scm.com/），或检查网络是否正常或链接地址({})是否正确!".format(url))
        print('下载完毕')

    @dbsession
    def add_remote_hub(self, name, url, branch='main'):
        """增加远程策略仓库

        :param str name: 本地仓库名称（自行起名）
        :param str url: git 仓库地址
        :param str branch: git 仓库分支
        """
        record = self._session.query(HubModel).filter(HubModel.name == name).first()
        checkif(record is not None, name, HubNameRepeatError)

        record = self._session.query(HubModel).filter(and_(HubModel.url == url, HubModel.branch == branch)).first()

        # 下载远程仓库
        local_dir = "{}/{}".format(self.remote_cache_dir, name)
        self.download_remote_hub(local_dir, url, branch)

        # 导入仓库各部件策略信息
        record = HubModel(name=name, hub_type='remote', url=url, branch=branch, local_base=name, local=local_dir)
        self.import_part_to_db(record)

        # 更新仓库记录
        self._session.add(record)

    @dbsession
    def add_local_hub(self, name, path):
        """增加本地数据仓库

        :param str name: 仓库名称
        :param str path: 本地全路径
        """
        checkif(not os.path.lexists(path), '找不到指定的路径（"{}"）'.format(path))

        # 获取绝对路径
        local_path = os.path.abspath(path)

        record = self._session.query(HubModel).filter(HubModel.name == name).first()
        checkif(record is not None, name, HubNameRepeatError)

        # 将本地路径的上一层路径加入系统路径
        sys.path.append(os.path.dirname(path))

        # 检查仓库目录名称是否与其他 python 模块存在冲突
        tmp = importlib.import_module(os.path.basename(local_path))
        checkif(
            tmp.__path__[0] != local_path if sys.platform == 'win32' else tmp.__path__[0].lower() != local_path.lower(),
            name,
            ModuleConflictError,
            conflict_module=tmp.__path__[0],
            hub_path=local_path
        )

        # 导入部件信息
        local_base = os.path.basename(local_path)
        hub_model = HubModel(name=name, hub_type='local', local_base=local_base, local=local_path)
        self.import_part_to_db(hub_model)

        # 更新仓库记录
        self._session.add(hub_model)

    @dbsession
    def update_hub(self, name):
        """更新指定仓库

        :param str name: 仓库名称
        """
        hub_model = self._session.query(HubModel).filter_by(name=name).first()
        checkif(hub_model is None, '指定的仓库（{}）不存在！'.format(name))

        self._session.query(PartModel).filter_by(hub_name=name).delete()
        if hub_model.hub_type == 'remote':
            self.download_remote_hub(hub_model.local, hub_model.url, hub_model.branch)
        self.import_part_to_db(hub_model)

    @dbsession
    def build_hub(self, name, cmd='buildall'):
        """构建 cpp 部分 part

        :param str name: 仓库名称
        :param str cmd: 同仓库下 python setup.py 后的命令参数，如: build -t ind -n cpp_example
        """
        hub_model = self._session.query(HubModel).filter_by(name=name).first()
        checkif(hub_model is None, '指定的仓库（{}）不存在！'.format(name))
        if sys.platform == 'win32':
            os.system(f"python {hub_model.local}/setup.py {cmd}")
        else:
            os.system(f"python3 {hub_model.local}/setup.py {cmd}")

    @dbsession
    def remove_hub(self, name):
        """删除指定的仓库

        :param str name: 仓库名称
        """
        self._session.query(PartModel).filter_by(hub_name=name).delete()
        self._session.query(HubModel).filter_by(name=name).delete()

    @dbsession
    def import_part_to_db(self, hub_model):
        part_dict = {
            'af': 'part/af',
            'cn': 'part/cn',
            'ev': 'part/ev',
            'mf': 'part/mf',
            'mm': 'part/mm',
            'pg': 'part/pg',
            'se': 'part/se',
            'sg': 'part/sg',
            'sp': 'part/sp',
            'st': 'part/st',
            'pf': 'pf',
            'sys': 'sys',
            'ind': 'ind',
            'other': 'other',
        }

        # 检查仓库本地目录是否存在，不存在则给出告警信息并直接返回
        local_dir = hub_model.local
        if not os.path.lexists(local_dir):
            self.logger.warning(
                'The {} hub path ("{}") is not exists! Ignored this hub!'.format(hub_model.name, hub_model.local)
            )
            return

        base_local = os.path.basename(local_dir)

        # 遍历仓库导入部件信息
        for part, part_dir in part_dict.items():
            path = "{}/{}".format(hub_model.local, part_dir)
            try:
                with os.scandir(path) as it:
                    for entry in it:
                        if (not entry.name.startswith('.')) and entry.is_dir() and (entry.name != "__pycache__"):
                            # 计算实际的导入模块名
                            module_name = '{}.part.{}.{}.part'.format(base_local, part, entry.name) if part not in (
                                'pf', 'sys', 'ind', 'other'
                            ) else '{}.{}.{}.part'.format(base_local, part, entry.name)

                            # 导入模块
                            try:
                                part_module = importlib.import_module(module_name)
                            except ModuleNotFoundError:
                                self.logger.error('{} 缺失 part.py 文件, 位置："{}"！'.format(module_name, entry.path))
                                continue
                            except:
                                self.logger.error('{} 无法导入该文件: {}'.format(module_name, entry.path))
                                continue

                            module_vars = vars(part_module)
                            if 'part' not in module_vars:
                                self.logger.error('缺失 part 函数！("{}")'.format(entry.path))
                                continue

                            name = '{}.{}.{}'.format(hub_model.name, part, entry.name) if part not in (
                                'pf', 'sys', 'ind', 'other'
                            ) else '{}.{}.{}'.format(hub_model.name, part, entry.name)

                            try:
                                part_model = PartModel(
                                    hub_name=hub_model.name,
                                    part=part,
                                    name=name,
                                    module_name=module_name,
                                    author=part_module.author.strip() if 'author' in module_vars else 'None',
                                    version=part_module.version.strip() if 'version' in module_vars else 'None',
                                    doc=part_module.part.__doc__.strip() if part_module.part.__doc__ else "None"
                                )
                                self._session.add(part_model)
                            except Exception as e:
                                self.logger.error('存在语法错误 ("{}/part.py")! {}'.format(entry.path, e))
                                continue

            except FileNotFoundError:
                continue

    @dbsession
    def get_part(self, name, **kwargs):
        """获取指定策略部件

        :param str name: 策略部件名称
        :param kwargs: 其他部件相关参数
        """
        name_parts = name.split('.')
        checkif(
            len(name_parts) < 2
            or (name_parts[-2] not in ('af', 'cn', 'ev', 'mf', 'mm', 'pg', 'se', 'sg', 'sp', 'st', 'pf', 'sys', 'ind', 'other')),
            name, PartNameError
        )

        # 未指定仓库名，则默认使用 'default' 仓库
        part_name = 'default.{}'.format(name) if len(name_parts) == 2 else name
        part_model = self._session.query(PartModel).filter_by(name=part_name).first()
        checkif(part_model is None, part_name, PartNotFoundError, cause='仓库中不存在')
        try:
            part_module = importlib.import_module(part_model.module_name)
        except ModuleNotFoundError:
            raise PartNotFoundError(part_name, '请检查部件对应路径是否存在')
        part = part_module.part(**kwargs)
        try:
            part.name = part_model.name
            part.info = self.get_part_info(part.name)
        except:
            pass
        return part

    @dbsession
    def get_part_info(self, name):
        """获取策略部件信息

        :param str name: 部件名称
        """
        part_model = self._session.query(PartModel).filter_by(name=name).first()
        checkif(part_model is None, name, PartNotFoundError, cause='仓库中不存在')
        return {
            'name': name,
            'author': part_model.author,
            'version': part_model.version,
            'doc': part_model.doc,
        }

    def print_part_info(self, name):
        info = self.get_part_info(name)
        print('+---------+------------------------------------------------')
        print('| name    | ', info['name'])
        print('+---------+------------------------------------------------')
        print('| author  | ', info['author'])
        print('+---------+------------------------------------------------')
        print('| version | ', info['version'])
        print('+---------+------------------------------------------------')
        # print('\n')
        print(info['doc'])
        # print('\n')
        # print('----------------------------------------------------------')

    @dbsession
    def get_hub_path(self, name):
        """获取仓库所在的本地路径

        :param str name: 仓库名
        """
        path = self._session.query(HubModel.local).filter_by(name=name).first()
        checkif(path is None, name, HubNotFoundError)
        return path[0]

    @dbsession
    def get_hub_name_list(self):
        """返回仓库名称列表"""
        return [record[0] for record in self._session.query(HubModel.name).all()]

    @dbsession
    def get_part_name_list(self, hub=None, part_type=None):
        """获取部件名称列表

        :param str hub: 仓库名
        :param str part_type: 部件类型
        """
        if hub is None and part_type is None:
            results = self._session.query(PartModel.name).all()
        elif hub is None:
            results = self._session.query(PartModel.name).filter_by(part=part_type).all()
        elif part_type is None:
            results = self._session.query(PartModel.name).filter_by(hub_name=hub).all()
        else:
            results = self._session.query(PartModel.name
                                          ).filter(and_(PartModel.hub_name == hub, PartModel.part == part_type)).all()
        return [record[0] for record in results]

    @dbsession
    def get_current_hub(self, filename):
        """用于在仓库part.py中获取当前所在的仓库名

        示例： get_current_hub(__file__)
        """
        abs_path = os.path.abspath(filename)  # 当前文件的绝对路径
        path_parts = pathlib.Path(abs_path).parts
        local_base = path_parts[-4] if path_parts[-3] in ('pf', 'sys', 'ind', 'other') else path_parts[-5]
        hub_model = self._session.query(HubModel.name).filter_by(local_base=local_base).first()
        checkif(hub_model is None, local_base, HubNotFoundError)
        return hub_model.name


def add_remote_hub(name, url, branch='main'):
    """增加远程策略仓库

    :param str name: 本地仓库名称（自行起名）
    :param str url: git 仓库地址
    :param str branch: git 仓库分支
    """
    HubManager().add_remote_hub(name, url, branch)


def add_local_hub(name, path):
    """增加本地数据仓库

    :param str name: 仓库名称
    :param str path: 本地全路径
    """
    HubManager().add_local_hub(name, path)


def update_hub(name):
    """更新指定仓库

    :param str name: 仓库名称
    """
    HubManager().update_hub(name)


def build_hub(name, cmd='buildall'):
    """构建 cpp 部分 part

    :param str name: 仓库名称
    :param str cmd: 同仓库下 python setup.py 后的命令参数，如: build -t ind -n cpp_example
    """
    HubManager().build_hub(name, cmd)


def remove_hub(name):
    """删除指定的仓库

    :param str name: 仓库名称
    """
    HubManager().remove_hub(name)


def get_part(name, *args, **kwargs):
    """获取指定策略部件

    :param str name: 策略部件名称
    :param args: 其他部件相关参数
    :param kwargs: 其他部件相关参数
    """
    return HubManager().get_part(name, *args, **kwargs)


def get_hub_path(name):
    """获取仓库所在的本地路径

    :param str name: 仓库名
    """
    return HubManager().get_hub_path(name)


def get_part_info(name):
    """获取策略部件信息

    :param str name: 部件名称
    """
    return HubManager().get_part_info(name)


def print_part_info(name):
    HubManager().print_part_info(name)


def get_hub_name_list():
    """返回仓库名称列表"""
    return HubManager().get_hub_name_list()


def get_part_name_list(hub=None, part_type=None):
    """获取部件名称列表
    :param str hub: 仓库名
    :param str part_type: 部件类型
    """
    return HubManager().get_part_name_list(hub, part_type)


def get_current_hub(filename):
    """用于在仓库part.py中获取当前所在的仓库名

    示例： get_current_hub(__file__)
    """
    return HubManager().get_current_hub(filename)


# 初始化仓库
try:
    HubManager().setup_hub()
except Exception as e:
    HubManager().logger.warning("无法初始化 hikyuu 策略仓库！ {}".format(e))

__all__ = [
    'add_remote_hub',
    'add_local_hub',
    'update_hub',
    'remove_hub',
    'build_hub',
    'get_part',
    'get_hub_path',
    'get_part_info',
    'print_part_info',
    'get_hub_name_list',
    'get_part_name_list',
    'get_current_hub',
]

if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO, format='%(asctime)-15s [%(levelname)s] - %(message)s [%(name)s::%(funcName)s]'
    )
    # add_local_hub('dev', '/home/fasiondog/workspace/stockhouse')
    remove_hub('dev')
    add_local_hub('dev', r'D:\workspace\hikyuu_hub')
    # update_hub('test1')
    # update_hub('default')
    # build_hub('dev', 'buildall')
    # sg = get_part('dev.st.fixed_percent')
    # print(sg)
    # print_part_info('default.sp.fixed_value')
    # print(get_part_name_list(part_type='sg'))
