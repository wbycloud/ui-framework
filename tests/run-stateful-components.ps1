param(
    [Parameter(Mandatory=$true)][string]$BuildDirectory,
    [Parameter(Mandatory=$true)][string]$Provider,
    [ValidateSet('light','webview2')][string]$Backend='light'
)
$ErrorActionPreference='Stop'
$buildPath=(Resolve-Path -LiteralPath $BuildDirectory).Path
$providerPath=(Resolve-Path -LiteralPath $Provider).Path
$runPath=Join-Path $buildPath ('state-evidence-'+$Backend+'-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $runPath | Out-Null
$executable=Join-Path $buildPath 'ui_stateful_components_test.exe'
$package=Join-Path $buildPath 'stateful_components.uapp'
$manifest=@{backend=$Backend;frameworkApi=9;provider=$providerPath;providerSha256=(Get-FileHash -LiteralPath $providerPath).Hash;executableSha256=(Get-FileHash -LiteralPath $executable).Hash;packageSha256=(Get-FileHash -LiteralPath $package).Hash;runtimeDllSha256=(Get-FileHash -LiteralPath (Join-Path $buildPath 'ui_framework.dll')).Hash;evidence=$runPath;startedUtc=[DateTime]::UtcNow.ToString('o')}
$manifest | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $runPath 'manifest.json') -Encoding utf8
function Run-Case([string]$profilePath,[string]$mode,[int]$dpi=96,[string]$label=$mode) {
    & $executable $package $providerPath $Backend $profilePath $mode $dpi $label 2>&1 | Tee-Object -FilePath (Join-Path $runPath ($label+'.log'))
    if($LASTEXITCODE -ne 0){throw "Actual state case failed: $label exit $LASTEXITCODE"}
}
function Set-Checksum([byte[]]$bytes) {
    [uint32]$hash=2166136261
    for($i=0;$i -lt $bytes.Length;$i++){if($i -lt 20 -or $i -ge 24){$hash=[uint32](([uint64]($hash -bxor $bytes[$i])*16777619) -band 0xffffffffL)}}
    [BitConverter]::GetBytes($hash).CopyTo($bytes,20)
}
foreach($dpi in @(96,144,192)) {
    $profilePath=Join-Path $runPath ('dpi-'+$dpi)
    New-Item -ItemType Directory -Path $profilePath | Out-Null
    Run-Case $profilePath write $dpi ('write-'+$dpi)
    Run-Case $profilePath restore $dpi ('restore-'+$dpi)
}
$main=Join-Path $runPath 'dpi-96'
$stateFile=Join-Path $main 'profile-A.ust'
$valid=[IO.File]::ReadAllBytes($stateFile)
Run-Case $main isolation 96
$env:UI_STATE_EXAMPLE_PROFILE='B'
try{Run-Case $main profile-b 96}finally{Remove-Item Env:UI_STATE_EXAMPLE_PROFILE}
Run-Case $main reload 96
Run-Case $main locked-profile 96
$env:UI_STATE_EXAMPLE_SCHEMA='reorder'
try{Run-Case $main restore 96 'reordered-columns'}finally{Remove-Item Env:UI_STATE_EXAMPLE_SCHEMA}
$env:UI_STATE_EXAMPLE_SCHEMA='evolve'
try{Run-Case $main evolve 96 'missing-new-columns'}finally{Remove-Item Env:UI_STATE_EXAMPLE_SCHEMA}
foreach($fault in @('empty','truncated','oversized','magic','checksum','future','columns-reserved','columns-future','layout-future','layout-invalid')) {
    $profilePath=Join-Path $runPath ('invalid-'+$fault)
    New-Item -ItemType Directory -Path $profilePath | Out-Null
    [byte[]]$bytes=$valid.Clone()
    switch($fault) {
        empty {$bytes=[byte[]]@()}
        truncated {$bytes=[byte[]]$bytes[0..40]}
        oversized {$bytes=[byte[]]::new(70001)}
        magic {$bytes[0]=0;Set-Checksum $bytes}
        checksum {$bytes[20]=$bytes[20] -bxor 1}
        future {$bytes[4]=2;Set-Checksum $bytes}
        columns-reserved {$bytes[44]=1;Set-Checksum $bytes}
        columns-future {$bytes[36]=99;Set-Checksum $bytes}
        layout-future {$columns=[BitConverter]::ToUInt32($bytes,8);$bytes[32+$columns+4]=99;Set-Checksum $bytes}
        layout-invalid {$columns=[BitConverter]::ToUInt32($bytes,8);$bytes[32+$columns+84]=1;Set-Checksum $bytes}
    }
    [IO.File]::WriteAllBytes((Join-Path $profilePath 'profile-A.ust'),$bytes)
    Run-Case $profilePath invalid 96 $fault
}
$missing=Join-Path $runPath 'missing'
New-Item -ItemType Directory -Path $missing | Out-Null
Run-Case $missing defaults 96 'missing-file'
$initialization=Join-Path $runPath 'initialization'
New-Item -ItemType Directory -Path $initialization | Out-Null
Run-Case $initialization init-failures 96 'failed-create-mount-lock-release'
Run-Case $main reset 96
Run-Case $main defaults 96 'reset-cross-process'
$manifest.completedUtc=[DateTime]::UtcNow.ToString('o')
$manifest | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $runPath 'manifest.json') -Encoding utf8
Write-Output "Stateful application actual $Backend PASS; evidence $runPath"
