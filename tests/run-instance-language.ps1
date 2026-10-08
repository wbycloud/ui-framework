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
$manifest=@{backend=$Backend;frameworkApi=9;input='real SendInput';provider=$providerPath;evidence=$runPath;startedUtc=[DateTime]::UtcNow.ToString('o');files=@()}
foreach($file in @($providerPath,$executable,$package,(Join-Path $buildPath 'ui_framework.dll'))){$manifest.files+=@{path=$file;sha256=(Get-FileHash -LiteralPath $file).Hash}}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $runPath 'manifest.json') -Encoding utf8
$preferences=Join-Path $runPath 'preferences'
New-Item -ItemType Directory -Path $preferences | Out-Null
& $executable $package $providerPath $Backend $preferences preferences 96 2>&1 | Tee-Object -FilePath (Join-Path $runPath 'preferences.log')
if($LASTEXITCODE -ne 0){throw "Complete DLL preference validation failed: exit $LASTEXITCODE"}
foreach($dpi in @(96,144,192)) {
    $profilePath=Join-Path $runPath ('dpi-'+$dpi)
    New-Item -ItemType Directory -Path $profilePath | Out-Null
    foreach($mode in @('write','restore')) {
        & $executable $package $providerPath $Backend $profilePath $mode $dpi 2>&1 | Tee-Object -FilePath (Join-Path $runPath ($mode+'-'+$dpi+'.log'))
        if($LASTEXITCODE -ne 0){throw "Actual language case failed: $mode dpi$dpi exit $LASTEXITCODE"}
    }
}
Write-Output "LANGUAGE_EVIDENCE=$runPath"
