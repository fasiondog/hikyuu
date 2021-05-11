# -*- coding: utf-8 -*-
import logging

import logging
from .restful import get, session_get

from data import SessionModel
from translate import _translate


def getServerStatus(session: SessionModel):
    try:
        r = session_get(session, "assist", "status")
        if r["result"]:
            return "running", _translate("ServerApi", "running")
        else:
            return "stop", _translate("ServerApi", "failed! {}").format(r["errmsg"])
    except Exception as e:
        logging.info("{}: {}".format(e.__class__.__name__, e))
        return "stop",  _translate("ServerApi", "failed connect!")
