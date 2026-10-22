param([Parameter(Mandatory=$true)][string]$SdkDir,[string]$BuildDir='build/sdk-only')
$ErrorActionPreference='Stop'
if (!$env:VSCMD_VER) {
    $lines=& cmd.exe /d /c "$PSScriptRoot\msvc-env.cmd"
    foreach($line in $lines){if($line -match '^([^=]+)=(.*)$'){[Environment]::SetEnvironmentVariable($matches[1],$matches[2],'Process')}}
}
& cmake -S "$PSScriptRoot/.." -B $BuildDir -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release' "-DUI_BROWSER_SDK_DIR=$SdkDir" '-DBROWSER_BUILD_CHECKS=OFF'
if($LASTEXITCODE){exit $LASTEXITCODE}
& cmake --build $BuildDir --target browser_package
exit $LASTEXITCODE
