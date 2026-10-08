$ErrorActionPreference='Stop'
$workspacePath=(Get-Location).Path
$workPath=Join-Path $workspacePath 'build/language-20261008'
$proof=Get-Content -LiteralPath (Join-Path $workPath 'sdk-archive.json') -Raw -Encoding utf8 | ConvertFrom-Json
$binPath=Join-Path $proof.extracted_directory 'bin'
$providerPath=Join-Path $binPath 'osmesa.dll'
$executable=Join-Path $binPath 'ui_instance_language_test.exe'
$packagePath=Join-Path $binPath 'stateful_components.uapp'
$verifyPath=Join-Path $workPath ('sdk-language-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $verifyPath | Out-Null
$manifest=@{sdk=$proof.sha256;product_commit=$proof.product_commit;source_snapshot=$proof.source_document_snapshot;input='real SendInput, exact extracted binaries';files=@();runs=@()}
foreach($file in @($providerPath,$executable,$packagePath,(Join-Path $binPath 'ui_framework.dll'))){$manifest.files+=@{path=$file;sha256=(Get-FileHash -LiteralPath $file).Hash}}
foreach($backend in @('light','webview2')) {
 foreach($mode in @('preferences','write','restore')) {
  $dpis=if($mode -eq 'preferences'){@(96)}else{@(96,144,192)}
  foreach($dpi in $dpis) {
   $profilePath=Join-Path $verifyPath ($backend+'-'+$(if($mode -eq 'preferences'){'preferences'}else{'dpi-'+$dpi}))
   if(!(Test-Path -LiteralPath $profilePath)){New-Item -ItemType Directory -Path $profilePath | Out-Null}
   $logPath=Join-Path $verifyPath ($backend+'-'+$mode+'-'+$dpi+'.log')
   & $executable $packagePath $providerPath $backend $profilePath $mode $dpi 2>&1 | Tee-Object -FilePath $logPath
   $code=$LASTEXITCODE
   $manifest.runs+=@{backend=$backend;mode=$mode;dpi=$dpi;exit_code=$code;profile=$profilePath;log=$logPath}
   $manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $verifyPath 'manifest.json') -Encoding utf8
   if($code -ne 0){throw "Extracted actual language $backend $mode DPI$dpi failed $code"}
  }
 }
}
Set-Content -LiteralPath (Join-Path $workPath 'sdk-language-run-path.txt') -Value $verifyPath -Encoding utf8
Write-Output "SDK_EXTRACTED_FULL_HOST_LANGUAGE=$verifyPath"
