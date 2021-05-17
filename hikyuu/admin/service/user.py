# -*- coding: utf-8 -*-

import logging
from .restful import check_res, session_get, session_post, session_delete, session_put

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

    @staticmethod
    def remove_user(session: SessionModel, userid):
        r = session_delete(session, "user", "user", json={"userid": userid})
        check_res(r)
        return r

    @staticmethod
    def reset_password(session: SessionModel, userid):
        r = session_put(session, "user", "reset_password", json={"userid": userid})
        check_res(r)
        return r
