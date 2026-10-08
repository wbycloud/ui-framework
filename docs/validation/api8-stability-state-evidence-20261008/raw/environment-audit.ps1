$ErrorActionPreference='Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class RoundEnvironment {
 [DllImport("advapi32.dll",EntryPoint="OpenSCManagerW",SetLastError=true)] private static extern IntPtr OpenSCManager(IntPtr machine,IntPtr database,uint access);
 [DllImport("advapi32.dll")] public static extern bool CloseServiceHandle(IntPtr handle);
 public static int ProbeServiceCreateAccess(){IntPtr h=OpenSCManager(IntPtr.Zero,IntPtr.Zero,2);if(h==IntPtr.Zero)return Marshal.GetLastWin32Error();CloseServiceHandle(h);return 0;}
 [DllImport("user32.dll")] public static extern int GetSystemMetrics(int metric);
 [DllImport("user32.dll")] public static extern uint GetDpiForSystem();
 [DllImport("dwmapi.dll")] public static extern int DwmIsCompositionEnabled(out bool enabled);
}
'@
$errorCode=[RoundEnvironment]::ProbeServiceCreateAccess()
$access=if($errorCode -eq 0){[IntPtr]1}else{[IntPtr]::Zero}
$composed=$false;$dwmResult=[RoundEnvironment]::DwmIsCompositionEnabled([ref]$composed)
$original='D:/应用软件框架/klayoutC'
$status=@(& git --no-optional-locks -C $original status --porcelain)
$head=& git --no-optional-locks -C $original rev-parse HEAD
$diff=@(& git --no-optional-locks -C $original diff --binary)
$roundPath=(Resolve-Path 'build/stability-state-20261008').Path
$diff | Set-Content -LiteralPath "$roundPath/business-user-changes-final.patch" -Encoding utf8
$prior=Get-Content -LiteralPath "$roundPath/business-user-changes.patch" -Raw
$current=Get-Content -LiteralPath "$roundPath/business-user-changes-final.patch" -Raw
$same=$prior.Replace("`r`n","`n") -ceq $current.Replace("`r`n","`n")
$status | Set-Content -LiteralPath "$roundPath/business-original-final-status.txt" -Encoding utf8
try{$os=Get-CimInstance Win32_OperatingSystem}catch{$version=Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion';$os=[pscustomobject]@{Caption=$version.ProductName;Version=[Environment]::OSVersion.Version.ToString();BuildNumber=$version.CurrentBuildNumber};Write-Output 'CIM read denied; OS identity uses read-only registry and process version'}
$manifest=[ordered]@{observedUtc=[DateTime]::UtcNow.ToString('o');frameworkHead=(& git rev-parse HEAD);branch=(& git branch --show-current);os=$os.Caption;osVersion=$os.Version;build=$os.BuildNumber;processSession=(Get-Process -Id $PID).SessionId;isAdministrator=([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator);scManagerCreateAccess=$access -ne [IntPtr]::Zero;scManagerError=$errorCode;serviceCreatedOrChanged=$false;currentSourceSession0='not_run';strictNoLoginRunner='unavailable_user_confirmed';hostedCiAuthorization=$false;hostedCiRun=$false;physicalMonitorAcceptance=$false;realChineseImeAcceptance=$false;longManualAcceptance=$false;formalBusinessModificationAuthorization=$false;desktop=@{width=[RoundEnvironment]::GetSystemMetrics(0);height=[RoundEnvironment]::GetSystemMetrics(1);detectedMonitors=[RoundEnvironment]::GetSystemMetrics(80);systemDpi=[RoundEnvironment]::GetDpiForSystem();dwmHresult=$dwmResult;dwmEnabled=$composed;physicalMixedDpiAndEdgeSnapAcceptance=$false};originalBusiness=@{head=$head;trackedChanges=@($status|Where-Object {$_ -notmatch '^\?\?'}).Count;trackedUserDiffUnchanged=$same;access='read_only';newCopy='abc8d870efc6401dc4fa55e50170a5449c11e36b';modifiedCopyFiles=@('app/manifest.ini','framework-sdk.lock.json')};hardwareWindowlessGlExpanded=$false}
$manifest | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath "$roundPath/environment-final.json" -Encoding utf8
if(!$same){throw 'Original tracked user diff changed; preserve and report before delivery'}
Write-Output "Current service create-access error $errorCode; original business tracked user diff unchanged"
