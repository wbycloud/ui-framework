from pathlib import Path
import hashlib, json, shutil, subprocess, zipfile
import xml.etree.ElementTree as ET

root = Path.cwd().resolve()
work = root / 'build/language-current-20261009'
target = root / 'docs/validation/instance-language-current-evidence-20261009'
raw = target / 'raw'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
proof_path = work / 'sdk-final-archive.json'
proof = json.loads(proof_path.read_text(encoding='utf-8-sig'))
archive = Path(proof['archive'])
sdk = Path(proof['extracted_directory'])
staged = work / 'sdk-delivery'
assert sha(archive) == proof['sha256']
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
manifest = json.loads((sdk / 'sha256.json').read_text(encoding='utf-8-sig'))
for item in manifest['files']:
    p = sdk / item['path']
    assert p.stat().st_size == item['bytes'] and sha(p) == item['sha256'], item['path']
assert sha(sdk / 'sha256.json') == sha(staged / 'sha256.json')
before = json.loads((work / 'artifacts-before.json').read_text(encoding='utf-8-sig'))
for item in before['artifacts']:
    assert sha(Path(item['path'])) == item['sha256'], item['path']
results = json.loads((work / 'sdk-final-actual-results.json').read_text(encoding='utf-8-sig'))
assert len(results) == 2 and all(r['exit'] == 0 for r in results)
suite_summary = []
for backend in ['light', 'webview2']:
    runs = list((sdk / 'bin').glob('language-evidence-' + backend + '-*'))
    assert len(runs) == 1
    run = runs[0]
    run_manifest = json.loads((run / 'manifest.json').read_text(encoding='utf-8-sig'))
    assert len(run_manifest['cases']) == 7
    assert all(c['exitCode'] == 0 and c['status'] == 'exited' for c in run_manifest['cases'])
    destination = raw / ('final-sdk-language-' + backend)
    destination.mkdir(exist_ok=False)
    for p in run.iterdir():
        if p.is_file() and p.suffix in ['.log', '.json']:
            shutil.copy2(p, destination / p.name)
    suite_summary.append({'backend': backend, 'processes': 7, 'all_actual_exits': 0,
                          'manifest': str(run / 'manifest.json')})
tests = list(ET.parse(work / 'sdk-offline-tests.xml').getroot().iter('testcase'))
assert len(tests) == 3 and all(t.find('failure') is None and t.find('skipped') is None for t in tests)
proof.update({'actual_extracted_complete_language_application': 'PASS',
              'extracted_suite_summary': suite_summary,
              'offline_source_build': 'PASS included fixed dependencies; no network fetch',
              'offline_tests': {'passed': 3, 'failed': 0, 'names': [t.attrib['name'] for t in tests]},
              'delivered_files_after_tests_and_build': 'PASS all 3602 unchanged including manifest',
              'source_product_delta_from_tested_8765882': 'NONE; docs and evidence only'})
proof_path.write_text(json.dumps(proof, ensure_ascii=False, indent=2), encoding='utf-8')
originals = [
    ('build/language-20261008/ui-framework-sdk0.9.0-dev-api9-6d88770-windows-x64.zip',
     '7c1dbffce1211b2c0654b41d2942b05adbd4c54d111e983d836c27550d8802e1'),
    ('build/ui-repair-20261009/ui-framework-sdk0.9.0-dev-api9-07a66a8-windows-x64.zip',
     'b5296fba8d39447603222705ab85edcf446fa5c98088d86c76a153a4fd9c60f0'),
    ('build/language-current-20261009/ui-framework-sdk0.9.0-dev-api9-8765882-windows-x64.zip',
     '141a09ac741f92c7d02b43ea69cef58916b4a09e99f1e23c8d39d61e99d226f8')]
preserved = []
for path, expected in originals:
    p = root / path
    assert sha(p) == expected
    preserved.append({'path': str(p), 'bytes': p.stat().st_size, 'sha256': expected, 'unchanged': True})
(work / 'sdk-final-preservation.json').write_text(json.dumps({
    'archive_unchanged_after_actual_extracted_tests': proof['sha256'],
    'delivered_files_checked': len(manifest['files']) + 1,
    'source_snapshot': proof['source_document_snapshot'],
    'historical_and_baseline_archives': preserved}, ensure_ascii=False, indent=2), encoding='utf-8')
names = ['sdk-final-archive.json', 'sdk-final-preservation.json', 'sdk-final-actual-results.json',
         'sdk-final-extracted-consumer.log', 'sdk-final-language-light.log', 'sdk-final-language-webview2.log',
         'sdk-offline-build.cmd', 'sdk-offline-configure.log', 'sdk-offline-build.log',
         'sdk-offline-tests.log', 'sdk-offline-tests.xml', 'sdk-offline-tests-command.ps1',
         'sdk-offline-target-failure.log', 'sdk-delivery-stage.log', 'sdk-delivery-package.log',
         'sdk-delivery-consumer-build.cmd', 'sdk-delivery-consumer-build.log',
         'sdk-delivery-consumer-staging.log', 'stage-sdk-delivery.py', 'package-sdk-delivery.py',
         'verify-sdk-final.ps1', 'finalize-sdk-evidence.py']
for name in names:
    assert not (raw / name).exists(), name
    shutil.copy2(work / name, raw / name)
shutil.copy2(sdk / 'identity.json', raw / 'sdk-final-identity.json')
shutil.copy2(sdk / 'README.md', raw / 'sdk-final-readme.md')
shutil.copy2(sdk / 'examples/sdk-consumer.c', raw / 'sdk-final-consumer.c')
(raw / 'sdk-final-tool-incident.md').write_text(
    '# 最终SDK离线构建目标选择失败\n\n'
    '首次构建命令将CTest用例ui_windowless_gl误写成不存在的可执行目标ui_windowless_gl_test，Ninja直接失败；'
    'sdk-offline-target-failure.log保留原始输出。改为仓库既有ui_offscreen_msaa_test目标后重新构建成功，'
    '实际CTest仍运行ui_windowless_gl并验证OSMesa。没有修改产品、超时、预算或原断言；'
    '后续通过不覆盖原目标选择失败。\n', encoding='utf-8')
print('Final SDK proof PASS; original archives and all delivered files unchanged; 14 actual exited processes, 3 offline tests.')
