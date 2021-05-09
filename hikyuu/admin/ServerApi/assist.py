# -*- coding: utf-8 -*-

import logging
from .config import getServerApiUrl, defaultRequestHeader
from .restful import get

from data import SessionModel


def getServerStatus(session: SessionModel):
    url = getServerApiUrl(session.host, session.port, "assist", "status")
    headers = defaultRequestHeader()
    headers["hku_token"] = session.token
    try:
        r = get(url, headers=headers)
        if r["result"]:
            status = "running"
        else:
            logging.warning(r["errmsg"])
    except Exception as e:
        logging.error(e)
        status = "connection failed"
    return status
