#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Check the structural parity between readme.md (English) and readme.zh.md (Chinese).

Design doc 04 §6 / design doc 05 §4.1 check 16:
    - both files exist and link to each other (language navigation);
    - ATX heading level sequences match;
    - referenced image filename sequences match;
    - documentation links use the matching language segment
      (readme.md -> /en/latest/, readme.zh.md -> /zh-cn/latest/);
    - no cross-language links (en must not point to /zh-cn/, zh must not point to /en/).

The donation / sponsorship section is exempt: its content (images, sub-headings)
is allowed to diverge between the two languages. A section is recognized by an
ATX heading whose title contains one of the _DONATION_KEYWORDS; everything from
that heading up to (but not including) the next heading of the same or a higher
level is skipped when fingerprints are built.

Usage:
    python3 docs/tools/check_readme_parity.py

Exit code: 0 = pass, non-zero = fail (usable directly in CI).
"""

import os
import re
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EN_README = os.path.join(REPO_ROOT, "readme.md")
ZH_README = os.path.join(REPO_ROOT, "readme.zh.md")

_HEADING = re.compile(r"^(#{1,6})\s+\S")
_IMAGE = re.compile(r"!\[[^\]]*\]\(([^)]+)\)|<img[^>]+src=[\"']([^\"']+)[\"']")
_LINK = re.compile(r"\[[^\]]*\]\(([^)]+)\)|href=[\"']([^\"']+)[\"']")

# Headings whose title contains any of these mark a language-specific section
# whose content is allowed to diverge between the two READMEs (case-insensitive).
_DONATION_KEYWORDS = ("donation", "donate", "sponsor", "捐赠", "捐款", "赞赏", "打赏")


def _is_exempt_heading(line):
    m = _HEADING.match(line)
    if not m:
        return False
    title = line[len(m.group(1)):].strip().lower()
    return any(keyword in title for keyword in _DONATION_KEYWORDS)


def fingerprint(path):
    headings, images = [], []
    # While inside an exempt section: level of its heading, otherwise None.
    skip_level = None
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            m = _HEADING.match(line)
            if skip_level is not None:
                if m and len(m.group(1)) <= skip_level:
                    # A new section starts here; fall through and process it.
                    skip_level = None
                else:
                    continue
            if m and _is_exempt_heading(line):
                skip_level = len(m.group(1))
                continue
            if m:
                headings.append(len(m.group(1)))
            for match in _IMAGE.finditer(line):
                images.append(os.path.basename(match.group(1) or match.group(2)))
    return headings, images


def links(path):
    found = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            for match in _LINK.finditer(line):
                url = match.group(1) or match.group(2)
                if url:
                    found.append(url)
    return found


def main():
    errors = []

    for path in (EN_README, ZH_README):
        if not os.path.exists(path):
            errors.append("missing file: %s" % os.path.basename(path))
    if errors:
        print("README PARITY CHECK FAILED (%d issue(s)):" % len(errors))
        for e in errors:
            print("  - " + e)
        return 1

    en_text = open(EN_README, "r", encoding="utf-8").read()
    zh_text = open(ZH_README, "r", encoding="utf-8").read()

    # Mutual language navigation.
    if "readme.zh.md" not in en_text:
        errors.append("readme.md has no navigation link to readme.zh.md")
    if "readme.md" not in zh_text:
        errors.append("readme.zh.md has no navigation link to readme.md")

    # Link language discipline.
    if "readthedocs.io/zh-cn/" in en_text:
        errors.append("readme.md must not link to the Chinese site (/zh-cn/)")
    if "readthedocs.io/en/" in zh_text:
        errors.append("readme.zh.md must not link to the English site (/en/)")

    en_headings, en_images = fingerprint(EN_README)
    zh_headings, zh_images = fingerprint(ZH_README)
    if en_headings != zh_headings:
        errors.append("heading level sequence mismatch: en=%s zh=%s" % (en_headings, zh_headings))
    if en_images != zh_images:
        errors.append("image reference sequence mismatch: en-only=%s zh-only=%s"
                      % (sorted(set(en_images) - set(zh_images)),
                         sorted(set(zh_images) - set(en_images))))

    # Notebook links must point at the language-specific tree (design doc 01 §2.3).
    for name, text in (("readme.md", en_text), ("readme.zh.md", zh_text)):
        for m in re.finditer(r"examples/notebook/(?!en/|zh/)([^)\s\"']+)", text):
            errors.append("%s links to the old unlocalized notebook path: examples/notebook/%s"
                          % (name, m.group(1)))

    if errors:
        print("README PARITY CHECK FAILED (%d issue(s)):" % len(errors))
        for e in errors:
            print("  - " + e)
        return 1

    print("README PARITY CHECK PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
