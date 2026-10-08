$ErrorActionPreference='Stop'
. ./tools/windows-ci-checks.ps1
$taskRoot=(Resolve-Path 'build/ui-repair-20261009').Path
$ctest='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
$env:GALLIUM_DRIVER='llvmpipe'
$mesa=(Resolve-Path '.deps/mesa-24.3.4/x64').Path
$dlls=@('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')
$summary=@()
foreach($configuration in @('native','light','osmesa','webview2')){
    $folder=if($configuration -eq 'webview2'){'baseline'}else{$configuration}
    $tree=(Resolve-Path "$taskRoot/$folder").Path
    $evidence=New-Item -ItemType Directory -Path "$taskRoot/matrix-$configuration"
    $plan=Get-Content -Raw "$taskRoot/$folder-plan.json" | ConvertFrom-Json
    $expected=@($plan.tests.name)
    foreach($dll in $dlls){Copy-Item -LiteralPath "$mesa/$dll" -Destination "$tree/$dll"}
    $phases=if($configuration -eq 'webview2'){@('wgl','provider','runtime')}elseif($configuration -eq 'osmesa'){@('wgl','provider')}else{@('all')}
    $combined=[xml]'<testsuite name="Local current acceptance" tests="0" failures="0" skipped="0"/>'
    $exits=@();$watch=[Diagnostics.Stopwatch]::StartNew()
    foreach($phase in $phases){
        if($phase -eq 'provider'){foreach($dll in $dlls){Remove-Item -LiteralPath "$tree/$dll"}}
        $excluded=if($configuration -eq 'webview2'){"$UiCiRuntimeTests|$UiCiProviderTests"}else{$UiCiProviderTests}
        $filter=if($phase -eq 'wgl'){@('-E',$excluded)}elseif($phase -eq 'provider'){@('-R',$UiCiProviderTests)}elseif($phase -eq 'runtime'){@('-R',$UiCiRuntimeTests)}else{@()}
        $started=[DateTime]::UtcNow.ToString('o')
        & $ctest --test-dir $tree @filter --no-tests=error --output-on-failure --output-junit "$evidence/ctest-$phase.xml" *> "$evidence/regression-$phase.log"
        $exit=$LASTEXITCODE
        Copy-Item -LiteralPath "$tree/Testing/Temporary/LastTest.log" -Destination "$evidence/test-output-$phase.log"
        $exits+=@{phase=$phase;exit=$exit;started=$started;finished=[DateTime]::UtcNow.ToString('o')}
        [xml]$part=Get-Content -Raw "$evidence/ctest-$phase.xml"
        foreach($test in $part.testsuite.testcase){[void]$combined.testsuite.AppendChild($combined.ImportNode($test,$true))}
        foreach($attribute in @('tests','failures','skipped')){$combined.testsuite.SetAttribute($attribute,[string]([int]$combined.testsuite.GetAttribute($attribute)+[int]$part.testsuite.GetAttribute($attribute)))}
        Write-Output "$configuration/$phase exit=$exit tests=$($part.testsuite.tests) failures=$($part.testsuite.failures) skipped=$($part.testsuite.skipped)"
    }
    if($configuration -in @('native','light')){foreach($dll in $dlls){Remove-Item -LiteralPath "$tree/$dll"}}
    $combined.Save("$evidence/ctest.xml")
    $gate='PASS';try{Assert-UiCiResults $combined $expected $configuration}catch{$gate=$_.Exception.Message}
    $summary+=@{configuration=$configuration;seconds=$watch.Elapsed.TotalSeconds;tests=[int]$combined.testsuite.tests;failures=[int]$combined.testsuite.failures;skipped=[int]$combined.testsuite.skipped;gate=$gate;phases=$exits}
    $summary | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 "$taskRoot/matrix-summary.json"
    Write-Output "$configuration exact-inventory gate=$gate"
}
