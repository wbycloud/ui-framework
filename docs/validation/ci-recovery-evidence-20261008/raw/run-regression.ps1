param([string]$BuildDirectory,[string]$EvidenceDirectory)
$ErrorActionPreference='Stop'
. './tools/windows-ci-checks.ps1'
$ctest='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
New-Item -ItemType Directory -Path $EvidenceDirectory | Out-Null
$buildPath=(Resolve-Path -LiteralPath $BuildDirectory).Path
$evidencePath=(Resolve-Path -LiteralPath $EvidenceDirectory).Path
$env:GALLIUM_DRIVER='llvmpipe'
foreach($name in @('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')){Copy-Item -LiteralPath (Join-Path '.deps/mesa-24.3.4/x64' $name) -Destination $buildPath}
& $ctest --test-dir $buildPath --show-only=json-v1 > (Join-Path $evidencePath 'test-plan.json')
$plan=Get-Content (Join-Path $evidencePath 'test-plan.json') -Raw | ConvertFrom-Json
$combined=[xml]'<testsuite name="Local API9 recovery" tests="0" failures="0" skipped="0"/>'
$failed=0
foreach($phase in @('wgl','provider','runtime')){
    if($phase -eq 'provider'){
        foreach($name in @('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')){Remove-Item -LiteralPath (Join-Path $buildPath $name)}
    }
    $filter=if($phase -eq 'wgl'){@('-E',"$UiCiRuntimeTests|$UiCiProviderTests")}elseif($phase -eq 'provider'){@('-R',$UiCiProviderTests)}else{@('-R',$UiCiRuntimeTests)}
    & $ctest --test-dir $buildPath -j 1 @filter --no-tests=error -V --output-junit (Join-Path $evidencePath ($phase+'.xml')) *> (Join-Path $evidencePath ($phase+'.log'))
    if($LASTEXITCODE){$failed=$LASTEXITCODE}
    Copy-Item -LiteralPath (Join-Path $buildPath 'Testing/Temporary/LastTest.log') -Destination (Join-Path $evidencePath ($phase+'-last-test.log'))
    [xml]$part=Get-Content (Join-Path $evidencePath ($phase+'.xml')) -Raw
    foreach($test in $part.testsuite.testcase){[void]$combined.testsuite.AppendChild($combined.ImportNode($test,$true))}
    foreach($attribute in @('tests','failures','skipped')){$combined.testsuite.SetAttribute($attribute,[string]([int]$combined.testsuite.GetAttribute($attribute)+[int]$part.testsuite.GetAttribute($attribute)))}
}
$combined.Save((Join-Path $evidencePath 'results.xml'))
foreach($pattern in @('state-evidence-*','language-evidence-*')){Get-ChildItem -LiteralPath $buildPath -Directory -Filter $pattern | Copy-Item -Destination $evidencePath -Recurse}
if($failed){throw "Local regression exit=$failed"}
Assert-UiCiResults $combined @($plan.tests | ForEach-Object name) webview2
Write-Output "Local full API9 PASS: $($combined.testsuite.tests) inventory; $($combined.testsuite.skipped) physical skip"
