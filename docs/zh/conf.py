#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Hikyuu documentation build configuration file (Chinese source tree).
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# NOTE: the English (docs/en) and Chinese (docs/zh) trees are two independent
# Sphinx projects. Keep both conf.py in sync (extensions, theme, source_suffix,
# static path, version); only `language` / `html_search_language` differ.

import sphinx_rtd_theme
import sys
import os

sys.path.insert(0, os.path.abspath('.'))

version = 'latest'
release = version

# General information about the project.
project = 'Hikyuu Quant Framework'
copyright = '2017, fasiondog, <a href="https://hikyuu.org/">Hikyuu</a>'
author = 'fasiondog'

extensions = [
    'myst_parser',
    'sphinx_markdown_tables',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.mathjax'
]
autosectionlabel_prefix_document = True

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
source_suffix = ['.rst', '.md']

# The master toctree document.
master_doc = 'index'
language = 'zh_CN'
exclude_patterns = []
pygments_style = 'sphinx'
todo_include_todos = False

html_theme = 'sphinx_rtd_theme'
html_favicon = "./_static/favicon.ico"

html_static_path = ['_static']
html_css_files = [
    'style.css',
]

html_use_index = True
html_search_language = 'zh'
htmlhelp_basename = 'Hikyuudoc'

needs_sphinx = '4.2.0'


def setup(app):
    """Allow CJK characters and full-width punctuation to follow an inline-
    markup *closing* delimiter.

    By default docutils only accepts ASCII whitespace/punctuation after an
    inline-markup closing delimiter, so Chinese text like ``**中文**（`` or
    double-backtick literals followed by full-width punctuation is wrongly
    reported as "Inline X start-string without end-string".

    We only relax the *trailing* constraint: ``closing_delimiters`` feeds
    docutils' ``end_string_suffix`` (used after a close) but is *not* used by
    the leading constraint, so ``|substitution|`` references, hyperlink
    targets, etc. stay at docutils' default and are unaffected. Appending the
    CJK ranges below makes a close valid when followed by any CJK character or
    full-width punctuation. This patch is zh-only: the English tree is pure
    ASCII and keeps docutils' default behaviour.
    """
    import docutils.utils.punctuation_chars as punctuation_chars
    # CJK symbols/punctuation, CJK unified ideographs, fullwidth forms
    cjk = '\u3000-\u303f\u4e00-\u9fff\uFF01-\uFF60'
    if cjk not in punctuation_chars.closing_delimiters:
        punctuation_chars.closing_delimiters += cjk
