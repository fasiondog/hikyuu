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

The quick-links section is exempt as well: its table points at
language-specific resources (the localized documentation site/project and the
en/zh notebook trees), so both the fingerprint and the link-discipline checks
skip everything under a heading whose title contains one of the
_QUICK_LINKS_KEYWORDS ("Quick Links" / "快速导航").

Another intentionally allowed divergence is the hero tagline (the centered
paragraph under the title mascot and above the badge row): its wording is
prose, and its line-break layout is language-specific. CJK is compact while
the equivalent English text is 1.5-2x longer, so the English tagline may need
more <br> breaks at semantic boundaries and may use concise wording variants
(e.g. "Trading model R&D" vs "交易模型研发"). Only structural presence is
checked -- both READMEs must have a centered, non-image tagline paragraph with
visible text right after the mascot; its text and the number of <br> tags are
never compared.

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

# A centered HTML paragraph block in the hero area.
_P_BLOCK = re.compile(r'<p\s+align="center">\s*(.*?)\s*</p>', re.DOTALL | re.IGNORECASE)
_TAG_STRIP = re.compile(r"<[^>]+>")

# Headings whose title contains any of these mark a language-specific section
# whose content is allowed to diverge between the two READMEs (case-insensitive).
_DONATION_KEYWORDS = ("donation", "donate", "sponsor", "捐赠", "捐款", "赞赏", "打赏")

# The quick-links table points at language-specific resources (localized
# documentation sites and the en/zh notebook trees), so its links are allowed
# to differ between the two READMEs.
_QUICK_LINKS_KEYWORDS = ("quick link", "快速导航", "快速链接")

_EXEMPT_SECTION_KEYWORDS = _DONATION_KEYWORDS + _QUICK_LINKS_KEYWORDS


def _is_exempt_heading(line):
    m = _HEADING.match(line)
    if not m:
        return False
    title = line[len(m.group(1)):].strip().lower()
    return any(keyword in title for keyword in _EXEMPT_SECTION_KEYWORDS)


_FENCE = re.compile(r"^\s{0,3}(`{3,}|~{3,})")


def fingerprint(path):
    headings, images = [], []
    # While inside an exempt section: level of its heading, otherwise None.
    skip_level = None
    # Fenced code block marker (``` or ~~~), so comment lines such as
    # "# comment" inside code samples are not mistaken for H1 headings.
    fence = None
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            fm = _FENCE.match(line)
            if fm:
                marker = fm.group(1)[0]
                if fence is None:
                    fence = marker
                elif fence == marker:
                    fence = None
                continue
            if fence is not None:
                continue
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


def strip_exempt_sections(text):
    """Remove exempt sections (see _is_exempt_heading) from markdown text.

    Everything from an exempt heading up to (but not including) the next
    heading of the same or a higher level is dropped -- including the heading
    itself -- so the link-language and notebook-path checks do not flag
    intentionally language-specific links inside those sections.
    """
    kept = []
    # While inside an exempt section: level of its heading, otherwise None.
    skip_level = None
    fence = None
    for line in text.splitlines(keepends=True):
        fm = _FENCE.match(line)
        if fm:
            marker = fm.group(1)[0]
            if fence is None:
                fence = marker
            elif fence == marker:
                fence = None
        elif fence is None:
            m = _HEADING.match(line)
            if skip_level is not None and m and len(m.group(1)) <= skip_level:
                # A new section starts here; fall through and keep the line.
                skip_level = None
            if m and _is_exempt_heading(line):
                skip_level = len(m.group(1))
        if skip_level is None:
            kept.append(line)
    return "".join(kept)


def links(path):
    found = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            for match in _LINK.finditer(line):
                url = match.group(1) or match.group(2)
                if url:
                    found.append(url)
    return found


def has_hero_tagline(text):
    """The first centered <p> after the mascot image must be the tagline:
    a non-image paragraph with visible text. Its wording and <br> count are
    deliberately not compared across languages.
    """
    blocks = _P_BLOCK.findall(text[:2000])
    seen_mascot = False
    for inner in blocks:
        if "00000-title" in inner:
            seen_mascot = True
            continue
        if not seen_mascot:
            continue
        if "<img" in inner.lower():
            continue  # badge row or language switch, not the tagline
        plain = _TAG_STRIP.sub("", inner).strip()
        return bool(plain)
    return False


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

    # Hero tagline slot must exist in both; wording and line breaks may differ.
    if not has_hero_tagline(en_text):
        errors.append("readme.md is missing the hero tagline (centered text "
                      "paragraph after the mascot)")
    if not has_hero_tagline(zh_text):
        errors.append("readme.zh.md is missing the hero tagline (centered text "
                      "paragraph after the mascot)")

    # Link language discipline. Exempt sections (donation, quick links) may
    # point at language-specific sites and are stripped before the check.
    en_links_text = strip_exempt_sections(en_text)
    zh_links_text = strip_exempt_sections(zh_text)
    if "readthedocs.io/zh-cn/" in en_links_text:
        errors.append("readme.md must not link to the Chinese site (/zh-cn/)")
    if "readthedocs.io/en/" in zh_links_text:
        errors.append("readme.zh.md must not link to the English site (/en/)")

    en_headings, en_images = fingerprint(EN_README)
    zh_headings, zh_images = fingerprint(ZH_README)
    if en_headings != zh_headings:
        errors.append("heading level sequence mismatch: en=%s zh=%s" % (en_headings, zh_headings))
    if en_images != zh_images:
        errors.append(
            "image reference sequence mismatch: en-only=%s zh-only=%s" %
            (sorted(set(en_images) - set(zh_images)), sorted(set(zh_images) - set(en_images)))
        )

    # Notebook links must point at the language-specific tree (design doc 01 §2.3).
    for name, text in (("readme.md", en_links_text), ("readme.zh.md", zh_links_text)):
        for m in re.finditer(r"examples/notebook/(?!en/|zh/)([^)\s\"']+)", text):
            errors.append("%s links to the old unlocalized notebook path: examples/notebook/%s" % (name, m.group(1)))

    if errors:
        print("README PARITY CHECK FAILED (%d issue(s)):" % len(errors))
        for e in errors:
            print("  - " + e)
        return 1

    print("README PARITY CHECK PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
