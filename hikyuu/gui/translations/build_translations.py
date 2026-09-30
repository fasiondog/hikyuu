#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Hikyuu GUI translation build script (Qt i18n: Chinese is the source language)

Naming convention (shared with the runtime loader in HikyuuTDX.install_translator):
    source catalog :  gui_<lang>.ts      (human-editable, kept in VCS)
    compiled output:  gui_<lang>.qm      (loaded at runtime; <lang> e.g. en, ja, fr)

Commands:
    release (default) : compile every gui_*.ts in this folder into gui_*.qm
    update            : run lupdate over the UI sources to refresh ALL existing
                        gui_*.ts (kept translations; new entries marked unfinished)
    add <lang>        : create a new gui_<lang>.ts catalog from the sources so a
                        new language can start from an existing one (e.g. add ja)
    all               : update, then release

Notes:
    - Only *.qm is needed at runtime; *.ts is the human-editable / lrelease source
    - After translating, always re-release so the *.qm takes effect
    - Relies on the lupdate / lrelease bundled with PySide6 (prefers the executables
      inside the PySide6 package dir, falls back to pyside6-* on PATH)

Examples:
    python build_translations.py                 # compile gui_*.ts -> gui_*.qm
    python build_translations.py update          # refresh every existing language .ts
    python build_translations.py add ja          # start a new Japanese catalog
    python build_translations.py all             # update + release in one go
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

# Folder layout: build_translations.py lives in <repo>/hikyuu/gui/translations/
THIS_DIR = Path(__file__).resolve().parent
GUI_DIR = THIS_DIR.parent                       # <repo>/hikyuu/gui
REPO_ROOT = THIS_DIR.parents[2]                 # <repo>

# Filename prefix shared by every language catalog (gui_<lang>.ts / gui_<lang>.qm)
PREFIX = "gui"
# Languages used when no gui_*.ts exists yet (fresh start)
DEFAULT_LANGS = ["en"]

# Sources lupdate must scan (*.ui static UI + *.py using self.tr / QCoreApplication.translate).
# Add new UI source files here as they appear; it is kept explicit so the extraction stays
# deterministic (a whole-directory scan would merge contexts and drop the curated catalog).
LUPDATE_SOURCES = [
    REPO_ROOT / "hikyuu" / "gui" / "data" / "MainWindow.ui",
    REPO_ROOT / "hikyuu" / "gui" / "HikyuuTDX.py",
    REPO_ROOT / "hikyuu" / "gui" / "data" / "UseTdxImportToH5Thread.py",
    REPO_ROOT / "hikyuu" / "gui" / "data" / "UsePytdxImportToH5Thread.py",
    REPO_ROOT / "hikyuu" / "gui" / "data" / "UseQmtImportToH5Thread.py",
]


def _exe(name):
    """Append the platform executable suffix (".exe" on Windows)"""
    return name + ".exe" if os.name == "nt" else name


def find_tool(name):
    """
    Locate a Qt tool (lupdate / lrelease).
    Prefer the executable inside the PySide6 package dir;
    otherwise fall back to pyside6-<name> or bare <name> on PATH.
    """
    try:
        import PySide6

        pyside_dir = Path(PySide6.__file__).parent
        candidate = pyside_dir / _exe(name)
        if candidate.exists():
            return str(candidate)
    except Exception:
        pass

    for alias in (f"pyside6-{name}", name):
        found = shutil.which(alias)
        if found:
            return found
    return None


def run(cmd):
    """Run a command, echo it, and return the exit code"""
    print("+ " + " ".join(str(c) for c in cmd))
    return subprocess.call([str(c) for c in cmd])


def ts_path(lang):
    """Return the gui_<lang>.ts path"""
    return THIS_DIR / f"{PREFIX}_{lang}.ts"


def discover_langs():
    """Existing language codes, from gui_*.ts already in this folder"""
    langs = []
    for p in sorted(THIS_DIR.glob(f"{PREFIX}_*.ts")):
        code = p.stem[len(PREFIX) + 1:]          # gui_en.ts -> en
        if code:
            langs.append(code)
    return langs


def _set_ts_language(path, lang):
    """Best-effort: set the <TS ... language=".."> attribute of a freshly created catalog"""
    try:
        text = path.read_text(encoding="utf-8")
        text = re.sub(r'(<TS[^>]*?language=")[^"]*(")', rf"\g<1>{lang}\g<2>", text, count=1)
        path.write_text(text, encoding="utf-8")
    except Exception:
        pass


def _check_sources():
    missing = [str(p) for p in LUPDATE_SOURCES if not p.exists()]
    if missing:
        print("Error: the following source files are missing; check the paths or update LUPDATE_SOURCES:", file=sys.stderr)
        for m in missing:
            print("   ", m, file=sys.stderr)
        return False
    return True


def do_update(langs):
    """Refresh the *.ts of every language from the sources (keeping existing translations)"""
    lupdate = find_tool("lupdate")
    if not lupdate:
        print("Error: lupdate not found; make sure PySide6 is installed (pip install PySide6)", file=sys.stderr)
        return 1
    if not _check_sources():
        return 1

    if not langs:
        langs = DEFAULT_LANGS
    targets = [ts_path(lang) for lang in langs]
    print(f"Updating language catalogs: {', '.join(langs)}")
    # -no-obsolete drops entries whose source text has been removed
    cmd = [lupdate, *[str(p) for p in LUPDATE_SOURCES], "-no-obsolete", "-ts", *[str(t) for t in targets]]
    rc = run(cmd)
    if rc == 0:
        print("\nIf new unfinished entries appeared, translate them per the glossary "
              "(docs/tools/glossary.zh-en.md), remove type=\"unfinished\", then run release.")
    return rc


def do_add(lang):
    """Create a new gui_<lang>.ts catalog from the sources (starting point for a new language)"""
    lupdate = find_tool("lupdate")
    if not lupdate:
        print("Error: lupdate not found; make sure PySide6 is installed (pip install PySide6)", file=sys.stderr)
        return 1
    if not _check_sources():
        return 1

    target = ts_path(lang)
    if target.exists():
        print(f"Error: {target.name} already exists; use 'update' to refresh it instead", file=sys.stderr)
        return 1

    print(f"Creating new language catalog: {target.name}")
    rc = run([lupdate, *[str(p) for p in LUPDATE_SOURCES], "-ts", str(target)])
    if rc == 0:
        _set_ts_language(target, lang)
        print(f"\nCreated {target.name} (all entries unfinished). Translate it per the glossary "
              f"(docs/tools/glossary.zh-en.md), then run release to produce {PREFIX}_{lang}.qm.")
    return rc


def do_release():
    """Compile every gui_*.ts in this folder into gui_*.qm"""
    lrelease = find_tool("lrelease")
    if not lrelease:
        print("Error: lrelease not found; make sure PySide6 is installed (pip install PySide6)", file=sys.stderr)
        return 1

    ts_files = sorted(THIS_DIR.glob(f"{PREFIX}_*.ts"))
    if not ts_files:
        print("Warning: no gui_*.ts catalogs found", file=sys.stderr)
        return 0

    rc_all = 0
    for ts in ts_files:
        qm = ts.with_suffix(".qm")
        rc = run([lrelease, str(ts), "-qm", str(qm)])
        rc_all = rc_all or rc
        if rc == 0:
            size = qm.stat().st_size if qm.exists() else 0
            print(f"  -> generated {qm.name} ({size} bytes)")
    return rc_all


def main():
    parser = argparse.ArgumentParser(
        description="Build Hikyuu GUI translations: lupdate extracts gui_<lang>.ts / lrelease compiles gui_<lang>.qm")
    parser.add_argument(
        "mode", nargs="?", default="release", choices=["release", "update", "add", "all"],
        help="release=compile gui_*.ts->*.qm (default); update=refresh all existing catalogs; "
             "add <lang>=create a new language catalog; all=update then release")
    parser.add_argument("lang", nargs="?", help="language code for 'add' (e.g. ja, fr, zh_TW)")
    args = parser.parse_args()

    if args.mode == "add":
        if not args.lang:
            parser.error("'add' requires a language code, e.g. 'add ja'")
        return do_add(args.lang.strip().lower())

    if args.mode in ("update", "all"):
        rc = do_update(discover_langs())
        if rc != 0:
            return rc
    if args.mode in ("release", "all"):
        return do_release()
    return 0


if __name__ == "__main__":
    sys.exit(main())
