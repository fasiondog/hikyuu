#!/usr/bin/env python
# -*- coding: utf8 -*-
# cp936
#
#===============================================================================
# History
# 1. 20200816, Added by fasiondog
#===============================================================================

import os
import sys
import git
from configparser import ConfigParser

from sqlalchemy import (create_engine, Sequence, Column, Integer, String, and_, UniqueConstraint)
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
Base = declarative_base()


class ConfigModel(Base):
    __tablename__ = 'house_config'
    id = Column(Integer, Sequence('config_id_seq'), primary_key=True)
    key = Column(String, index=True)  # 参数名
    value = Column(String)  # 参数值

    __table_args__ = (UniqueConstraint('key'), )

    def __str__(self):
        return "ConfigModel(id={}, key={}, value={})".format(self.id, self.key, self.value)

    def __repr__(self):
        return "<{}>".format(self.__str__())


class HouseModel(Base):
    __tablename__ = 'house_repo'
    id = Column(Integer, Sequence('remote_id_seq'), primary_key=True)
    name = Column(String, index=True)  # 本地仓库名
    house_type = Column(String)
    local = Column(String)  # 本地地址
    url = Column(String)  # git 仓库地址
    branch = Column(String)  # 远程仓库分支

    __table_args__ = (UniqueConstraint('name'), )

    def __str__(self):
        return "RemoteHouse(id={}, url={}, local={})".format(self.id, self.url, self.local)

    def __repr__(self):
        return "<{}>".format(self.__str__())


class PartModel(Base):
    __tablename__ = 'house_part'
    id = Column(Integer, Sequence('part_id_seq'), primary_key=True)
    house_id = Column(Integer)  #所属仓库标识
    part = Column(String)  # 部件类型
    name = Column(String)  # 策略名称
    path = Column(String)  # 策略路径


class StockHouse(object):
    """策略库管理"""
    def __init__(self):
        usr_dir = os.path.expanduser('~')

        # 创建仓库数据库
        engine = create_engine("sqlite:///{}/.hikyuu/stockhouse.db".format(usr_dir))
        Base.metadata.create_all(engine)
        Session = sessionmaker(bind=engine)
        self._session = Session()

        # 检查并建立远端仓库的本地缓存目录
        self.remote_cache_dir = self._session.query(ConfigModel.value
                                                    ).filter(ConfigModel.key == 'remote_cache_dir'
                                                             ).first()
        if self.remote_cache_dir is None:
            self.remote_cache_dir = "{}/.hikyuu/stockhouse".format(usr_dir)
            record = ConfigModel(key='remote_cache_dir', value=self.remote_cache_dir)
            self._session.add(record)
            self._session.commit()
        else:
            self.remote_cache_dir = self.remote_cache_dir[0]

        if not os.path.lexists(self.remote_cache_dir):
            os.makedirs(self.remote_cache_dir)

        # 检查并下载 hikyuu 策略仓库
        hikyuu_house_path = self._session.query(HouseModel.local
                                                ).filter(HouseModel.name == 'hikyuu_house').first()
        if hikyuu_house_path is None:
            self.add_remote_house(
                'hikyuu_house', 'https://gitee.com/fasiondog/stockhouse.git', 'master'
            )

    def add_remote_house(self, name, url, branch='master'):
        """增加远程策略仓库

        :param str name: 仓库名称（自行起名）
        :param str url: git 仓库地址
        :param str branch: git 仓库分支
        """
        record = self._session.query(HouseModel).filter(HouseModel.name == name).first()
        assert record is None, '本地仓库名重复'

        record = self._session.query(HouseModel).filter(
            and_(HouseModel.url == url, HouseModel.branch == branch)
        ).first()
        assert record is None, '该仓库分支已存在'

        # 如果存在同名缓存目录，则强制删除
        local_dir = "{}/{}".format(self.remote_cache_dir, name)
        if os.path.lexists(local_dir):
            os.removedirs(local_dir)

        print('正在克隆至 "stockhouse/{}"'.format(name))
        try:
            clone = git.Repo.clone_from(url, local_dir, branch=branch)
        except:
            raise RuntimeError("请检查网络是否正常或链接地址({})是否正确!".format(url))

        # 导入仓库各部件策略信息
        self.import_part_to_db(local_dir)

        # 更新仓库记录
        record = HouseModel(name=name, house_type='remote', url=url, branch=branch, local=local_dir)
        self._session.add(record)
        self._session.commit()

    def import_part_to_db(self, local_dir):
        for root, dirs, files in os.walk(local_dir):
            for dir_name in dirs:
                print(dir_name)
            for file_name in files:
                print(file_name)


if __name__ == "__main__":
    house = StockHouse()