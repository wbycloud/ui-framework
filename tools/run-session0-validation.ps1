param(
    [Parameter(Mandatory=$true)][string]$BuildDirectory,
    [Parameter(Mandatory=$true)][string]$OSMesaLibrary,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [switch]$RequireNoLogin
)
$ErrorActionPreference = 'Stop'
$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
$principal = [Security.Principal.WindowsPrincipal]::new($identity)
if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw 'Session0 service validation requires an elevated administrator or Windows CI service-management rights.'
}
$build = (Resolve-Path -LiteralPath $BuildDirectory).Path
$provider = (Resolve-Path -LiteralPath $OSMesaLibrary).Path
$runner = (Resolve-Path -LiteralPath (Join-Path $build 'ui_session0_gl_test.exe')).Path
$package = (Resolve-Path -LiteralPath (Join-Path $build 'session0_gl_app.uapp')).Path
$out = [IO.Path]::GetFullPath($OutputDirectory)
[void][IO.Directory]::CreateDirectory($out)
if (Test-Path -LiteralPath (Join-Path $out 'result.txt')) {
    throw 'Use a fresh output directory: existing results must not be mistaken for this service run.'
}
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$commit = (& git -C $root rev-parse HEAD).Trim()
if ($LASTEXITCODE) { throw 'Cannot record source commit.' }
$dirty = @(& git -C $root status --porcelain).Count -ne 0
$manifest = [ordered]@{
    source_commit=$commit; source_dirty=$dirty; github_sha=$env:GITHUB_SHA
    github_run_id=$env:GITHUB_RUN_ID; github_run_attempt=$env:GITHUB_RUN_ATTEMPT
    os=[Environment]::OSVersion.VersionString; image_os=$env:ImageOS; image_version=$env:ImageVersion
    runner_identity=$identity.Name; runner_session=[Diagnostics.Process]::GetCurrentProcess().SessionId
    machine_user=(Get-CimInstance Win32_ComputerSystem).UserName; require_no_login=[bool]$RequireNoLogin
    files=@()
}
$files = @($runner,$package,(Join-Path $build 'ui_session0_gl_app.dll'),(Join-Path $build 'ui_framework.dll'))
$files += @(Get-ChildItem -LiteralPath (Split-Path -Parent $provider) -Filter '*.dll' | ForEach-Object FullName)
foreach ($file in $files) {
    $hash = Get-FileHash -LiteralPath $file -Algorithm SHA256
    $manifest.files += [ordered]@{name=[IO.Path]::GetFileName($file); sha256=$hash.Hash.ToLowerInvariant(); bytes=(Get-Item -LiteralPath $file).Length}
}
$name = 'UiFrameworkSession0-' + [Guid]::NewGuid().ToString('N')
$manifest.service_name = $name
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $out 'manifest.json') -Encoding utf8
# The service is demand-start LocalSystem, with an exact GUID name owned by this
# invocation. No autostart, stored passwords, logoff, or existing service changes.
$binary = '"{0}" "{1}" "{2}" "{3}" --service "{4}"' -f $runner,$package,$provider,$out,$name
if ($RequireNoLogin) { $binary += ' --require-no-login' }
$created = $false
try {
    New-Service -Name $name -BinaryPathName $binary -StartupType Manual | Out-Null
    $created = $true
    Start-Service -Name $name
    $end = [DateTime]::UtcNow.AddSeconds(60)
    do {
        Start-Sleep -Milliseconds 200
        $service = Get-Service -Name $name
        $status = $service.Status
        $service.Dispose()
    } while ($status -ne 'Stopped' -and [DateTime]::UtcNow -lt $end)
    if ($status -ne 'Stopped') { throw 'Session0 validation exceeded 60 seconds.' }
    $resultFile = Join-Path $out 'result.txt'
    if (!(Test-Path -LiteralPath $resultFile)) { throw 'Service stopped without a completed result (crash/startup failure is not acceptance).' }
    Get-Content -LiteralPath (Join-Path $out 'run.log')
    Get-Content -LiteralPath (Join-Path $out 'errors.log')
    if ((Get-Content -LiteralPath $resultFile -Raw).Trim() -ne '0') { throw 'Session0 / no-login application validation failed; retain all artifacts.' }
} catch {
    $_ | Format-List * -Force | Out-File -LiteralPath (Join-Path $out 'service-error.log')
    if ($created) {
        Get-CimInstance Win32_Service -Filter "Name='$name'" | Select-Object Name,State,ExitCode,ProcessId,PathName |
            Format-List | Out-File -LiteralPath (Join-Path $out 'service-error.log') -Append
    }
    throw
} finally {
    if ($created) {
        $service = Get-Service -Name $name
        if ($service.Status -ne 'Stopped') {
            Stop-Service -Name $name
            $service.WaitForStatus('Stopped',[TimeSpan]::FromSeconds(10))
        }
        $service.Dispose()
        & sc.exe delete $name | Out-File -LiteralPath (Join-Path $out 'service-cleanup.log')
        if ($LASTEXITCODE) { throw 'Cannot remove the temporary validation service.' }
    }
}
