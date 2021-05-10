# -*- coding: utf-8 -*-
import logging

import logging
from .config import getServerApiUrl, defaultRequestHeader
from .restful import get
from .user import update_token

from data import SessionModel
from translate import _translate


def getServerStatus(session: SessionModel):
    url = getServerApiUrl(session.host, session.port, "assist", "status")
    headers = defaultRequestHeader()
    headers["hku_token"] = session.token
    try:
        r = get(url, headers=headers)
        update_token(r, session)
        if r["result"]:
            return "running", _translate("ServerApi", "running")
        else:
            return "stop", _translate("ServerApi", "failed! {}").format(r["errmsg"])
    except Exception as e:
        return "stop",  _translate("ServerApi", "failed connect!")
