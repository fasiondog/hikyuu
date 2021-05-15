# -*- coding: utf-8 -*-

import logging
from .restful import session_get, RestfulError

from data import SessionModel


def queryUsers(session: SessionModel):
    r = session_get(session, "user", "user")
    if r["result"]:
        return r["data"]
    else:
        raise RestfulError(r)
