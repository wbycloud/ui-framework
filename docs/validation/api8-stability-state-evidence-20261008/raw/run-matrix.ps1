$ErrorActionPreference='Stop'
. ./tools/windows-ci-checks.ps1
$buildPath=(Resolve-Path -LiteralPath build/stability-state-20261008/after).Path
$roundPath=(Resolve-Path -LiteralPath build/stability-state-20261008).Path
$ctestPath='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
$env:GALLIUM_DRIVER='llvmpipe'
$allPassed=$true
$combined=[xml]'<testsuite name="Current API8 local" tests="0" failures="0" skipped="0"/>'
foreach($phase in @('wgl','provider','runtime')) {
    if($phase -eq 'wgl') {
        foreach($name in @('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')) {Copy-Item -LiteralPath (Join-Path '.deps/mesa-24.3.4/x64' $name) -Destination (Join-Path $buildPath $name)}
        $filter=@('-E',"$UiCiRuntimeTests|$UiCiProviderTests")
    } elseif($phase -eq 'provider') {
        foreach($name in @('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')) {Remove-Item -LiteralPath (Join-Path $buildPath $name)}
        $filter=@('-R',$UiCiProviderTests)
    } else {$filter=@('-R',$UiCiRuntimeTests)}
    & $ctestPath --test-dir $buildPath @filter --no-tests=error --output-on-failure --output-junit (Join-Path $roundPath ('matrix-'+$phase+'.xml')) *> (Join-Path $roundPath ('matrix-'+$phase+'.log'))
    if($LASTEXITCODE -ne 0){$allPassed=$false}
    Copy-Item -LiteralPath (Join-Path $buildPath 'Testing/Temporary/LastTest.log') -Destination (Join-Path $roundPath ('matrix-'+$phase+'-raw.log'))
    [xml]$part=Get-Content -LiteralPath (Join-Path $roundPath ('matrix-'+$phase+'.xml')) -Raw
    foreach($case in $part.testsuite.testcase){[void]$combined.testsuite.AppendChild($combined.ImportNode($case,$true))}
    foreach($attribute in @('tests','failures','skipped')){$combined.testsuite.SetAttribute($attribute,[string]([int]$combined.testsuite.GetAttribute($attribute)+[int]$part.testsuite.GetAttribute($attribute)))}
}
$combined.Save((Join-Path $roundPath 'matrix.xml'))
$plan=& $ctestPath --test-dir $buildPath --show-only=json-v1 | ConvertFrom-Json
Assert-UiCiResults $combined @($plan.tests | ForEach-Object name) webview2
if(!$allPassed){throw 'Current matrix failed; original logs retained'}
Write-Output "Current local matrix PASS $($combined.testsuite.tests) tests, skipped $($combined.testsuite.skipped)"
