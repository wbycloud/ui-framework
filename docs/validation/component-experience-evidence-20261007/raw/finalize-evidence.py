from pathlib import Path
import hashlib, json, shutil, subprocess, xml.etree.ElementTree as ET

root=Path.cwd().resolve(); work=root/'build/experience-20261007'
evidence=root/'docs/validation/component-experience-evidence-20261007'
assert evidence.is_relative_to(root)
def inventory():
    items=[]
    for path in sorted(evidence.rglob('*')):
        if path.is_file() and path != evidence/'sha256.json':
            items.append({'path':path.relative_to(evidence).as_posix(),'bytes':path.stat().st_size,'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
    (evidence/'sha256.json').write_text(json.dumps({'algorithm':'SHA256','excluded_self':'sha256.json','files':items},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    return items
inventory()
subprocess.run([r'C:/Users/wbycl/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe',str(work/'delivery-check.py')],check=True)
shutil.copy2(work/'delivery-check.log',evidence/'raw/delivery-check.log')
shutil.copy2(work/'finalize-evidence.py',evidence/'raw/finalize-evidence.py')
results={}
for name, expected in [('framework-delivery-final',(55,0,1)),('native-8078281',(20,0,1)),('business-8078281',(29,1,0)),('sdk-offline-tests',(3,0,0))]:
    tree=ET.parse(work/(name+'.xml')); cases=list(tree.getroot().iter('testcase'))
    fail=sum(c.find('failure') is not None or c.find('error') is not None for c in cases)
    skip=sum(c.find('skipped') is not None for c in cases)
    actual=(len(cases)-fail-skip,fail,skip)
    assert actual==expected,(name,actual)
    results[name]={'passed':actual[0],'failed':actual[1],'skipped':actual[2]}
for name in ['runtime32-1.log','runtime32-2.log']:
    log=(work/name).read_text(encoding='utf-8')
    assert log.count('API7 integration: reopen ')==32
    assert '0 failures' in log and 'pending Runtime cleanup=0' in log
assert 'REGRESSION_RESULT: 102 checks 0 failures' in (work/'business-real-scroll-pixels.log').read_text(encoding='utf-8')
(evidence/'results.json').write_text(json.dumps({'source':'original JUnit and actual logs, not filenames','current':results,'runtime_ready_cycles':[32,32],'business_framework_observer_checks':102,'original_business_assertions_modified':False,'strict_service_session0':'pending','remote_ci_run':False},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
items=inventory()
for item in items:
    assert hashlib.sha256((evidence/item['path']).read_bytes()).hexdigest()==item['sha256']
print('Evidence hashes PASS',len(items),'files; actual results',results)
