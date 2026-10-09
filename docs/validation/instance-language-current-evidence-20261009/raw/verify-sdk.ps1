$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path 'build/language-current-20261009').Path
$current=(Resolve-Path "$taskRoot/current").Path
$sdk=(Resolve-Path "$taskRoot/sdk-extracted/sdk").Path
$provider=(Resolve-Path "$sdk/bin/osmesa.dll").Path
$results=@()
foreach($backend in @('light','webview2')){
    $taskProfileDirectory=New-Item -ItemType Directory -Path "$taskRoot/confirmation-$backend" -Force
    & "$current/confirmation-observer.exe" "$current/stateful_components.uapp" $provider $backend $taskProfileDirectory.FullName *> "$taskRoot/confirmation-$backend.log"
    $exit=$LASTEXITCODE
    $results+=@{case='complete DLL confirmation owner/open-time snapshot';backend=$backend;exit=$exit;home=$taskProfileDirectory.FullName}
    $results | ConvertTo-Json -Depth 5 | Set-Content -Encoding UTF8 "$taskRoot/sdk-actual-results.json"
    Write-Output "CONFIRMATION $backend exit=$exit"
    if($exit){throw 'Confirmation snapshot failed; preserve original log'}
}
foreach($backend in @('light','webview2')){
    $started=[DateTime]::UtcNow.ToString('o')
    & pwsh -NoProfile -File "$sdk/source/tests/run-instance-language.ps1" -BuildDirectory "$sdk/bin" -Provider $provider -Backend $backend *> "$taskRoot/sdk-language-$backend.log"
    $exit=$LASTEXITCODE
    $results+=@{case='extracted actual DLL/package complete language script';backend=$backend;exit=$exit;started=$started;finished=[DateTime]::UtcNow.ToString('o')}
    $results | ConvertTo-Json -Depth 5 | Set-Content -Encoding UTF8 "$taskRoot/sdk-actual-results.json"
    Write-Output "EXTRACTED_LANGUAGE $backend exit=$exit"
    if($exit){throw 'Extracted complete application failed; preserve original log'}
}
