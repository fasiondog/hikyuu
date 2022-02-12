# -*- coding: utf-8 -*-
import logging

import logging
from .restful import session_get

from data import SessionModel
from translate import _translate


class AssisService:
    @staticmethod
    def getServerStatus(session: SessionModel):
        try:
            r = session_get(session, "assist", "status")
            if r["result"]:
                session.running = True
                return "running", _translate("service", "running")
            else:
                session.running = False
                return "stop", _translate("service", "failed! {}").format(r["errmsg"])
        except Exception as e:
            session.running = False
            logging.info("{}: {}".format(e.__class__.__name__, e))
            return "stop", str(e)
