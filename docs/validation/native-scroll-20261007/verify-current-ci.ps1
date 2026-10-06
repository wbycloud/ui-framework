$ErrorActionPreference = 'Stop'
& ./tools/test-windows-ci.ps1
. ./tools/windows-ci-checks.ps1
$ctest = 'C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
$planText = & $ctest --test-dir build/native-scroll-20261007/patched --show-only=json-v1
if ($LASTEXITCODE -ne 0) { throw 'CTest plan failed' }
$planText | Set-Content build/native-scroll-20261007/ci-final-test-plan.json -Encoding utf8
$plan = $planText | ConvertFrom-Json
[xml]$results = Get-Content -Raw build/native-scroll-20261007/framework-light-full.xml
$expected = @($plan.tests | ForEach-Object name)
Assert-UiCiResults $results $expected light
Write-Output ('CI current exact plan PASS: ' + $expected.Count + ' tests, including real native scrollbar')
[xml]$withoutNative = $results.OuterXml
$node = $withoutNative.testsuite.SelectSingleNode("testcase[@name='ui_component_scroll_native']")
$withoutNative.testsuite.RemoveChild($node) | Out-Null
$rejected = $false
try { Assert-UiCiResults $withoutNative @($expected | Where-Object { $_ -ne 'ui_component_scroll_native' }) light } catch { $rejected = $true; Write-Output $_.Exception.Message }
if (!$rejected) { throw 'Missing native scrollbar unexpectedly accepted' }
Write-Output 'CI missing native scrollbar correctly rejected'
