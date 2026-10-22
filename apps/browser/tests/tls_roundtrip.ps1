param([Parameter(Mandatory=$true)][switch]$AllowTemporaryRoot,[Parameter(Mandatory=$true)][string]$Certificate,[Parameter(Mandatory=$true)][string]$Program,[Parameter(Mandatory=$true)][string]$Package,[Parameter(Mandatory=$true)][string]$Python,[Parameter(Mandatory=$true)][string]$ResultDir)
$ErrorActionPreference='Stop'
if (!$AllowTemporaryRoot) {throw 'Requires explicit current-run authorization'}
$certPath=(Resolve-Path -LiteralPath $Certificate).Path
$cert=[System.Security.Cryptography.X509Certificates.X509Certificate2]::new($certPath)
if($cert.Subject -ne 'CN=localhost' -or $cert.Issuer -ne $cert.Subject -or $cert.NotAfter -gt (Get-Date).AddDays(2)){throw 'Not the short-lived localhost fixture'}
$thumb=$cert.Thumbprint
& certutil.exe -user -store Root $thumb *> "$ResultDir/cert-before.txt"
if($LASTEXITCODE -eq 0){throw 'Refusing to alter an existing trust entry'}
$code=2;$remaining=-1
function Native-Cert([string]$Phase,[string[]]$Arguments) {
    $process=Start-Process certutil.exe -ArgumentList $Arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput "$ResultDir/certutil-$Phase.txt" -RedirectStandardError "$ResultDir/certutil-$Phase-error.txt"
    $helper=Start-Process $Python -ArgumentList @("`"$PSScriptRoot/confirm_test_certificate.py`"",[string]$process.Id,$thumb,$Phase) -WindowStyle Hidden -PassThru -RedirectStandardOutput "$ResultDir/cert-$Phase.txt" -RedirectStandardError "$ResultDir/cert-$Phase-error.txt"
    $process.WaitForExit();$helper.WaitForExit()
    return $process.ExitCode
}
try {
    $importCode=Native-Cert 'add' @('-user','-f','-addstore','Root',"`"$certPath`"")
    if($importCode){throw 'Fixture root import failed'}
    # Inspect in a fresh native process; do not rely on a previously opened
    # logical store or chain engine retaining its old cached collection.
    & certutil.exe -user -store Root $thumb *> "$ResultDir/tls-setup.txt"
    if($LASTEXITCODE){throw 'Fixture root absent after native import'}
    & $Program $Package --trusted-local *> "$ResultDir/tls-positive.txt";$code=$LASTEXITCODE
} finally {
    $removeCode=Native-Cert 'remove' @('-user','-f','-delstore','Root',$thumb)
    & certutil.exe -user -store Root $thumb *> "$ResultDir/cert-after.txt"
    $remaining=if($LASTEXITCODE -eq 0){1}else{0}
    [pscustomobject]@{sha1=$thumb;store='CurrentUser/Root';initial_count=0;remaining_count=$remaining;positive_exit_code=$code} | ConvertTo-Json | Set-Content "$ResultDir/tls-trust.json" -Encoding utf8
    if($remaining){throw 'Fixture root remains trusted'}
}
if($code){Get-Content "$ResultDir/tls-positive.txt" -Tail 3;exit $code}
& $Program $Package --tls-only *> "$ResultDir/tls-negative.txt";$negative=$LASTEXITCODE
Get-Content "$ResultDir/tls-trust.json"
Get-Content "$ResultDir/tls-negative.txt" -Tail 2
exit $negative
