import hashlib,json,re,zipfile
from pathlib import Path
from urllib.parse import unquote,urlsplit
repo=Path(__file__).parents[3]
ev=repo/"docs/validation/visual-polish-evidence-20261007"
work=repo/"build/visual-polish-20261007"
def sha(b):return hashlib.sha256(b).hexdigest()
links=[];bad=[]
for md in [repo/"README.md",repo/"CHANGELOG.md",*sorted((repo/"docs").rglob("*.md"))]:
    for target in re.findall(r"!?\[[^\]\n]*\]\(([^)\n]+)\)",md.read_text(encoding="utf8")):
        target=target.strip().strip("<>")
        if re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:",target) or target.startswith("#"):continue
        target=unquote(target.split("#",1)[0])
        if not target:continue
        path=md.parent/target
        links.append((str(md.relative_to(repo)),target))
        if not path.exists():bad.append((str(md.relative_to(repo)),target))
identity=json.loads((ev/"artifact-identity.json").read_text(encoding="utf8"))
for n,h in identity["headers"].items():
    assert sha((repo/"include/ui_framework"/n).read_bytes())==h
    # git archive stores LF; the Windows checkout uses CRLF. Compare Git content
    # while keeping exact current artifact bytes separately hashed above.
    assert (repo/"include/ui_framework"/n).read_bytes().replace(b"\r\n",b"\n")==(work/"framework-baseline-source/include/ui_framework"/n).read_bytes().replace(b"\r\n",b"\n"),n
source=json.loads((ev/"final-source-before-commit.json").read_text(encoding="utf8"))
for n,h in source["sha256"].items():assert sha((repo/n).read_bytes())==h,n
for n,h in identity["files"].items():
    p=repo/n if n.startswith(".deps/") else work/n
    b=p.read_bytes()
    assert len(b)==h["bytes"] and sha(b)==h["sha256"],n
index=json.loads((ev/"index.json").read_text(encoding="utf8"))
b=(ev/"raw-evidence.zip").read_bytes()
assert len(b)==index["archive"]["bytes"] and sha(b)==index["archive"]["sha256"]
with zipfile.ZipFile(ev/"raw-evidence.zip") as z:
    assert z.testzip() is None
    for n,h in index["files"].items():assert sha(z.read(n))==h["sha256"],n
for n,h in index["gallery"].items():assert sha((ev/n).read_bytes())==h["png_sha256"],n
before=json.loads((ev/"business-readonly-hashes-before.json").read_text(encoding="utf8"))
after=json.loads((ev/"business-readonly-hashes-after.json").read_text(encoding="utf8"))
assert before==after
for n,h in before.items():assert sha((Path("D:/应用软件框架/klayoutC")/n).read_bytes())==h,n
result={"local_links":len(links),"broken_links":bad,"product_source_hashes":len(source["sha256"]),"unchanged_public_headers":len(identity["headers"]),"artifact_hashes":len(identity["files"]),"raw_archive_crc_sha":"PASS","raw_files":len(index["files"]),"paired_real_pngs":len(index["gallery"]),"readonly_files":len(before),"product_commit":identity["product_commit"]}
(ev/"verification.json").write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n",encoding="utf8")
print(json.dumps(result,ensure_ascii=False))
assert not bad
