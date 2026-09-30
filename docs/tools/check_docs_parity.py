#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Check the structural parity between the two documentation trees (docs/en and docs/zh).

The dual-source documentation scheme (design doc 02 / 05) has no translation memory; the only
defense line is "structural consistency". This script compares, for every file of one tree, the
structure fingerprint against the same file in the other tree:

    fingerprint = (title hierarchy, explicit labels, image references,
                   code-block languages, literalinclude paths)

Usage:
    python3 docs/tools/check_docs_parity.py          # structure only (default)
    python3 docs/tools/check_docs_parity.py --full   # additionally hash _static and key conf.py items
    python3 docs/tools/check_docs_parity.py --allowlist <file>   # extra allowlist file

Exit code: 0 = pass, non-zero = fail (usable directly in CI).
"""

import argparse
import hashlib
import os
import re
import sys

DOCS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EN_DIR = os.path.join(DOCS_DIR, "en")
ZH_DIR = os.path.join(DOCS_DIR, "zh")
DEFAULT_ALLOWLIST = os.path.join(DOCS_DIR, "tools", "parity_allowlist.txt")
DEFAULT_CJK_ALLOWLIST = os.path.join(DOCS_DIR, "tools", "cjk_allowlist.txt")

# File extensions that carry documentation content.
CONTENT_EXTS = (".rst", ".md", ".ipynb")

# The conf.py keys that must stay identical between the two trees (design doc 05 §4.1 check 9).
CONF_KEYS = [
    "extensions",
    "html_theme",
    "html_static_path",
    "source_suffix",
    "project",
    "author",
    "copyright",
    "version",
    "release",
]

_TITLE_UNDERLINE = re.compile(r"^[=~`^+\-*#\"'<>,.:;_!]{4,}\s*$")


def collect_files(root):
    """Return the set of relative content file paths under root."""
    result = set()
    for dirpath, _dirnames, filenames in os.walk(root):
        for name in filenames:
            if name.startswith("."):
                continue
            if name.endswith(CONTENT_EXTS):
                rel = os.path.relpath(os.path.join(dirpath, name), root)
                result.add(rel)
    return result


def parse_rst_title_hierarchy(path):
    """Extract the (level char, title text) sequence as the structural fingerprint."""
    result = []
    with open(path, "r", encoding="utf-8") as f:
        lines = f.readlines()
    for i, line in enumerate(lines):
        stripped = line.rstrip("\n")
        if i + 1 < len(lines) and _TITLE_UNDERLINE.match(stripped):
            # The title text is the previous non-empty line.
            if i >= 1:
                text = lines[i - 1].rstrip("\n").strip()
                if text and not _TITLE_UNDERLINE.match(text):
                    result.append((stripped[0], text))
    return result


def parse_md_title_hierarchy(path):
    """Extract the markdown ATX heading level sequence (skipping fenced code blocks)."""
    result = []
    in_code = False
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if line.startswith("```"):
                in_code = not in_code
                continue
            if in_code:
                continue
            m = re.match(r"^(#{1,6})\s+(.*)$", line)
            if m:
                result.append((len(m.group(1)), m.group(2).strip()))
    return result


def parse_title_hierarchy(path):
    if path.endswith(".rst"):
        return parse_rst_title_hierarchy(path)
    if path.endswith(".md"):
        return parse_md_title_hierarchy(path)
    return []  # .ipynb title hierarchies are not compared


def parse_rst_items(path):
    """Extract explicit labels, image refs, code-block languages and literalinclude paths."""
    labels = set()
    images = set()
    code_langs = []
    literal_includes = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            m = re.match(r"^\.\.\s+_([^:]+):", line)
            if m:
                labels.add(m.group(1))
                continue
            m = re.match(r"^\.\.\s+(?:figure|image)::\s+(.+)$", line)
            if m:
                images.add(m.group(1).strip())
                continue
            m = re.match(r"^\.\.\s+code-block::\s*(\w+)", line)
            if m:
                code_langs.append(m.group(1))
                continue
            m = re.match(r"^\.\.\s+literalinclude::\s+(.+)$", line)
            if m:
                literal_includes.append(m.group(1).strip())
    return labels, images, code_langs, literal_includes


def parse_md_items(path):
    """Extract image refs and fenced code-block languages from a markdown file."""
    images = set()
    code_langs = []
    in_code = False
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if line.startswith("```"):
                if not in_code:
                    m = re.match(r"^```(\w+)?", line)
                    if m and m.group(1):
                        code_langs.append(m.group(1))
                in_code = not in_code
                continue
            if in_code:
                continue
            m = re.search(r"!\[[^\]]*\]\(([^)]+)\)", line)
            if m:
                images.add(m.group(1).strip())
    return set(), images, code_langs, []


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


def conf_fingerprint(conf_path):
    fingerprint = {}
    with open(conf_path, "r", encoding="utf-8") as f:
        text = f.read()
    for key in CONF_KEYS:
        m = re.search(r"^%s\s*=\s*(.+)$" % re.escape(key), text, re.MULTILINE)
        fingerprint[key] = m.group(1).strip() if m else "<missing>"
    return fingerprint


def static_hashes(root):
    result = {}
    static_dir = os.path.join(root, "_static")
    if not os.path.isdir(static_dir):
        return result
    for dirpath, _dirnames, filenames in os.walk(static_dir):
        for name in filenames:
            if name.startswith("."):
                continue
            path = os.path.join(dirpath, name)
            rel = os.path.relpath(path, static_dir)
            with open(path, "rb") as f:
                result[rel] = hashlib.sha256(f.read()).hexdigest()
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--full", action="store_true", help="additionally compare _static and conf.py")
    parser.add_argument("--allowlist", default=DEFAULT_ALLOWLIST, help="parity allowlist file")
    parser.add_argument("--cjk-allowlist", default=DEFAULT_CJK_ALLOWLIST,
                        help="files in docs/en allowed to contain CJK text (domain data, real output samples)")
    parser.add_argument("--skip-cjk", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()

    en_files = collect_files(EN_DIR)
    zh_files = collect_files(ZH_DIR)
    allowlist = load_allowlist(args.allowlist)
    cjk_allowlist = load_allowlist(args.cjk_allowlist) if os.path.exists(args.cjk_allowlist) else set()

    errors = []

    # Check 2: file set parity (allowlist-aware).
    en_only = en_files - zh_files
    zh_only = zh_files - en_files
    en_only = {f for f in en_only if f not in allowlist and "en/" + f not in allowlist}
    zh_only = {f for f in zh_only if f not in allowlist and "zh/" + f not in allowlist}
    if en_only:
        errors.append("files only in docs/en (allowlist-excluded): %s" % sorted(en_only))
    if zh_only:
        errors.append("files only in docs/zh (allowlist-excluded): %s" % sorted(zh_only))

    # Per-file structure fingerprint.
    for rel in sorted(en_files & zh_files):
        en_path = os.path.join(EN_DIR, rel)
        zh_path = os.path.join(ZH_DIR, rel)
        if rel in allowlist:
            continue

        en_titles = parse_title_hierarchy(en_path)
        zh_titles = parse_title_hierarchy(zh_path)
        # Titles: compare the level sequence only (the text is translated by design).
        if [t[0] for t in en_titles] != [t[0] for t in zh_titles]:
            errors.append(
                "%s: title level sequence mismatch: en=%s zh=%s"
                % (rel, [t[0] for t in en_titles], [t[0] for t in zh_titles])
            )

        en_labels, en_images, en_code, en_lit = parse_rst_items(
            en_path) if rel.endswith(".rst") else parse_md_items(en_path)
        zh_labels, zh_images, zh_code, zh_lit = parse_rst_items(
            zh_path) if rel.endswith(".rst") else parse_md_items(zh_path)

        if en_labels != zh_labels:
            errors.append("%s: explicit label mismatch: en-only=%s zh-only=%s" %
                          (rel, sorted(en_labels - zh_labels), sorted(zh_labels - en_labels)))
        if en_images != zh_images:
            errors.append("%s: image reference mismatch: en-only=%s zh-only=%s" %
                          (rel, sorted(en_images - zh_images), sorted(zh_images - en_images)))
        if en_code != zh_code:
            errors.append("%s: code-block language sequence mismatch: en=%s zh=%s" % (rel, en_code, zh_code))
        if en_lit != zh_lit:
            errors.append("%s: literalinclude path mismatch: en-only=%s zh-only=%s" %
                          (rel, sorted(set(en_lit) - set(zh_lit)), sorted(set(zh_lit) - set(en_lit))))

    # Check 13: docs/en must not contain Chinese body text. Files listed in cjk_allowlist.txt
    # are allowed to contain CJK because they are domain data / real tool output (stock names,
    # A-share API fields, DataFrame samples, Chinese bibliography, etc., design doc 01 §4).
    # --skip-cjk is kept as a hidden emergency escape hatch (design doc 05 §4.3).
    if not args.skip_cjk:
        cjk = re.compile("[\u4e00-\u9fff]")
        for rel in sorted(en_files):
            if rel in allowlist or rel in cjk_allowlist:
                continue
            path = os.path.join(EN_DIR, rel)
            with open(path, "r", encoding="utf-8") as f:
                for lineno, line in enumerate(f, 1):
                    if cjk.search(line):
                        errors.append("%s:%d: Chinese text found in docs/en" % (rel, lineno))

    # Check 11: no hardcoded cross-language links. The English tree must not link to /zh-cn/,
    # and the Chinese tree must not link to /en/ (design doc 02 §5.3). Same-language absolute
    # links (e.g. /en/latest inside docs/en) are tolerated for now.
    def _cross_language_links(root, forbidden_segment):
        hits = []
        pattern = re.compile(r"readthedocs\.io/%s/" % re.escape(forbidden_segment))
        for rel in sorted(collect_files(root)):
            if rel in allowlist:
                continue
            with open(os.path.join(root, rel), "r", encoding="utf-8") as f:
                for lineno, line in enumerate(f, 1):
                    if pattern.search(line):
                        hits.append("%s:%d: cross-language link /%s/ found"
                                    % (rel, lineno, forbidden_segment))
        return hits

    errors.extend(_cross_language_links(EN_DIR, "zh-cn"))
    errors.extend(_cross_language_links(ZH_DIR, "en"))

    if args.full:
        # Check 8: _static same-name file hashes.
        en_static = static_hashes(EN_DIR)
        zh_static = static_hashes(ZH_DIR)
        for name in sorted(set(en_static) | set(zh_static)):
            if name not in en_static:
                errors.append("_static/%s: missing in docs/en" % name)
            elif name not in zh_static:
                errors.append("_static/%s: missing in docs/zh" % name)
            elif en_static[name] != zh_static[name]:
                errors.append("_static/%s: content hash mismatch" % name)

        # Check 9: conf.py key items.
        en_conf = conf_fingerprint(os.path.join(EN_DIR, "conf.py"))
        zh_conf = conf_fingerprint(os.path.join(ZH_DIR, "conf.py"))
        for key in CONF_KEYS:
            if en_conf[key] != zh_conf[key]:
                errors.append("conf.py %s mismatch: en=%s zh=%s" % (key, en_conf[key], zh_conf[key]))

    if errors:
        print("PARITY CHECK FAILED (%d issue(s)):" % len(errors))
        for e in errors:
            print("  - " + e)
        return 1

    print("PARITY CHECK PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
