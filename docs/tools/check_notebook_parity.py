#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Check the dual-source parity of the example notebooks (design doc 01 §2.3 / 05 §4.1 check 17).

The notebooks under hikyuu/examples/notebook are dual-source:
    notebook/en/  (English markdown cells)  +  notebook/zh/  (Chinese markdown cells)

Constraints to verify (design doc 01 §2.3):
    1. The file set (including Demo/) is identical on both sides.
    2. The cell count and the cell type sequence are identical per notebook.
    3. The code cell source texts are byte-identical on both sides (code is single-source, not
       translated). The markdown cells are only compared for existence, not content.

Usage:
    python3 docs/tools/check_notebook_parity.py
    python3 docs/tools/check_notebook_parity.py --allowlist <file>

Exit code: 0 = pass, non-zero = fail.
"""

import argparse
import hashlib
import os
import sys

import nbformat

DOCS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NOTEBOOK_DIR = os.path.join(DOCS_DIR, "..", "hikyuu", "examples", "notebook")
NOTEBOOK_DIR = os.path.normpath(NOTEBOOK_DIR)
EN_DIR = os.path.join(NOTEBOOK_DIR, "en")
ZH_DIR = os.path.join(NOTEBOOK_DIR, "zh")
DEFAULT_ALLOWLIST = os.path.join(DOCS_DIR, "tools", "parity_allowlist.txt")


def collect_notebooks(root):
    result = set()
    for dirpath, _dirnames, filenames in os.walk(root):
        for name in filenames:
            if name.endswith(".ipynb"):
                rel = os.path.relpath(os.path.join(dirpath, name), root)
                result.add(rel)
    return result


def cell_fingerprint(nb_path):
    nb = nbformat.read(nb_path, as_version=4)
    types = []
    code_hashes = []
    for cell in nb.cells:
        types.append(cell.cell_type)
        if cell.cell_type == "code":
            source = "".join(cell.source)
            code_hashes.append(hashlib.sha256(source.encode("utf-8")).hexdigest())
    return types, code_hashes


def load_allowlist(path):
    if not path or not os.path.exists(path):
        return set()
    result = set()
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.split("#", 1)[0].strip()
            if line:
                result.add(line)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--allowlist", default=DEFAULT_ALLOWLIST)
    args = parser.parse_args()

    if not (os.path.isdir(EN_DIR) and os.path.isdir(ZH_DIR)):
        print("NOTEBOOK PARITY CHECK SKIPPED: notebook/en or notebook/zh does not exist")
        return 0

    en_files = collect_notebooks(EN_DIR)
    zh_files = collect_notebooks(ZH_DIR)
    allowlist = load_allowlist(args.allowlist)

    errors = []

    en_only = en_files - zh_files
    zh_only = zh_files - en_files
    en_only = {f for f in en_only if f not in allowlist}
    zh_only = {f for f in zh_only if f not in allowlist}
    if en_only:
        errors.append("notebooks only in en: %s" % sorted(en_only))
    if zh_only:
        errors.append("notebooks only in zh: %s" % sorted(zh_only))

    for rel in sorted(en_files & zh_files):
        if rel in allowlist:
            continue
        en_types, en_code = cell_fingerprint(os.path.join(EN_DIR, rel))
        zh_types, zh_code = cell_fingerprint(os.path.join(ZH_DIR, rel))
        if en_types != zh_types:
            errors.append("%s: cell type sequence mismatch" % rel)
        if en_code != zh_code:
            errors.append("%s: code cell content mismatch" % rel)

    if errors:
        print("NOTEBOOK PARITY CHECK FAILED (%d issue(s)):" % len(errors))
        for e in errors:
            print("  - " + e)
        return 1

    print("NOTEBOOK PARITY CHECK PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
