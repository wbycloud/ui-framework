"""Verify delivered bytes and acceptance records, without rebuilding evidence."""
import argparse
import hashlib
import json
from pathlib import Path
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser()
parser.add_argument("--with-local-originals", action="store_true")
args = parser.parse_args()
root = Path(__file__).resolve().parent
repo = root.parents[2]
errors = []


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def verify(path, digest, length=None):
    if not path.is_file():
        errors.append(f"missing: {path}")
        return
    if length is not None and path.stat().st_size != length:
        errors.append(f"size: {path}")
    if hashlib.sha256(path.read_bytes()).hexdigest() != digest.lower():
        errors.append(f"SHA256: {path}")


index = read(root / "evidence-sha256.json")
for item in index["files"]:
    verify(root / item["path"], item["sha256"], item["bytes"])
images = read(root / "screenshots.json")
for item in images:
    verify(root / item["derived"], item["derivedSha256"])
    if args.with_local_originals:
        verify(repo / (item.get("raw") or item["source"]),
               item.get("rawSha256") or item["sourceSha256"],
               item.get("rawBytes"))
if args.with_local_originals:
    for item in read(root / "raw/copied-file-identities.json"):
        verify(repo / item["source"], item["sha256"])

tests = failures = skipped = 0
for folder in ("matrix-native", "matrix-final-light", "matrix-final-osmesa", "matrix-final-webview2"):
    suite = ET.parse(root / "raw" / folder / "ctest.xml").getroot()
    tests += int(suite.get("tests"))
    failures += int(suite.get("failures"))
    skipped += int(suite.get("skipped"))
    for case in suite.findall("testcase"):
        if case.find("skipped") is not None and case.get("name") != "ui_monitor_transition":
            errors.append(f"unexpected skip: {folder}/{case.get('name')}")
if (tests, failures, skipped) != (185, 0, 4):
    errors.append(f"JUnit counts: {tests}/{failures}/{skipped}")
runs = read(root / "raw/script-runs-index.json")
for run in runs:
    cases = read(root / run["portableDirectory"] / "manifest.json")["cases"]
    if len(cases) != run["cases"] or any(c["status"] != "exited" or c["exitCode"] != 0 for c in cases):
        errors.append(f"incomplete process: {run['directory']}")
if len(runs) != 6:
    errors.append(f"script groups: {len(runs)}")
result = dict(files=len(index["files"]), images=len(images), tests=tests,
              failures=failures, skipped=skipped, scriptGroups=len(runs),
              localOriginals=args.with_local_originals, errors=errors)
print(json.dumps(result, ensure_ascii=True, indent=2))
raise SystemExit(1 if errors else 0)
