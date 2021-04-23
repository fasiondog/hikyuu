# -*- coding: utf-8 -*-

import os
import sqlalchemy
from .BaseModel import BaseModel
from .model import SessionModel


class LocalDatabase:
    def __init__(self):
        self._db_name = "%s/.hikyuu/admin/hkuadmin.db" % os.path.expanduser('~')
        self._engine = sqlalchemy.create_engine(
            "sqlite:///%s?check_same_thread=false" % self._db_name, echo=True, future=True
        )
        BaseModel.metadata.create_all(self._engine)
        self._session = sqlalchemy.orm.sessionmaker(bind=self._engine)()

    #def __del__(self):
    #    self._session.close()
    #    self._engine.dispose()

    @property
    def session(self):
        return self._session
