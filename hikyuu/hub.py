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
from hikyuu.util import hku_info
import os
import stat
import errno
import sys
import shutil
import pathlib
import logging
import importlib
import inspect
import sqlalchemy
from functools import lru_cache

# The environment variables must be set before importing git, otherwise it may fail with an error in some cases
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
    key = Column(String, index=True)  # the parameter name
    value = Column(String)  # the parameter value

    __table_args__ = (UniqueConstraint('key'), )

    def __str__(self):
        return "ConfigModel(id={}, key={}, value={})".format(self.id, self.key, self.value)

    def __repr__(self):
        return "<{}>".format(self.__str__())


class HubModel(Base):
    __tablename__ = 'hub_repo'
    id = Column(Integer, Sequence('remote_id_seq'), primary_key=True)
    name = Column(String, index=True)  # the local repository name
    hub_type = Column(String)  # 'remote' (remote repository) | 'local' (local repository)
    local_base = Column(String)  # the base name of the local path
    local = Column(String)  # the local path
    url = Column(String)  # the git repository url
    branch = Column(String)  # the branch of the remote repository

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
    hub_name = Column(String)  # the identifier of the repository it belongs to
    part = Column(String)  # the part type
    name = Column(String)  # the strategy name
    author = Column(String)  # the strategy author
    version = Column(String)  # the version
    doc = Column(String)  # the help description
    module_name = Column(String)  # the actual module name imported by the strategy
    label = Column(String)  # the label
    __table_args__ = (
        UniqueConstraint('name', name='uq_part_model_name'),
    )

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
        return "A repository with the same name ({}) already exists, please use another repository name!".format(self.name)


class HubNotFoundError(Exception):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return 'The specified repository ("{}") cannot be found'.format(self.name)


class ModuleConflictError(Exception):
    def __init__(self, hub_name, conflict_module, hub_path):
        self.hub_name = hub_name
        self.conflict_module = conflict_module
        self.hub_path = hub_path

    def __str__(self):
        return 'The path name of this repository ({}) conflicts with another python module ("{}"), please change the directory name! ("{}")'.format(
            self.hub_name, self.conflict_module, self.hub_path
        )


class PartNotFoundError(Exception):
    def __init__(self, name, cause):
        self.name = name
        self.cause = cause

    def __str__(self):
        return 'The specified strategy part cannot be found: "{}", {}!'.format(self.name, self.cause)


class PartNameError(Exception):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return 'Invalid strategy part name: "{}"!'.format(self.name)


# On Windows, shutil.rmtree fails if the directory to be deleted contains read-only files or directories; this function is used to handle it
# See also: https://blog.csdn.net/Tri_C/article/details/99862201
def handle_remove_read_only(func, path, exc):
    excvalue = exc[1]
    if func in (os.rmdir, os.remove, os.unlink) and excvalue.errno == errno.EACCES:
        os.chmod(path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)  # 0777
        func(path)
    else:
        raise RuntimeError('Unable to remove the directory "{}", please delete it manually'.format(path))


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
    """Strategy repository management"""

    def __init__(self):
        self.logger = logging.getLogger(self.__class__.__name__)
        usr_dir = os.path.expanduser('~')
        hku_dir = '{}/.hikyuu'.format(usr_dir)
        if not os.path.lexists(hku_dir):
            os.mkdir(hku_dir)

        # Create the repository database
        engine = create_engine("sqlite:///{}/.hikyuu/hub.db".format(usr_dir))

        inspector = sqlalchemy.inspect(engine)
        if inspector.has_table(PartModel.__tablename__):
            columns = inspector.get_columns(PartModel.__tablename__)
            column_exists = any(column['name'] == 'label' for column in columns)
            if not column_exists:
                add_column_sql = sqlalchemy.text(
                    f"ALTER TABLE {PartModel.__tablename__} ADD COLUMN label TEXT;")
                with engine.connect() as connection:
                    connection.execute(add_column_sql)

            indexes = inspector.get_indexes(PartModel.__tablename__)
            index_exists = any(index['name'] == "uq_part_model_name" for index in indexes)
            if not index_exists:
                create_index_sql = sqlalchemy.text(
                    f"CREATE INDEX uq_part_model_name ON {PartModel.__tablename__} (name);")
                with engine.connect() as connection:
                    connection.execute(create_index_sql)

        Base.metadata.create_all(engine)
        self._scoped_Session = scoped_session(sessionmaker(autocommit=False, autoflush=False, bind=engine))
        self._session = None

    @dbsession
    def setup_hub(self):
        """Initialize the default hikyuu strategy repository"""
        usr_dir = os.path.expanduser('~')

        # Check and create the local cache directory of the remote repository
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

        # Add the local cache address of the remote repository to the system path
        sys.path.append(self.remote_cache_dir)

        # Add the parent paths of all the local repositories to the system path
        hub_models = self._session.query(HubModel).filter_by(hub_type='local').all()
        for model in hub_models:
            sys.path.append(os.path.dirname(model.local))

        # Check and download the default hikyuu strategy repository; hikyuu_hub avoids the module name conflicting with hikyuu when importing
        hikyuu_hub_path = self._session.query(HubModel.local).filter(HubModel.name == 'default').first()
        if hikyuu_hub_path is None:
            self.add_remote_hub('default', 'https://gitee.com/fasiondog/hikyuu_hub.git', 'main')

    def download_remote_hub(self, local_dir, url, branch):
        print('Downloading the hikyuu strategy repository to: "{}"'.format(local_dir))

        # If a cache directory with the same name exists, delete it forcibly
        if os.path.lexists(local_dir):
            shutil.rmtree(local_dir, onerror=handle_remove_read_only)

        try:
            git.Repo.clone_from(url, local_dir, branch=branch)
        except:
            raise RuntimeError("git (https://git-scm.com/) must be installed, or please check whether the network works and the url ({}) is correct!".format(url))
        print('Download completed')

    @dbsession
    def add_remote_hub(self, name, url, branch='main'):
        """Add a remote strategy repository

        :param str name: the local repository name (named by yourself)
        :param str url: the git repository url
        :param str branch: the git repository branch
        """
        record = self._session.query(HubModel).filter(HubModel.name == name).first()
        checkif(record is not None, name, HubNameRepeatError)

        record = self._session.query(HubModel).filter(and_(HubModel.url == url, HubModel.branch == branch)).first()

        # Download the remote repository
        local_dir = "{}/{}".format(self.remote_cache_dir, name)
        self.download_remote_hub(local_dir, url, branch)

        # Import the strategy part information of the repository
        record = HubModel(name=name, hub_type='remote', url=url, branch=branch, local_base=name, local=local_dir)
        self.import_part_to_db(record)

        # Update the repository record
        self._session.add(record)

    @dbsession
    def add_local_hub(self, name, path):
        """Add a local data repository

        :param str name: the repository name
        :param str path: the local full path
        """
        checkif(not os.path.lexists(path), 'The specified path ("{}") cannot be found'.format(path))

        # Get the absolute path
        local_path = os.path.abspath(path)

        record = self._session.query(HubModel).filter(HubModel.name == name).first()
        checkif(record is not None, name, HubNameRepeatError)

        # Add the parent path of the local path to the system path
        sys.path.append(os.path.dirname(path))

        # Check whether the repository directory name conflicts with other python modules
        tmp = importlib.import_module(os.path.basename(local_path))
        checkif(
            tmp.__path__[0] != local_path if sys.platform == 'win32' else tmp.__path__[0].lower() != local_path.lower(),
            name,
            ModuleConflictError,
            conflict_module=tmp.__path__[0],
            hub_path=local_path
        )

        # Import the part information
        local_base = os.path.basename(local_path)
        hub_model = HubModel(name=name, hub_type='local', local_base=local_base, local=local_path)
        self.import_part_to_db(hub_model)

        # Update the repository record
        self._session.add(hub_model)

    @dbsession
    def update_hub(self, name):
        """Update the specified repository

        :param str name: the repository name
        """
        hub_model = self._session.query(HubModel).filter_by(name=name).first()
        checkif(hub_model is None, 'The specified repository ({}) does not exist!'.format(name))

        self._session.query(PartModel).filter_by(hub_name=name).delete()
        if hub_model.hub_type == 'remote':
            self.download_remote_hub(hub_model.local, hub_model.url, hub_model.branch)
        self.import_part_to_db(hub_model)

    @dbsession
    def build_hub(self, name, cmd='buildall'):
        """Build the cpp part

        :param str name: the repository name
        :param str cmd: the command arguments after python setup.py in the repository, e.g. build -t ind -n cpp_example
        """
        hub_model = self._session.query(HubModel).filter_by(name=name).first()
        checkif(hub_model is None, 'The specified repository ({}) does not exist!'.format(name))
        if sys.platform == 'win32':
            os.system(f"python {hub_model.local}/setup.py {cmd}")
        else:
            os.system(f"python3 {hub_model.local}/setup.py {cmd}")

    @dbsession
    def remove_hub(self, name):
        """Remove the specified repository

        :param str name: the repository name
        """
        self._session.query(PartModel).filter_by(hub_name=name).delete()
        self._session.query(HubModel).filter_by(name=name).delete()

    @lru_cache
    def _get_module(self, module_name):
        return importlib.import_module(module_name)

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

        # Check whether the local directory of the repository exists; if not, give a warning and return directly
        local_dir = hub_model.local
        if not os.path.lexists(local_dir):
            self.logger.warning(
                'The {} hub path ("{}") is not exists! Ignored this hub!'.format(hub_model.name, hub_model.local)
            )
            return

        base_local = os.path.basename(local_dir)

        # Traverse the repository and import the part information
        for part, part_dir in part_dict.items():
            path = "{}/{}".format(hub_model.local, part_dir)
            try:
                with os.scandir(path) as it:
                    for entry in it:
                        if (not entry.name.startswith('.')) and entry.is_dir() and (entry.name != "__pycache__"):
                            # Calculate the actual module name to import
                            module_name = '{}.part.{}.{}.part'.format(base_local, part, entry.name) if part not in (
                                'pf', 'sys', 'ind', 'other'
                            ) else '{}.{}.{}.part'.format(base_local, part, entry.name)

                            # Import the module
                            try:
                                # part_module = importlib.import_module(module_name)
                                part_module = self._get_module(module_name)
                            except ModuleNotFoundError:
                                self.logger.error('{}: the part.py file is missing, location: "{}"!'.format(module_name, entry.path))
                                continue
                            except Exception as e:
                                self.logger.error('{}: unable to import the file: {}! {}'.format(module_name, entry.path, str(e)))
                                continue

                            module_vars = vars(part_module)
                            if 'part' not in module_vars:
                                self.logger.error('The part function is missing! ("{}")'.format(entry.path))
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
                                    doc=part_module.part.__doc__.strip() if part_module.part.__doc__ else "None",
                                    label=part_module.label.strip() if 'label' in module_vars else 'None',
                                )
                                self._session.add(part_model)
                            except Exception as e:
                                self.logger.error('There is a syntax error ("{}/part.py")! {}'.format(entry.path, e))
                                continue

            except FileNotFoundError:
                continue

    @dbsession
    def get_part(self, name, **kwargs):
        """Get the specified strategy part

        :param str name: the strategy part name
        :param kwargs: other part-related arguments
        """
        name_parts = name.split('.')
        checkif(
            len(name_parts) < 2
            or (name_parts[-2] not in ('af', 'cn', 'ev', 'mf', 'mm', 'pg', 'se', 'sg', 'sp', 'st', 'pf', 'sys', 'ind', 'other')),
            name, PartNameError
        )

        # If no repository name is specified, the 'default' repository is used by default
        part_name = 'default.{}'.format(name) if len(name_parts) == 2 else name
        part_model = self._session.query(PartModel).filter_by(name=part_name).first()
        checkif(part_model is None, part_name, PartNotFoundError, cause='does not exist in the repository')
        try:
            part_module = importlib.import_module(part_model.module_name)
        except ModuleNotFoundError:
            raise PartNotFoundError(part_name, 'Please check whether the path of the part exists')
        part = part_module.part(**kwargs)
        try:
            part.name = part_model.name
            part.info = self.get_part_info(part.name)
        except:
            pass
        return part

    @dbsession
    def get_part_module(self, name):
        """Get the specified strategy part

        :param str name: the strategy part name
        :param kwargs: other part-related arguments
        """
        name_parts = name.split('.')
        checkif(
            len(name_parts) < 2
            or (name_parts[-2] not in ('af', 'cn', 'ev', 'mf', 'mm', 'pg', 'se', 'sg', 'sp', 'st', 'pf', 'sys', 'ind', 'other')),
            name, PartNameError
        )

        # If no repository name is specified, the 'default' repository is used by default
        part_name = 'default.{}'.format(name) if len(name_parts) == 2 else name
        part_model = self._session.query(PartModel).filter_by(name=part_name).first()
        checkif(part_model is None, part_name, PartNotFoundError, cause='does not exist in the repository')
        try:
            part_module = importlib.import_module(part_model.module_name)
        except ModuleNotFoundError:
            raise PartNotFoundError(part_name, 'Please check whether the path of the part exists')
        return part_module

    @dbsession
    def get_part_info(self, name):
        """Get the strategy part information

        :param str name: the part name
        """
        part_model = self._session.query(PartModel).filter_by(name=name).first()
        checkif(part_model is None, name, PartNotFoundError, cause='does not exist in the repository')
        try:
            part_module = importlib.import_module(part_model.module_name)
        except ModuleNotFoundError:
            raise PartNotFoundError(name, 'Please check whether the path of the part exists')
        signature = inspect.signature(part_module.part)
        func_name = f'\npart("{name}",'
        for param_name, param in signature.parameters.items():
            if param.default is param.empty:
                func_name += f"{param_name}, "
            else:
                default_value = param.default
                func_name += f"{param_name}={default_value}, "
        func_name += ")\n"
        func_name += part_module.part.__doc__
        return {
            'name': name,
            'author': part_model.author,
            'version': part_model.version,
            'doc': func_name  # part_module.part.__doc__,
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
        print(info['doc'])

    @dbsession
    def get_hub_path(self, name):
        """Get the local path of the repository

        :param str name: the repository name
        """
        path = self._session.query(HubModel.local).filter_by(name=name).first()
        checkif(path is None, name, HubNotFoundError)
        return path[0]

    @dbsession
    def get_hub_name_list(self):
        """Return the list of the repository names"""
        return [record[0] for record in self._session.query(HubModel.name).all()]

    @dbsession
    def get_part_name_list(self, hub=None, part_type=None):
        """Get the list of the part names

        :param str hub: the repository name
        :param str part_type: the part type
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
        """Used to get the name of the repository the current part.py belongs to.
        Example: get_current_hub(__file__)
        """
        abs_path = os.path.abspath(filename)  # the absolute path of the current file
        path_parts = pathlib.Path(abs_path).parts
        local_base = path_parts[-4] if path_parts[-3] in ('pf', 'sys', 'ind', 'other') else path_parts[-5]
        hub_model = self._session.query(HubModel.name).filter_by(local_base=local_base).first()
        checkif(hub_model is None, local_base, HubNotFoundError)
        return hub_model.name

    @dbsession
    def search_part(self, name: str = None, hub: str = None, part_type: str = None, label=None):
        """Search parts
        :param str name: the part name
        :param str hub: the repository name
        :param str part_type: the part type
        :param str label: the label
        :return: the list of the part names
        :rtype: list
        """
        parts = None
        if name is not None:
            parts = self._session.query(PartModel).filter(PartModel.name.like(f'%{name}%'))
        if hub is not None:
            if parts is None:
                parts = self._session.query(PartModel).fileter(PartModel.hub_name.like(f'%{hub}%'))
            else:
                parts = parts.filter(PartModel.hub_name.like(f'%{hub}%'))
        if part_type is not None:
            if parts is None:
                parts = self._session.query(PartModel).filter(PartModel.part.like(f'%{part_type}%'))
            else:
                parts = parts.filter(PartModel.part.like(f'%{part_type}%'))
        if label is not None:
            if parts is None:
                parts = self._session.query(PartModel).filter(PartModel.label.like(f'%{label}%'))
            else:
                parts = parts.filter(PartModel.label.like(f'%{label}%'))
        records = parts.all() if parts is not None else []
        return [record.name for record in records]


def add_remote_hub(name, url, branch='main'):
    """Add a remote strategy repository

    :param str name: the local repository name (named by yourself)
    :param str url: the git repository url
    :param str branch: the git repository branch
    """
    HubManager().add_remote_hub(name, url, branch)


def add_local_hub(name, path):
    """Add a local data repository

    :param str name: the repository name
    :param str path: the local full path
    """
    HubManager().add_local_hub(name, path)


def update_hub(name):
    """Update the specified repository

    :param str name: the repository name
    """
    HubManager().update_hub(name)


def build_hub(name, cmd='buildall'):
    """Build the cpp part

    :param str name: the repository name
    :param str cmd: the command arguments after python setup.py in the repository, e.g. build -t ind -n cpp_example
    """
    HubManager().build_hub(name, cmd)


def remove_hub(name):
    """Remove the specified repository

    :param str name: the repository name
    """
    HubManager().remove_hub(name)


def get_part(name, *args, **kwargs):
    """Get the specified strategy part

    :param str name: the strategy part name
    :param args: other part-related arguments
    :param kwargs: other part-related arguments
    """
    return HubManager().get_part(name, *args, **kwargs)


def get_part_list(name_list):
    """
    Get the list of the specified strategy parts

    :param list name_list: the list of the part names
    :return: the list of parts
    :rtype: list
    """
    return [get_part(name) for name in name_list]


def get_hub_path(name):
    """Get the local path of the repository

    :param str name: the repository name
    """
    return HubManager().get_hub_path(name)


def get_part_info(name):
    """Get the strategy part information

    :param str name: the part name
    """
    return HubManager().get_part_info(name)


def print_part_info(name):
    HubManager().print_part_info(name)


help_part = print_part_info


def get_hub_name_list():
    """Return the list of the repository names"""
    return HubManager().get_hub_name_list()


def get_part_name_list(hub=None, part_type=None):
    """Get the list of the part names
    :param str hub: the repository name
    :param str part_type: the part type
    """
    return HubManager().get_part_name_list(hub, part_type)


def get_part_module(part_name: str):
    """Get the part module
    :param str part_name: the part name
    :return: the part module
    :rtype: module
    """
    return HubManager().get_part_module(part_name)


@lru_cache
def get_current_hub(filename):
    """Used to get the name of the repository the current part.py belongs to.
    Example: get_current_hub(__file__)
    """
    return HubManager().get_current_hub(filename)


def search_part(name: str = None, hub: str = None, part_type: str = None, label: str = None):
    """Search parts

    :param str name: the part name
    :param str hub: the repository name
    :param str part_type: the part type
    :param str label: the label
    :return: the list of the part names
    :rtype: list
    """
    return HubManager().search_part(name, hub, part_type, label)


# Initialize the repository
try:
    HubManager().setup_hub()
except Exception as e:
    HubManager().logger.warning("Unable to initialize the hikyuu strategy repository! {}".format(e))

__all__ = [
    'add_remote_hub',
    'add_local_hub',
    'update_hub',
    'remove_hub',
    'build_hub',
    'help_part',
    'get_part',
    'get_part_list',
    'get_hub_path',
    'get_part_info',
    'get_part_module',
    'print_part_info',
    'get_hub_name_list',
    'get_part_name_list',
    'get_current_hub',
    'search_part',
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
