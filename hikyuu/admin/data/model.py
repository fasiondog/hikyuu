# -*- coding: utf-8 -*-

import os
from sqlalchemy import Column, Integer, String
from .BaseModel import BaseModel


class SessionModel(BaseModel):
    __tablename__ = 'session'

    id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String(64), unique=True)
    host = Column(String(39))
    port = Column(Integer)
    user = Column(String(64))
    password = Column(String(64))
    remark = Column(String(256))

    def __init__(self):
        self.name = ''
        self.host = '127.0.0.1'
        self.port = 9001
        self.user = ''
        self.password = ''
        self.remark = ''

    def __str__(self):
        return 'SessionModel(id={}, name="{}", host="{}", port={}, user="{}", password="******")'.format(
            self.id, self.name, self.host, self.port, self.user
        )

    def __repr__(self):
        return '<{}>'.format(str(self))
