# -*- coding: utf-8 -*-

import os
from sqlalchemy import Column, Integer, String
from .BaseModel import BaseModel


class SessionModel(BaseModel):
    __tablename__ = 'session'

    id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String(64))
    host = Column(String(39))
    port = Column(Integer)
    user = Column(String(64))
    password = Column(String(64))
    remark = Column(String(256))
