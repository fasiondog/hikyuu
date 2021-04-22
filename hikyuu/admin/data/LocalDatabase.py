# -*- coding: utf-8 -*-

import os
import sqlalchemy
from .BaseModel import BaseModel
from .model import SessionModel


class LocalDatabase:
    def __init__(self):
        self.db_name = "%s/.hikyuu/admin/hkuadmin.db" % os.path.expanduser('~')
        self.engine = sqlalchemy.create_engine(
            "sqlite:///%s?check_same_thread=false" % self.db_name, echo=False, future=True
        )
        BaseModel.metadata.create_all(self.engine)
        self.session = sqlalchemy.orm.sessionmaker(bind=self.engine)()

    def __del__(self):
        self.session.close()
        self.engine.dispose()
