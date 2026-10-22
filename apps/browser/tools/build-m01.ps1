param([string]$BuildDir='build/m01-sdk',[string]$SdkDir="$PSScriptRoot/../deliverables/browser-sdk-20261020",[string[]]$Targets=@('browser_package','browser_document_check','browser_url_check','browser_workspace_check','browser_failure_check','browser_m01_check','browser_settings_check'))
$ErrorActionPreference='Stop'
if (!$env:VSCMD_VER) {
    $lines=& cmd.exe /d /c "$PSScriptRoot/msvc-env.cmd"
    foreach($line in $lines){if($line -match '^([^=]+)=(.*)$'){[Environment]::SetEnvironmentVariable($matches[1],$matches[2],'Process')}}
}
$sdk=(Resolve-Path -LiteralPath $SdkDir).Path
& cmake -S "$PSScriptRoot/.." -B $BuildDir -G Ninja '-DCMAKE_BUILD_TYPE=Release' "-DUI_BROWSER_SDK_DIR=$sdk" '-DBROWSER_BUILD_CHECKS=ON' '-DBROWSER_DIAGNOSTIC_SYMBOLS=OFF'
if($LASTEXITCODE){exit $LASTEXITCODE}
& cmake --build $BuildDir --target @Targets
exit $LASTEXITCODE
