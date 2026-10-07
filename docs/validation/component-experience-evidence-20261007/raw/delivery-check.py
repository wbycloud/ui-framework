from pathlib import Path
from urllib.parse import unquote
import hashlib, json, re, subprocess, zipfile

root = Path.cwd().resolve(); work = root/'build/experience-20261007'
evidence = root/'docs/validation/component-experience-evidence-20261007'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
lock = work/'business-source/framework-sdk.lock.json'
data = json.loads(lock.read_text(encoding='utf-8'))
data['source_availability'] = 'Local product807828147bdfb6959abb49b4668dfa90c34eda56, test0fe08640c3008874449dce77fd7445ef91f6a578; SDK0.8/API8/standard8. Preserve native scrollbar fixes9df202f/2e87e02. Rebuild matched current headers/import library/shared DLL/host; no original application repository changes. Framework observation is not complete business acceptance.'
lock.write_text(json.dumps(data, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
changed = []
with zipfile.ZipFile(work/'business-source-original.zip') as z:
    for item in z.infolist():
        if not item.is_dir():
            current = work/'business-source'/item.filename
            assert current.is_file(), item.filename
            if current.read_bytes() != z.read(item): changed.append(item.filename)
assert sorted(changed) == ['app/manifest.ini','framework-sdk.lock.json'], changed
(evidence/'raw/business-copy-lock.json').write_text(json.dumps(data, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')

ctest = r'C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
plan = {
 'kind':'actual-local-execution-record', 'product_commit':'807828147bdfb6959abb49b4668dfa90c34eda56', 'test_commit':'0fe08640c3008874449dce77fd7445ef91f6a578',
 'desktop_execution':'Windows Session1, real Runtime/browser processes require normal desktop approval; all GUI runs serial',
 'framework_build': 'cmd /c build\\experience-20261007\\build.cmd',
 'framework_full': [ctest,'--test-dir','build/experience-20261007/after','--output-on-failure','-j','1','--output-junit','build/experience-20261007/framework-delivery-final.xml'],
 'native_build':'cmd /c build\\experience-20261007\\native-build.cmd',
 'native_tests':[ctest,'--test-dir','build/experience-20261007/native','--output-on-failure','-j','1','--output-junit','build/experience-20261007/native-8078281.xml'],
 'business_build':'cmd /c build\\experience-20261007\\business-after-build.cmd',
 'business_full':[ctest,'--test-dir','build/experience-20261007/business-after','--output-on-failure','-j','1','--output-junit','build/experience-20261007/business-8078281.xml'],
 'business_real_mouse':{'cwd':'build/experience-20261007/business-after','argv':['business-scroll-observation.exe','klayoutc.uapp','--workspace-input'],'source_commit':'abc8d870efc6401dc4fa55e50170a5449c11e36b','result':'102 checks, zero failures','copy_changes':changed},
 'runtime_independent_runs':{'count':2,'environment':{'UI_RUNTIME_CYCLES':'32','UI_RUNTIME_READY_REOPENS':'1'},'argv':['build/experience-20261007/after/ui_api7_integration_test.exe','build/experience-20261007/after/api7_fixture.uapp','.deps/mesa-24.3.4/x64/osmesa.dll','webview2'],'logs':['runtime32-1.log','runtime32-2.log']},
 'framework_capture':{'observer':'api8-rich-observer.c','argv':['build/experience-20261007/after/api8-rich-observer.exe','build/experience-20261007/after/api7_fixture.uapp','.deps/mesa-24.3.4/x64/osmesa.dll','light|webview2','build/experience-20261007/capture/light|webview2'],'result':'actual host/DLL/OSMesa, zero failures per backend'},
 'business_capture':{'argv':['build/experience-20261007/after/business-observe.exe','build/experience-20261007/business-after/klayoutc.uapp','build/experience-20261007/business-source/tests/reference/precision/precision-official.gds','build/experience-20261007/capture/current'],'note':'actual read-only data open through existing permission confirmation, four shapes, modified=false; current observation only'},
 'ci_local_check':['powershell','-NoProfile','-File','tools/test-windows-ci.ps1'],
 'sdk_consumer_build':'cmd /c build\\experience-20261007\\sdk-consumer-build.cmd',
 'sdk_offline_build':'cmd /c build\\experience-20261007\\sdk-offline-build.cmd',
 'sdk_offline_tests':[ctest,'--test-dir','build/experience-20261007/sdk-offline-rebuild','-R','^(ui_public_headers_c|ui_component_widths|ui_component_experience)$','--output-on-failure','-j','1'],
 'remote_execution':False, 'current_session0_service':'unrun, SCM access denied error5', 'strict_no_login_runner':'unavailable'
}
(evidence/'raw/delivery-run-plan.json').write_text(json.dumps(plan,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

tracked = subprocess.check_output(['git','ls-files','-z']).decode('utf-8').split('\0')
docs = [root/name for name in tracked if name.endswith('.md')]
docs += [root/'docs/migration-v0.7-to-v0.8.md',root/'docs/validation/component-experience-validation.md',evidence/'README.md']
missing=[]; checked=0
for path in docs:
    source = path.read_text(encoding='utf-8-sig')
    source = re.sub(r'(?ms)^\s*(```|~~~).*?^\s*\1\s*$', '', source)
    for match in re.finditer(r'\[[^\]\n]*\]\(([^\n]+?)\)',source):
        link = match.group(1).strip().split(' "')[0].strip('<>')
        if re.match(r'^[a-zA-Z][a-zA-Z0-9+.-]*:',link) or link.startswith(('#','/','\\')):continue
        target = unquote(link.split('#')[0].split('?')[0])
        if not target:continue
        checked+=1
        if not (path.parent/target).exists():missing.append({'file':path.relative_to(root).as_posix(),'target':target})
assert missing in [[], [{'file':'docs/validation/component-experience-evidence-20261007/README.md','target':'sha256.json'}]], missing
screens = json.loads((evidence/'screenshots.json').read_text(encoding='utf-8'))
for item in screens:
    assert sha(evidence/'screenshots'/item['png']) == item['png_sha256']
    assert sha(root/item['bmp_source']) == item['bmp_sha256']
report={'relative_links_checked':checked,'unresolved_links':missing,'evidence_manifest_pending':not (evidence/'sha256.json').is_file(),'business_copy_changed_only':changed,'screenshot_hashes_checked':len(screens),'frozen_product_validation':'see collect-evidence.py'}
(work/'delivery-check.log').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))
