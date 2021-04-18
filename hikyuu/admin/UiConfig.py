# -*- coding: utf-8 -*-

import os
from configparser import ConfigParser


class UiConfig(ConfigParser):
    def __init__(self):
        super(UiConfig, self).__init__()
        self.config_path = "%s/.hikyuu/admin" % os.path.expanduser('~')
        self.config_file = '%s/ui.ini' % self.config_path
        if os.path.exists(self.config_file):
            self.read(self.config_file, encoding='utf-8')
        if not self.has_section('main_window'):
            self.add_section('main_window')

    def save(self, main_window):
        self.set('main_window', 'width', str(main_window.width()))
        self.set('main_window', 'height', str(main_window.height()))
        self.set('main_window', 'maximized', str(main_window.isMaximized()))

        if not os.path.lexists(self.config_path):
            os.makedirs(self.config_path)
        with open(self.config_file, 'w', encoding='utf-8') as f:
            self.write(f)
