$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path 'build/ui-repair-20261009').Path
$binaryDir=New-Item -ItemType Directory -Path "$taskRoot/accepted-07-bin"
foreach($name in @('ui_interaction_probe.exe','ui_display_probe.exe')){Copy-Item -LiteralPath "$taskRoot/baseline/$name" -Destination "$binaryDir/$name"}
$sourcePaths=@('tests/interaction_probe.c','tests/display_probe.c','include/ui_framework/components.h','include/ui_framework/menus.h','src/components.c','src/components.html','src/menus.c','src/backends/light_web.c','src/backends/light_text.inc','src/backends/light_scroll.inc','src/platform/win32_shell.c')
$files=@($sourcePaths | ForEach-Object {@{path=$_;sha256=(Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash.ToLowerInvariant()}})
$files+=@(Get-ChildItem -LiteralPath $binaryDir -File | ForEach-Object {@{path=$_.FullName;bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}})
@{productCommit=(git rev-parse HEAD);input='Windows SendInput OS input path; not hardware/manual acceptance';captureLoss='controlled SetCapture/ReleaseCapture after native mouse down';sources=$files} | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 "$taskRoot/accepted-07-provenance.json"
foreach($mode in @('form','form-webview2','float','float-form','display')){
 $evidence=New-Item -ItemType Directory -Path "$taskRoot/accepted-07-$mode"
 $started=[DateTime]::UtcNow.ToString('o')
 if($mode -eq 'display'){& "$binaryDir/ui_display_probe.exe" $evidence.FullName *> "$taskRoot/accepted-07-$mode.log"}else{& "$binaryDir/ui_interaction_probe.exe" $evidence.FullName $mode *> "$taskRoot/accepted-07-$mode.log"}
 $exit=$LASTEXITCODE
 @{mode=$mode;exit=$exit;started=$started;finished=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json -Compress | Add-Content -Encoding UTF8 "$taskRoot/accepted-07-exits.jsonl"
 Write-Output "$mode exit=$exit"
 if($exit -ne 0){throw "$mode probe failed: $exit"}
}
