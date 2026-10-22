param([Parameter(Mandatory=$true)][string]$SdkDir,[string]$BuildDir='build/m2-sdk-only',[switch]$Checks)
$ErrorActionPreference='Stop'
if(!$env:VSCMD_VER){
    $lines=& cmd.exe /d /c "$PSScriptRoot/msvc-env.cmd"
    foreach($line in $lines){if($line -match '^([^=]+)=(.*)$'){[Environment]::SetEnvironmentVariable($matches[1],$matches[2],'Process')}}
}
$sdk=(Resolve-Path -LiteralPath $SdkDir).Path
$checksFlag=if($Checks){'ON'}else{'OFF'}
& cmake -S "$PSScriptRoot/.." -B $BuildDir -G Ninja '-DCMAKE_BUILD_TYPE=Release' "-DUI_BROWSER_SDK_DIR=$sdk" "-DBROWSER_BUILD_CHECKS=$checksFlag" '-DBROWSER_DIAGNOSTIC_SYMBOLS=OFF'
if($LASTEXITCODE){exit $LASTEXITCODE}
$targets=@('browser_package')
if($Checks){$targets+=@('browser_document_check','browser_url_check','browser_settings_check','browser_encoding_check','browser_image_check','browser_reading_check','browser_m2_check','browser_m01_check','browser_failure_check','browser_workspace_check')}
& cmake --build $BuildDir --target @targets
exit $LASTEXITCODE
