"""Check the consistency of the built-in part registration.

Every built-in factory is expected to pair a `void registerPart_XXX();` declaration (in its crt
header), a definition (in the imp/*.cpp or the module build_in.cpp) and one call inside the
module aggregation function of the module build_in.cpp. Nothing is generated here: the script
only reports the mismatches, so it is safe to run in CI.

Usage: python hikyuu_cpp/tools/check_part_registration.py
"""

import re
import sys
from pathlib import Path

CPP = Path(__file__).resolve().parents[1] / "hikyuu"

DECL = re.compile(r"void\s+(registerPart_\w+)\s*\(\s*\)\s*;")
DEF = re.compile(r"void\s+(registerPart_\w+)\s*\(\s*\)\s*\{")
CALL = re.compile(r"^\s+(registerPart_\w+)\s*\(\s*\)\s*;\s*$", re.M)

# crt headers that are combinations of existing parts rather than a creatable part
EXCLUDED_HEADERS = {
    "SG_Logic.h",
    "CN_Logic.h",
    "EV_Logic.h",
    "SE_Logic.h",
    "PF_Mode.h",
    "TC_TestStub.h",
}


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def main() -> int:
    headers = sorted(CPP.glob("**/crt/**/*.h")) + sorted(CPP.glob("*/crt/*.h"))
    build_ins = sorted(CPP.glob("**/build_in.cpp"))

    decl = {}
    for header in {p.resolve(): p for p in headers}.values():
        for name in DECL.findall(read(header)):
            decl.setdefault(name, header.relative_to(CPP).as_posix())

    # a definition may live anywhere in the library (imp, crt or the module aggregation file)
    defined = {}
    for source in sorted(CPP.rglob("*.cpp")):
        for name in DEF.findall(read(source)):
            defined.setdefault(name, source.relative_to(CPP).as_posix())

    called = {}
    for source in build_ins:
        for name in CALL.findall(read(source)):
            called.setdefault(name, source.relative_to(CPP).as_posix())

    problems = []
    for name, where in sorted(decl.items()):
        if name not in defined:
            problems.append(f"{name}: declared in {where} but never defined")
        if name not in called:
            problems.append(f"{name}: declared in {where} but not aggregated")
    for name, where in sorted(defined.items()):
        if name not in decl:
            problems.append(f"{name}: defined in {where} but never declared")
    for name, where in sorted(called.items()):
        if name not in defined:
            problems.append(f"{name}: aggregated in {where} but never defined")

    registered_headers = set()
    for name, where in decl.items():
        registered_headers.add(Path(where).name)
    for header in {p.resolve(): p for p in headers}.values():
        if header.name in EXCLUDED_HEADERS:
            continue
        if header.name not in registered_headers:
            problems.append(f"{header.relative_to(CPP).as_posix()}: no registerPart_XXX declared")

    # the full_name segment is lower case by convention; flag any mixed-case core.* string.
    # test_PartRegistry_builtin.cpp deliberately queries in upper case to cover the
    # case-insensitive lookup, so it is exempt.
    name_pat = re.compile(r"core\.[a-z]+\.([A-Za-z0-9_]+)")
    sources = sorted(CPP.rglob("*.cpp")) + sorted(CPP.rglob("*.h"))
    sources.append(CPP.parent.parent / "AGENTS.md")
    for source in sources:
        rel = "AGENTS.md" if source.name == "AGENTS.md" else source.relative_to(CPP).as_posix()
        if rel == "hikyuu_cpp/unit_test/hikyuu/part/test_PartRegistry_builtin.cpp":
            continue
        for m in name_pat.finditer(read(source)):
            if any(c.isupper() for c in m.group(1)):
                problems.append(f'{rel}: non-lower-case part name "{m.group(0)}"')

    if problems:
        print("part registration is inconsistent:")
        for line in problems:
            print("  " + line)
        return 1

    print(f"part registration ok: {len(decl)} declared, {len(called)} aggregated")
    return 0


if __name__ == "__main__":
    sys.exit(main())
