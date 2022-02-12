# -*- coding: utf-8 -*-

from .model import SessionModel
from .LocalDatabase import get_local_db

# 创建数据库
get_local_db().create_all()

__all__ = [
    'get_local_db',
    'SessionModel',
]
