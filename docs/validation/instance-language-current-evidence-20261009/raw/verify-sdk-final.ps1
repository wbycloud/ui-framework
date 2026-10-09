$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path 'build/language-current-20261009').Path
$sdk=(Resolve-Path "$taskRoot/sdk-delivery-extracted/sdk").Path
$provider=(Resolve-Path "$sdk/bin/osmesa.dll").Path
$results=@()
foreach($backend in @('light','webview2')){
    $started=[DateTime]::UtcNow.ToString('o')
    & pwsh -NoProfile -File "$sdk/source/tests/run-instance-language.ps1" -BuildDirectory "$sdk/bin" -Provider $provider -Backend $backend *> "$taskRoot/sdk-final-language-$backend.log"
    $exit=$LASTEXITCODE
    $results+=@{backend=$backend;exit=$exit;started=$started;finished=[DateTime]::UtcNow.ToString('o');dll=(Get-FileHash -LiteralPath "$sdk/bin/ui_framework.dll").Hash;package=(Get-FileHash -LiteralPath "$sdk/bin/stateful_components.uapp").Hash}
    $results | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath "$taskRoot/sdk-final-actual-results.json" -Encoding UTF8
    Write-Output "FINAL_EXTRACTED_LANGUAGE $backend exit=$exit"
    if($exit){throw 'Final SDK extracted language suite failed; original log retained'}
}