from __future__ import annotations

import argparse
import os
import re
from pathlib import Path

BEST_BLOCK_RE = re.compile(
    r"^Best in\s+(?P<gen>\d+)\s*$\s*(?P<xml><Individual\b.*?</Individual>)",
    re.MULTILINE | re.DOTALL,
)

FITNESSMIN_RE = re.compile(r'<FitnessMin\s+value="(?P<val>-?\d+(?:\.\d+)?)"\s*/>')


def extract_fitnessmin(xml: str) -> float:
    m = FITNESSMIN_RE.search(xml)
    if not m:
        raise ValueError("Could not find <FitnessMin value=\"...\"/> in Individual XML block.")
    return float(m.group("val"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("logfile", help="Path to log.txt")
    ap.add_argument(
        "-o",
        "--outdir",
        default=".",
        help="Output directory for best_<gen>.txt files (default: current dir)",
    )
    args = ap.parse_args()

    log_path = Path(args.logfile)
    out_dir = Path(args.outdir)
    out_dir.mkdir(parents=True, exist_ok=True)

    text = log_path.read_text(encoding="utf-8", errors="replace")

    last_saved_fitness = float("inf")
    saved = 0

    for m in BEST_BLOCK_RE.finditer(text):
        gen = int(m.group("gen"))
        xml = m.group("xml").strip()

        try:
            fitness = extract_fitnessmin(xml)
        except ValueError:
            continue

        if fitness < last_saved_fitness:
            out_file = out_dir / f"best_{gen}.txt"
            out_file.write_text(xml + "\n", encoding="utf-8")
            last_saved_fitness = fitness
            saved += 1

    print(f"Saved {saved} files to: {out_dir.resolve()}")


if __name__ == "__main__":
    raise SystemExit(main())
