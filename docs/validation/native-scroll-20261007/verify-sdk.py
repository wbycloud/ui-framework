from pathlib import Path
import zipfile,json,hashlib
base=Path(r"D:\应用软件框架\应用层序框架\build\native-scroll-20261007")
dest=base/"sdk-unpacked"
assert not dest.exists(),"fresh extraction directory required"
dest.mkdir()
with zipfile.ZipFile(base/"ui-framework-sdk0.7-api7-native-scroll-2e87e02.zip") as z:
 for info in z.infolist():
  target=(dest/info.filename).resolve()
  assert target.is_relative_to(dest.resolve()),info.filename
 z.extractall(dest)
m=json.loads((dest/"sha256.json").read_text(encoding="utf-8"))
for p,sha in m["files"].items():assert hashlib.sha256((dest/p).read_bytes()).hexdigest()==sha,p
print("EXTRACTED_FILES_VERIFIED",len(m["files"]))
