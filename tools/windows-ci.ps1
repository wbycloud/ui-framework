param(
    [Parameter(Mandatory)][ValidateSet('native','light','webview2','osmesa')][string]$Configuration,
    [Parameter(Mandatory)][ValidateSet('prepare','build','test')][string]$Stage
)
$ErrorActionPreference = 'Stop'
$buildDirectory = "build/ci-$Configuration"
$evidenceDirectory = "build/ci-evidence-$Configuration"
New-Item -ItemType Directory -Force $evidenceDirectory | Out-Null
function CheckExit([string]$operation) { if ($LASTEXITCODE) { throw "$operation failed ($LASTEXITCODE)" } }
function DownloadChecked([string]$url,[string]$path,[string]$expected) {
    Invoke-WebRequest $url -OutFile $path
    $sha = (Get-FileHash $path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($sha -ne $expected) { throw "Archive SHA256 mismatch: $path" }
    "source=$url sha256=$sha" | Add-Content "$evidenceDirectory/dependencies.log"
}
if ($Stage -eq 'prepare') {
    New-Item -ItemType Directory -Force .deps | Out-Null
    DownloadChecked 'https://github.com/pal1000/mesa-dist-win/releases/download/24.3.4/mesa3d-24.3.4-release-msvc.7z' '.deps/ci-mesa.7z' '7ebc711ad1896ac88ab21e142f1017f8ff035f0f342bdb72fbb5e2eb881ba363'
    & 7z x .deps/ci-mesa.7z '-o.deps/mesa-24.3.4' -y | Out-File "$evidenceDirectory/extraction.log"; CheckExit 'Mesa extraction'
    if ($Configuration -ne 'native') {
        foreach ($dependency in @(
            @('lexbor','https://github.com/lexbor/lexbor.git','7fb22cf5664a331d7c24b113489e566767c9c25a'),
            @('quickjs','https://github.com/quickjs-ng/quickjs.git','2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278'))) {
            & git init ".deps/$($dependency[0])"; CheckExit 'Dependency init'
            & git -C ".deps/$($dependency[0])" fetch --depth 1 $dependency[1] $dependency[2]; CheckExit 'Pinned dependency fetch'
            & git -C ".deps/$($dependency[0])" checkout --detach FETCH_HEAD; CheckExit 'Pinned dependency checkout'
            $actual = & git -C ".deps/$($dependency[0])" rev-parse HEAD; CheckExit 'Dependency HEAD'
            if ($actual -ne $dependency[2]) { throw 'Dependency commit mismatch' }
            "source=$($dependency[1]) commit=$actual" | Add-Content "$evidenceDirectory/dependencies.log"
        }
    }
    if ($Configuration -eq 'webview2') {
        DownloadChecked 'https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/1.0.4129.50/microsoft.web.webview2.1.0.4129.50.nupkg' '.deps/ci-webview2.zip' 'd3934f482d484b89fb4825df720c710664e1143a1e90f7b3a60794ef33f473d2'
        Expand-Archive .deps/ci-webview2.zip .deps/Microsoft.Web.WebView2.1.0.4129.50 -Force
        $runtimeKeys = @('HKLM:\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}', 'HKCU:\Software\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}')
        $versions = @($runtimeKeys | ForEach-Object { (Get-ItemProperty $_ -ErrorAction SilentlyContinue).pv } | Where-Object { $_ -and $_ -ne '0.0.0.0' })
        $minimumRuntime = [version]'138.0.3351.48' # ControllerOptions4 / AllowHostInputProcessing
        "before=$($versions -join ',') minimum=$minimumRuntime" | Add-Content "$evidenceDirectory/runtime.log"
        if (!($versions | Where-Object { [version]$_ -ge $minimumRuntime })) {
            # Ephemeral CI VM only. The real Runtime test is mandatory below.
            if ($env:GITHUB_ACTIONS -ne 'true') { throw 'Install a compatible WebView2 Runtime before running local preparation' }
            $url = 'https://go.microsoft.com/fwlink/p/?LinkId=2124703'
            Invoke-WebRequest $url -OutFile .deps/MicrosoftEdgeWebview2Setup.exe
            $signature = Get-AuthenticodeSignature .deps/MicrosoftEdgeWebview2Setup.exe
            if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'Microsoft Corporation') { throw 'Runtime installer signature invalid' }
            "runtime_bootstrapper=$url sha256=$((Get-FileHash .deps/MicrosoftEdgeWebview2Setup.exe).Hash)" | Add-Content "$evidenceDirectory/dependencies.log"
            $installer = Start-Process -FilePath (Resolve-Path .deps/MicrosoftEdgeWebview2Setup.exe).Path -ArgumentList '/silent','/install' -WindowStyle Hidden -Wait -PassThru
            if ($installer.ExitCode) { throw "Runtime installation failed ($($installer.ExitCode))" }
        }
        $versions = @($runtimeKeys | ForEach-Object { (Get-ItemProperty $_ -ErrorAction SilentlyContinue).pv } | Where-Object { $_ -and $_ -ne '0.0.0.0' })
        if (!($versions | Where-Object { [version]$_ -ge $minimumRuntime })) { throw "WebView2 Runtime must be at least $minimumRuntime; actual=$($versions -join ',')" }
        foreach ($key in $runtimeKeys) { Get-ItemProperty $key -ErrorAction SilentlyContinue | Select-Object pv | Out-File "$evidenceDirectory/runtime.log" -Append }
    }
    return
}
if ($Stage -eq 'build') {
    $vs = & "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe" -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'x64 MSVC tools missing' }
    $env:UI_CI_VS = "$vs/Common7/Tools/VsDevCmd.bat"
    $env:UI_CI_BUILD = $buildDirectory; $env:UI_CI_EVIDENCE = $evidenceDirectory
    $env:UI_CI_PROVIDER = if ($Configuration -in @('osmesa','webview2')) { (Resolve-Path .deps/mesa-24.3.4/x64/osmesa.dll).Path } else { '' }
    $env:UI_CI_LIGHT = if ($Configuration -eq 'native') { 'OFF' } else { 'ON' }
    $env:UI_CI_WEBVIEW2 = if ($Configuration -eq 'webview2') { 'ON' } else { 'OFF' }
    @'
@echo off
call "%UI_CI_VS%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl 2>"%UI_CI_EVIDENCE%/compiler.log"
cmake -S . -B "%UI_CI_BUILD%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=%UI_CI_LIGHT% -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=%UI_CI_LIGHT% -DUI_FRAMEWORK_ENABLE_WEBVIEW2=%UI_CI_WEBVIEW2% "-DUI_OSMESA_LIBRARY=%UI_CI_PROVIDER%" >"%UI_CI_EVIDENCE%/configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "%UI_CI_BUILD%" >"%UI_CI_EVIDENCE%/build.log" 2>&1
exit /b %errorlevel%
'@ | Set-Content "$evidenceDirectory/build.cmd" -Encoding ascii
    & cmd.exe /c (Resolve-Path "$evidenceDirectory/build.cmd").Path
    if ($LASTEXITCODE) { Get-Content "$evidenceDirectory/build.log" -Tail 100; throw 'CI build failed' }
    # Explicit software WGL deployment for the hosted VM's desktop tests.
    # Product code retains strict context requests. Nothing is installed globally.
    foreach ($dll in @('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')) { Copy-Item ".deps/mesa-24.3.4/x64/$dll" $buildDirectory }
    "WGL provider: application-local Mesa24.3.4 llvmpipe; OSMesa: explicit library; hosted logged-in desktop, not no-login acceptance" | Add-Content "$evidenceDirectory/dependencies.log"
    return
}
Add-Type @'
using System.Runtime.InteropServices;
public static class UiCiDesktop {
    [DllImport("user32.dll")] public static extern int GetSystemMetrics(int index);
    [DllImport("user32.dll")] public static extern uint GetDpiForSystem();
}
'@
$desktopBefore = @([UiCiDesktop]::GetSystemMetrics(0),[UiCiDesktop]::GetSystemMetrics(1))
if ($env:GITHUB_ACTIONS -eq 'true') {
    # Wide-window fixtures require a 1920x1080 virtual desktop. Only the
    # disposable hosted VM is changed; this is not physical-monitor evidence.
    Set-DisplayResolution -Width 1920 -Height 1080 -Force
    if ([UiCiDesktop]::GetSystemMetrics(0) -lt 1920 -or [UiCiDesktop]::GetSystemMetrics(1) -lt 1080) { throw 'Hosted GUI desktop is smaller than the required 1920x1080' }
}
$manifest = [ordered]@{
    commit = (& git rev-parse HEAD); configuration = $Configuration
    os = [Environment]::OSVersion.VersionString; processSession = (Get-Process -Id $PID).SessionId
    image = $env:ImageVersion; arch = $env:PROCESSOR_ARCHITECTURE
    desktopBefore = $desktopBefore
    desktop = @([UiCiDesktop]::GetSystemMetrics(0),[UiCiDesktop]::GetSystemMetrics(1))
    dpi = [UiCiDesktop]::GetDpiForSystem()
    wgl = 'explicit application-local Mesa24.3.4 software GL'; osmesa = 'explicit Mesa24.3.4 memory context'
    runtimeGraphics = if ($Configuration -eq 'webview2') { 'platform graphics; software WGL DLLs removed before real Runtime tests' } else { 'Runtime disabled' }
    noLoginAccepted = $false; physicalManualAccepted = $false
}
$manifest | ConvertTo-Json | Set-Content "$evidenceDirectory/manifest.json"
$env:GALLIUM_DRIVER = 'llvmpipe'
$runtimeTests = 'ui_webview2_|ui_api5_integration|ui_api5_native_host|ui_component_experience_webview2|ui_api6_integration_webview2'
$phases = if ($Configuration -eq 'webview2') { @('wgl','runtime') } else { @('all') }
$testExit = 0
'' | Set-Content "$evidenceDirectory/regression.log"
'' | Set-Content "$evidenceDirectory/test-output.log"
$combined = [xml]'<testsuite name="Windows regression" tests="0" failures="0" skipped="0"/>'
foreach ($phase in $phases) {
    if ($phase -eq 'runtime') {
        # Keep Chromium's platform graphics separate from explicit software WGL.
        # OSMesa tests still load their independently specified actual provider.
        foreach ($dll in @('opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll')) { Remove-Item -LiteralPath "$buildDirectory/$dll" }
    }
    $filter = if ($phase -eq 'wgl') { @('-E',$runtimeTests) } elseif ($phase -eq 'runtime') { @('-R',$runtimeTests) } else { @() }
    & ctest --test-dir $buildDirectory @filter --output-on-failure --output-junit "$((Resolve-Path $evidenceDirectory).Path)/ctest-$phase.xml" *> "$evidenceDirectory/regression-$phase.log"
    if ($LASTEXITCODE) { $testExit = $LASTEXITCODE }
    Get-Content "$evidenceDirectory/regression-$phase.log" | Add-Content "$evidenceDirectory/regression.log"
    Get-Content "$buildDirectory/Testing/Temporary/LastTest.log" | Add-Content "$evidenceDirectory/test-output.log"
    [xml]$part = Get-Content "$evidenceDirectory/ctest-$phase.xml" -Raw
    foreach ($test in $part.testsuite.testcase) { [void]$combined.testsuite.AppendChild($combined.ImportNode($test,$true)) }
    foreach ($attribute in @('tests','failures','skipped')) { $combined.testsuite.SetAttribute($attribute,[string]([int]$combined.testsuite.GetAttribute($attribute)+[int]$part.testsuite.GetAttribute($attribute))) }
}
$combined.Save("$((Resolve-Path $evidenceDirectory).Path)/ctest.xml")
Get-Content "$evidenceDirectory/regression.log"
Get-ChildItem $buildDirectory -Filter 'api6-*-frame.ppm' | Copy-Item -Destination $evidenceDirectory
if (Test-Path "$buildDirectory/session0-control/manifest.json") { Copy-Item "$buildDirectory/session0-control" "$evidenceDirectory/session0-interactive-control" -Recurse }
[xml]$results = Get-Content "$evidenceDirectory/ctest.xml" -Raw
foreach ($test in $results.testsuite.testcase) {
    if ($test.status -eq 'notrun' -and $test.name -ne 'ui_monitor_transition') { throw "Unexpected skipped test: $($test.name)" }
}
if ($Configuration -eq 'webview2') {
    foreach ($required in @('ui_component_experience_webview2','ui_api6_integration_webview2','ui_api5_integration','ui_webview2_render')) {
        if (!($results.testsuite.testcase | Where-Object name -eq $required)) { throw "Required real Runtime test absent: $required" }
    }
}
if ($testExit) { throw "Regression failed ($testExit)" }
