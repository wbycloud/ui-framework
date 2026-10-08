param(
    [Parameter(Mandatory=$true)][string]$BuildDirectory,
    [Parameter(Mandatory=$true)][string]$Provider,
    [ValidateSet('light','webview2')][string]$Backend='light'
)
$ErrorActionPreference='Stop'
$buildPath=(Resolve-Path -LiteralPath $BuildDirectory).Path
$providerPath=(Resolve-Path -LiteralPath $Provider).Path
$runPath=Join-Path $buildPath ('language-evidence-'+$Backend+'-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $runPath | Out-Null
$executable=Join-Path $buildPath 'ui_instance_language_test.exe'
$package=Join-Path $buildPath 'stateful_components.uapp'
$manifest=@{backend=$Backend;frameworkApi=9;input='real SendInput';provider=$providerPath;evidence=$runPath;startedUtc=[DateTime]::UtcNow.ToString('o');files=@();cases=@()}
foreach($file in @($providerPath,$executable,$package,(Join-Path $buildPath 'ui_framework.dll'))){$manifest.files+=@{path=$file;sha256=(Get-FileHash -LiteralPath $file).Hash}}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $runPath 'manifest.json') -Encoding utf8
function Run-Case([string]$profilePath,[string]$mode,[int]$dpi) {
    $label=$mode+'-'+$dpi
    $case=@{label=$label;mode=$mode;dpi=$dpi;startedUtc=[DateTime]::UtcNow.ToString('o');status='running'}
    $manifest.cases+=$case
    $manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $runPath 'manifest.json') -Encoding utf8
    Write-Output "CASE_BEGIN label=$label utc=$($case.startedUtc) evidence=$runPath"
    $watch=[Diagnostics.Stopwatch]::StartNew()
    & $executable $package $providerPath $Backend $profilePath $mode $dpi 2>&1 | Tee-Object -FilePath (Join-Path $runPath ($label+'.log'))
    $case.exitCode=$LASTEXITCODE;$case.elapsedMs=$watch.ElapsedMilliseconds;$case.status='exited';$case.completedUtc=[DateTime]::UtcNow.ToString('o')
    $manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $runPath 'manifest.json') -Encoding utf8
    Write-Output "CASE_END label=$label elapsedMs=$($case.elapsedMs) exit=$($case.exitCode)"
    if($case.exitCode -ne 0){throw "Actual language case failed: $label exit $($case.exitCode)"}
}
$preferences=Join-Path $runPath 'preferences'
New-Item -ItemType Directory -Path $preferences | Out-Null
Run-Case $preferences preferences 96
foreach($dpi in @(96,144,192)) {
    $profilePath=Join-Path $runPath ('dpi-'+$dpi)
    New-Item -ItemType Directory -Path $profilePath | Out-Null
    foreach($mode in @('write','restore')) {
        Run-Case $profilePath $mode $dpi
    }
}
$manifest.completedUtc=[DateTime]::UtcNow.ToString('o')
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $runPath 'manifest.json') -Encoding utf8
Write-Output "LANGUAGE_EVIDENCE=$runPath"
