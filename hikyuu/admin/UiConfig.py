# -*- coding: utf-8 -*-

import os
from configparser import ConfigParser


class UiConfig(ConfigParser):
    def __init__(self, main_window):
        super(UiConfig, self).__init__()
        self.config_path = "%s/.hikyuu/admin" % os.path.expanduser('~')
        self.config_file = '%s/ui.ini' % self.config_path
        self.main_win = main_window
        if os.path.exists(self.config_file):
            self.read(self.config_file, encoding='utf-8')

    def setup(self):
        if self.getboolean('main_window', 'maximized', fallback=False):
            self.main_win.showMaximized()
        else:
            self.main_win.resize(
                self.getint('main_window', 'width', fallback=800), self.getint('main_window', 'height', fallback=600)
            )

    def save(self):
        if not self.has_section('main_window'):
            self.add_section('main_window')
        self.set('main_window', 'width', str(self.main_win.width()))
        self.set('main_window', 'height', str(self.main_win.height()))
        self.set('main_window', 'maximized', str(self.main_win.isMaximized()))

        if not os.path.lexists(self.config_path):
            os.makedirs(self.config_path)
        with open(self.config_file, 'w', encoding='utf-8') as f:
            self.write(f)
