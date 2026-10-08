$ErrorActionPreference='Stop'
$roundPath=(Resolve-Path 'build/stability-state-20261008').Path
$buildPath=Join-Path $roundPath 'after'
$providerPath=(Resolve-Path '.deps/mesa-24.3.4/x64/osmesa.dll').Path
$ctestPath='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
$env:GALLIUM_DRIVER='llvmpipe'
$env:UI_RUNTIME_CYCLES='32';$env:UI_RUNTIME_READY_REOPENS='1'
try{foreach($run in 1,2){& "$buildPath/ui_api7_integration_test.exe" "$buildPath/api7_fixture.uapp" $providerPath webview2 *> "$roundPath/runtime32-$run.log";if($LASTEXITCODE){throw "Runtime independent $run failed"}}}finally{Remove-Item Env:UI_RUNTIME_CYCLES,Env:UI_RUNTIME_READY_REOPENS}
foreach($run in 1,2){& "$buildPath/ui_host_repaint_test.exe" "$buildPath/api7_fixture.uapp" $providerPath *> "$roundPath/repaint-independent-$run.log";if($LASTEXITCODE){throw "Repaint independent $run failed"}}
$nativePath=Join-Path $roundPath 'native'
foreach($name in @('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')){Copy-Item -LiteralPath (Join-Path '.deps/mesa-24.3.4/x64' $name) -Destination $nativePath}
& $ctestPath --test-dir $nativePath --output-on-failure --output-junit "$roundPath/native-current.xml" *> "$roundPath/native-current.log"
if($LASTEXITCODE){throw 'Current native matrix failed'}
$businessUi=Join-Path $roundPath 'business-ui'
Copy-Item -LiteralPath "$buildPath/ui_framework.dll","$buildPath/business-observe.exe","$roundPath/business-current/business-scroll-observation.exe" -Destination $businessUi
Copy-Item -LiteralPath "$roundPath/business-source/tests/reference/api6-layout-format1.kclayout" -Destination "$businessUi/layout-format1.kclayout"
& "$businessUi/business-observe.exe" "$roundPath/business-current/klayoutc.uapp" "$roundPath/business-source/tests/reference/precision/precision-official.gds" "$roundPath/business-observation-final" *> "$roundPath/business-fold-final.log"
if($LASTEXITCODE){throw 'Current business fold observer failed'}
Push-Location $businessUi
try{& './business-scroll-observation.exe' "$roundPath/business-current/klayoutc.uapp" --workspace-input *> "$roundPath/business-scroll-final.log";if($LASTEXITCODE){throw 'Business real scroll/layout observer failed'}}finally{Pop-Location}
& $ctestPath --test-dir "$roundPath/business-current" --output-on-failure --output-junit "$roundPath/business-product-final.xml" *> "$roundPath/business-product-final.log"
[xml]$result=Get-Content -LiteralPath "$roundPath/business-product-final.xml" -Raw
$failed=@($result.testsuite.testcase | Where-Object {$_.SelectSingleNode('failure')})
if($result.testsuite.tests -ne '30' -or $failed.Count -ne 1 -or $failed[0].name -ne 'kc_host'){throw 'Business full suite differs from preserved 29/30; diagnose before delivery'}
Write-Output 'Independent Runtime/repaint/native/business observations complete; original business kc_host failure retained'
