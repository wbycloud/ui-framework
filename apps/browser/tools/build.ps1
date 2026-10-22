param([string]$BuildDir = 'build/release', [switch]$ConfigureOnly, [string[]]$Targets = @(),[switch]$Symbols,[switch]$FrameworkTests)
$ErrorActionPreference = 'Stop'
if (!$env:VSCMD_VER) {
    $lines = & cmd.exe /d /c "$PSScriptRoot\msvc-env.cmd"
    foreach ($line in $lines) {
        if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
    }
}
& cmake -S "$PSScriptRoot/.." -B $BuildDir -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release' '-DUI_BUILD_TESTS=OFF' '-DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF' '-DUI_OSMESA_LIBRARY='
if ($LASTEXITCODE) { exit $LASTEXITCODE }
if ($Symbols) { & cmake -S "$PSScriptRoot/.." -B $BuildDir '-DBROWSER_DIAGNOSTIC_SYMBOLS=ON'; if($LASTEXITCODE){exit $LASTEXITCODE} }
if ($FrameworkTests) { & cmake -S "$PSScriptRoot/.." -B $BuildDir '-DUI_BUILD_TESTS=ON'; if($LASTEXITCODE){exit $LASTEXITCODE} }
if (!$ConfigureOnly) {
    if ($Targets.Count) { & cmake --build $BuildDir --target @Targets } else { & cmake --build $BuildDir }
    exit $LASTEXITCODE
}
