$ErrorActionPreference='Stop'
$work=(Resolve-Path -LiteralPath 'build/stability-state-20261008').Path
$archive=Get-Content -LiteralPath (Join-Path $work 'sdk-archive.json') -Raw | ConvertFrom-Json
$sdk=$archive.extracted_directory
$evidence=Join-Path $work ('sdk-extracted-state-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $evidence | Out-Null
$exe=Join-Path $sdk 'bin/ui_stateful_components_test.exe'
$package=Join-Path $sdk 'bin/stateful_components.uapp'
$provider=Join-Path $sdk 'bin/osmesa.dll'
$manifest=@{startedUtc=[DateTime]::UtcNow.ToString('o');sourceSnapshot=$archive.source_document_snapshot;archiveSha256=$archive.sha256;extractedSdk=$sdk;executableSha256=(Get-FileHash -LiteralPath $exe).Hash;packageSha256=(Get-FileHash -LiteralPath $package).Hash;runtimeDllSha256=(Get-FileHash -LiteralPath (Join-Path $sdk 'bin/ui_framework.dll')).Hash;providerSha256=(Get-FileHash -LiteralPath $provider).Hash;cases=@()}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidence 'manifest.json') -Encoding utf8
foreach($backend in @('light','webview2')) {
    $profilePath=Join-Path $evidence $backend
    New-Item -ItemType Directory -Path $profilePath | Out-Null
    foreach($mode in @('write','restore','isolation','profile-b')) {
        if($mode -eq 'profile-b'){$env:UI_STATE_EXAMPLE_PROFILE='B'}
        try {
            & $exe $package $provider $backend $profilePath $mode 96 2>&1 | Tee-Object -FilePath (Join-Path $evidence ($backend+'-'+$mode+'.log'))
            if($LASTEXITCODE -ne 0){throw "Extracted SDK actual host $backend/$mode failed: $LASTEXITCODE"}
        } finally { if($mode -eq 'profile-b'){Remove-Item Env:UI_STATE_EXAMPLE_PROFILE} }
        $manifest.cases+=@{backend=$backend;mode=$mode;result='PASS';independentProcess=$true}
        $manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidence 'manifest.json') -Encoding utf8
    }
}
$manifest.completedUtc=[DateTime]::UtcNow.ToString('o')
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidence 'manifest.json') -Encoding utf8
$archive.extracted_real_host_state='PASS: 8 independent processes; Light and real Runtime write/restore/isolation/profile-B'
$archive | Add-Member -NotePropertyName extracted_real_host_evidence -NotePropertyValue $evidence
$archive | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $work 'sdk-archive.json') -Encoding utf8
Write-Output "Extracted SDK actual state: 8/8 PASS; $evidence"
