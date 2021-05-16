# -*- coding: utf-8 -*-

import logging
from .restful import session_get, session_post, check_res

from data import SessionModel


class UserService:
    @staticmethod
    def query_users(session: SessionModel):
        r = session_get(session, "user", "user")
        check_res(r)
        return r["data"]

    @staticmethod
    def add_user(session: SessionModel, name: str, password: str):
        r = session_post(session, "user", "user", json={"user": name, "password": password})
        check_res(r)
        return r
