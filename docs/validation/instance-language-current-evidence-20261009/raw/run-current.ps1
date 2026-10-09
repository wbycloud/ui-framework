$ErrorActionPreference='Stop'
. ./tools/windows-ci-checks.ps1
$taskRoot=(Resolve-Path 'build/language-current-20261009').Path
$ctest='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
$mesa=(Resolve-Path '.deps/mesa-24.3.4/x64').Path
$env:GALLIUM_DRIVER='llvmpipe'
$results=@()
foreach($configuration in @('native','webview2')){
    $folder=if($configuration -eq 'native'){'native'}else{'current'}
    $tree=(Resolve-Path "$taskRoot/$folder").Path
    if(!$tree.StartsWith($taskRoot+[IO.Path]::DirectorySeparatorChar)){throw 'Build outside current task'}
    $evidence=New-Item -ItemType Directory -Path "$taskRoot/matrix-$configuration"
    & $ctest --test-dir $tree --show-only=json-v1 > "$evidence/test-plan.json"
    if($LASTEXITCODE){throw 'Test inventory failed'}
    $plan=Get-Content -Raw "$evidence/test-plan.json" | ConvertFrom-Json
    $expected=@($plan.tests.name)
    $dlls=@('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')
    foreach($dll in $dlls){Copy-Item -LiteralPath "$mesa/$dll" -Destination "$tree/$dll"}
    $phases=if($configuration -eq 'native'){@('all')}else{@('wgl','provider','runtime')}
    $combined=[xml]'<testsuite name="Current API9 language delivery" tests="0" failures="0" skipped="0"/>'
    $phaseResults=@();$watch=[Diagnostics.Stopwatch]::StartNew()
    foreach($phase in $phases){
        if($phase -eq 'provider'){foreach($dll in $dlls){Remove-Item -LiteralPath "$tree/$dll"}}
        $filter=if($phase -eq 'wgl'){@('-E',"$UiCiRuntimeTests|$UiCiProviderTests")}elseif($phase -eq 'provider'){@('-R',$UiCiProviderTests)}elseif($phase -eq 'runtime'){@('-R',$UiCiRuntimeTests)}else{@()}
        $started=[DateTime]::UtcNow.ToString('o')
        Write-Output "START $configuration/$phase $started"
        & $ctest --test-dir $tree @filter --no-tests=error --output-on-failure --output-junit "$evidence/ctest-$phase.xml" *> "$evidence/regression-$phase.log"
        $exit=$LASTEXITCODE
        Copy-Item -LiteralPath "$tree/Testing/Temporary/LastTest.log" -Destination "$evidence/test-output-$phase.log"
        $phaseResults+=@{phase=$phase;exit=$exit;started=$started;finished=[DateTime]::UtcNow.ToString('o')}
        [xml]$part=Get-Content -Raw "$evidence/ctest-$phase.xml"
        foreach($test in $part.testsuite.testcase){[void]$combined.testsuite.AppendChild($combined.ImportNode($test,$true))}
        foreach($attribute in @('tests','failures','skipped')){$combined.testsuite.SetAttribute($attribute,[string]([int]$combined.testsuite.GetAttribute($attribute)+[int]$part.testsuite.GetAttribute($attribute)))}
        Write-Output "END $configuration/$phase exit=$exit tests=$($part.testsuite.tests) failures=$($part.testsuite.failures) skipped=$($part.testsuite.skipped)"
    }
    if($configuration -eq 'native'){foreach($dll in $dlls){Remove-Item -LiteralPath "$tree/$dll"}}
    $combined.Save("$evidence/ctest.xml")
    $gate='PASS';try{Assert-UiCiResults $combined $expected $configuration}catch{$gate=$_.Exception.Message}
    $results+=@{configuration=$configuration;sourceCommit=(& git rev-parse HEAD);seconds=$watch.Elapsed.TotalSeconds;tests=[int]$combined.testsuite.tests;failures=[int]$combined.testsuite.failures;skipped=[int]$combined.testsuite.skipped;gate=$gate;phases=$phaseResults}
    $results | ConvertTo-Json -Depth 7 | Set-Content -Encoding UTF8 "$taskRoot/matrix-summary.json"
    Write-Output "$configuration exact-inventory gate=$gate"
}
if(@($results | Where-Object {$_.gate -ne 'PASS'}).Count){exit 1}
