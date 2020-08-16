#!/usr/bin/env python
# -*- coding: utf8 -*-
# cp936
#
#===============================================================================
# History
# 1. 20200816, Added by fasiondog
#===============================================================================

import os
import git
from configparser import ConfigParser

from sqlalchemy import create_engine, Sequence, Column, Integer, String, and_
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
Base = declarative_base()


class RemoteHouse(Base):
    __tablename__ = 'remote_house'

    id = Column(Integer, Sequence('remote_id_seq'), primary_key=True)
    name = Column(String, index=True)  # 本地仓库名
    url = Column(String)  # git 仓库地址
    branch = Column(String)  # 远程仓库分支
    local = Column(String)  # 本地缓存地址

    def __str__(self):
        return "RemoteHouse(id={}, url={}, local={})".format(self.id, self.url, self.local)

    def __repr__(self):
        return "<{}>".format(self.__str__())


class StockHouse(object):
    """策略库管理"""
    def __init__(self):
        usr_dir = os.path.expanduser('~')

        # 创建仓库数据库
        engine = create_engine("sqlite:///{}/.hikyuu/stockhouse.db".format(usr_dir))
        Base.metadata.create_all(engine)
        Session = sessionmaker(bind=engine)
        self._session = Session()

        # 获取策略仓库配置信息
        self.config_file = "{}/.hikyuu/stockhouse.ini".format(usr_dir)
        self.config = ConfigParser()

        if os.path.exists(self.config_file):
            self.config.read(self.config_file, encoding='utf-8')
        else:
            # 如果配置文件不存在，则创建默认配置信息
            # 创建本地默认策略仓库缓存目录
            house_dir = "{}/.hikyuu/stockhouse".format(usr_dir)
            if not os.path.lexists(house_dir):
                os.makedirs(house_dir)
            self.config["remote"] = {
                "cache": house_dir,
                "hikyuu": "https://gitee.com/fasiondog/stockhouse.git"
            }
            self.add_remote_house(self.config['remote']['hikyuu'], 'hikyuu', 'master')

            self._save_config()

    def _save_config(self):
        """保存配置信息"""
        with open(self.config_file, 'w', encoding='utf-8') as f:
            self.config.write(f)

    def add_remote_house(self, url, name, branch='master'):
        record = self._session.query(RemoteHouse).filter(RemoteHouse.name == name).first()
        assert record is None, '本地仓库名重复'

        record = self._session.query(RemoteHouse).filter(
            and_(RemoteHouse.url == url, RemoteHouse.branch == branch)
        ).first()
        assert record is None, '该仓库分支已存在'

        local_cache = "{}/{}".format(self.config["remote"]["cache"], name)
        clone = git.Repo.clone_from(url, local_cache, branch=branch)
        record = RemoteHouse(name=name, url=url, branch=branch, local=local_cache)
        self._session.add(record)
        self._session.commit()


if __name__ == "__main__":
    house = StockHouse()