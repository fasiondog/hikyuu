# -*- coding: utf-8 -*-

import hashlib
from .restful import check_res, session_get, session_post, session_delete, session_put

from data import SessionModel


def calPassword(pwd: str):
    sha = hashlib.sha256()
    sha.update(bytes(pwd, encoding='utf8'))
    return sha.hexdigest()


class UserService:
    @staticmethod
    def query_users(session: SessionModel, userid: int = None, name: str = None):
        data = {}
        if userid:
            data["userid"] = userid
        if name:
            data["name"] = name
        r = session_get(session, "user", "user", params=data) if data else session_get(session, "user", "user")
        check_res(r)
        data = r["data"]
        return data if data is not None else []

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

    @staticmethod
    def change_password(session: SessionModel, old_pwd, new_pwd, confirm_pwd):
        r = session_put(
            session,
            "user",
            "change_password",
            json=dict(old=calPassword(old_pwd), new=calPassword(new_pwd), confirm=calPassword(confirm_pwd))
        )
        check_res(r)
        return r
