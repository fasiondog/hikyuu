# -*- coding: utf-8 -*-

import os

class GuiConfig:
    def __init__(self):
        self.config_path = "%s/.hikyuu" % os.path.expanduser('~')
        self.config_file = '%s/admin_ui.ini' % self.config_path



