# -*- coding: utf-8 -*-

import os
import sqlalchemy
from sqlalchemy.ext.declarative import declarative_base


class LocalDatabase:
    model = declarative_base()

    def __init__(self):
        self._db_name = "%s/.hikyuu/admin/hkuadmin.db" % os.path.expanduser('~')
        self._engine = sqlalchemy.create_engine(
            "sqlite:///%s?check_same_thread=false" % self._db_name, echo=True, future=True
        )
        self.model.metadata.create_all(self._engine)
        self._session = sqlalchemy.orm.sessionmaker(bind=self._engine)()

    @property
    def session(self):
        return self._session


g_local_db = LocalDatabase()


def get_local_db():
    return g_local_db


def model_save(self):
    if self.id is None:
        g_local_db.session.add(self)
    g_local_db.session.commit()


def model_delete(self):
    if self.id is not None:
        g_local_db.delete(self)
        g_local_db.commit()
        self.id = None


# 为 Model 增加 save 和 delete 方法
LocalDatabase.model.save = model_save
LocalDatabase.model.delete = model_save
