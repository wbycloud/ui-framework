param([string]$BuildDir='build/m2-production',[string]$FrameworkDir='D:/应用软件框架/应用层序框架')
$ErrorActionPreference='Stop'
$lines=& cmd.exe /d /c "$PSScriptRoot/msvc-env.cmd"
foreach($line in $lines){if($line -match '^([^=]+)=(.*)$'){[Environment]::SetEnvironmentVariable($matches[1],$matches[2],'Process')}}
& cmake -S $FrameworkDir -B $BuildDir -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DUI_BUILD_TESTS=OFF' '-DBROWSER_BUILD_CHECKS=OFF' '-DBROWSER_DIAGNOSTIC_SYMBOLS=OFF' '-DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF' '-DUI_OSMESA_LIBRARY='
if($LASTEXITCODE){exit $LASTEXITCODE}
& cmake --build $BuildDir --target framework_host browser_package uapp_pack
exit $LASTEXITCODE
