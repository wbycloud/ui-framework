from pathlib import Path
from PIL import Image
import hashlib, json, shutil, subprocess, collections

root = Path.cwd().resolve()
work = root / 'build/experience-20261007'
evidence = root / 'docs/validation/component-experience-evidence-20261007'
assert work.is_relative_to(root) and evidence.is_relative_to(root)
raw = evidence / 'raw'
raw.mkdir(parents=True, exist_ok=True)
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
screenshots = json.loads((evidence / 'screenshots.json').read_text(encoding='utf-8'))
for item in screenshots:
    source = root / item['bmp_source']
    target = evidence / 'screenshots' / item['png']
    with Image.open(source) as img:
        img.convert('RGB').save(target, format='PNG')
        item['size'] = list(img.size)
    item['bmp_sha256'] = sha(source)
    item['png_sha256'] = sha(target)
    failed = item['png'].startswith('before-runtime-') and not item['png'].startswith('before-runtime-desktop-')
    item['accepted_baseline'] = item['phase'] == 'before' and not failed
    item['classification'] = 'failed-restricted-runtime-capture' if failed else ('paired-framework-baseline' if item['phase'] == 'before' else 'actual-current-capture')
(evidence / 'screenshots.json').write_text(json.dumps(screenshots, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
for source in sorted(work.iterdir()):
    if source.is_file() and source.suffix.lower() in {'.log', '.xml', '.json', '.cmd', '.ps1', '.py', '.c', '.inc'}:
        shutil.copy2(source, raw / source.name)

frozen = json.loads((work / 'delivery-frozen-hashes.json').read_text(encoding='utf-8'))
for item in frozen['artifacts']:
    assert sha(root / item['path']) == item['sha256'], item['path']
extra = []
for folder, extensions in [(root/'include/ui_framework',{'.h'}), (work/'native',{'.exe','.dll','.uapp'}), (work/'business-after',{'.exe','.dll','.uapp'})]:
    for source in sorted(folder.glob('*')):
        if source.is_file() and source.suffix.lower() in extensions:
            extra.append({'path':source.relative_to(root).as_posix(),'bytes':source.stat().st_size,'sha256':sha(source)})
identity = {
    'sdk':'0.8.0-dev', 'api':8, 'standard':8, 'abi':1, 'package':1,
    'baseline_commit':'c62753d55b5f97d97a0a9acba0b0ec93e350b9c7',
    'product_commit':frozen['product_commit'], 'test_commit':frozen['test_commit'],
    'delivery_kind':'local_unpublished',
    'final_artifacts':frozen['artifacts'], 'additional_artifacts':extra,
    'source_business_binding':json.loads((work/'business-source-binding.json').read_text(encoding='utf-8')),
    'screenshot_counts':dict(collections.Counter(s['classification'] for s in screenshots)),
    'original_business_tracked_status':subprocess.check_output(['git','-C',r'D:\应用软件框架\klayoutC','status','--porcelain','--untracked-files=no']).decode('utf-8'),
    'unchanged_native_backend':subprocess.check_output(['git','diff','c62753d','HEAD','--','src/backends/light_web.c']).decode('utf-8') == '',
}
assert identity['original_business_tracked_status'] == ''
assert identity['unchanged_native_backend']
(evidence/'artifact-identity.json').write_text(json.dumps(identity, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
print(json.dumps({'raw_files':len(list(raw.iterdir())),'screenshots':len(screenshots),'classifications':identity['screenshot_counts'],'frozen_hashes':'PASS','original_tracked_status':'clean'},ensure_ascii=False))
