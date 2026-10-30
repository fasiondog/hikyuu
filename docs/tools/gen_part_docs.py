#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate the part meta pages from the JSON dumped by demo/part_dump.cpp.

The generated pages live in docs/{zh,en}/part_generated/ and coexist with the hand written
documents; they are marked as orphan so the Sphinx build does not require a toctree entry until
the quality has been verified. Both trees are generated from the same template so that the
structural parity check between them keeps passing.

Usage:
    python3 docs/tools/gen_part_docs.py                     # regenerate in place
    python3 docs/tools/gen_part_docs.py --check             # fail if the committed pages are stale
    python3 docs/tools/gen_part_docs.py --json <file>       # use the given dump instead of build/part_meta.json
"""

import argparse
import json
import sys
from pathlib import Path

DOCS = Path(__file__).resolve().parents[1]
ROOT = DOCS.parent
DEFAULT_JSON = ROOT / "build" / "part_meta.json"

LANGS = ("zh", "en")

PAGE_TITLE = {"zh": "{ptype} 部件", "en": "{ptype} parts"}
INDEX_TITLE = {"zh": "部件元信息（自动生成）", "en": "Part metadata (generated)"}
COLUMNS = {
    "zh": ("名称", "类型", "默认值", "必填", "说明"),
    "en": ("Name", "Type", "Default", "Required", "Description"),
}
REQUIRED = {"zh": ("是", "否"), "en": ("yes", "no")}


def category_file(ptype: str) -> str:
    return f"{ptype}.rst"


def display_width(text: str) -> int:
    """Width in terminal columns: CJK characters occupy two columns."""
    return sum(2 if ord(c) > 0x2E80 else 1 for c in text)


def render_category(lang: str, ptype: str, parts: list) -> str:
    title = PAGE_TITLE[lang].format(ptype=ptype.upper())
    cols = COLUMNS[lang]
    yes, no = REQUIRED[lang]
    lines = [":orphan:", "", title, "=" * display_width(title), ""]
    for p in parts:
        name = p["name"]
        lines += [f".. _{p['full_name']}:", "", name, "-" * display_width(name), ""]
        doc = " ".join(p["doc"].split()) if lang == "zh" else ""
        if doc:
            lines += [doc, ""]
        if p["params"]:
            lines += [
                ".. list-table::",
                "   :header-rows: 1",
                "   :widths: 18 18 18 8 38",
                "",
            ]
            lines.append(f"   * - {cols[0]}")
            for c in cols[1:]:
                lines.append(f"     - {c}")
            for q in p["params"]:
                default = q["default"] or "-"
                desc = " ".join(q["desc"].split()) if lang == "zh" else ""
                lines += [
                    f"   * - {q['name']}",
                    f"     - {q['type']}",
                    f"     - {default}",
                    f"     - {yes if q['required'] else no}",
                    f"     - {desc}",
                ]
            lines.append("")
    return "\n".join(lines) + "\n"


def render_index(lang: str, groups: dict) -> str:
    title = INDEX_TITLE[lang]
    lines = [":orphan:", "", title, "=" * display_width(title), ""]
    for ptype in sorted(groups):
        lines += [
            f"* :doc:`{ptype}` — {len(groups[ptype])}",
        ]
    return "\n".join(lines) + "\n"


def generate(meta: list) -> dict:
    groups = {}
    for p in meta:
        groups.setdefault(p["type"], []).append(p)
    out = {}
    for lang in LANGS:
        base = DOCS / lang / "part_generated"
        out[base / "index.rst"] = render_index(lang, groups)
        for ptype, parts in groups.items():
            out[base / category_file(ptype)] = render_category(lang, ptype, parts)
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", default=str(DEFAULT_JSON))
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    meta_path = Path(args.json)
    if not meta_path.exists():
        print(f"missing part meta dump: {meta_path} (run demo/part_dump first)")
        return 2

    meta = json.loads(meta_path.read_text(encoding="utf-8"))
    generated = generate(meta)

    stale = []
    for path, text in generated.items():
        if args.check:
            current = path.read_text(encoding="utf-8") if path.exists() else None
            if current != text:
                stale.append(path)
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")

    if args.check:
        if stale:
            print("generated part docs are stale, rerun docs/tools/gen_part_docs.py:")
            for path in stale:
                print("  " + str(path.relative_to(ROOT)))
            return 1
        print(f"generated part docs are up to date ({len(generated)} files)")
        return 0

    print(f"generated {len(generated)} files from {len(meta)} parts")
    return 0


if __name__ == "__main__":
    sys.exit(main())
