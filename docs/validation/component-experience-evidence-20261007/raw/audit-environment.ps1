Add-Type -TypeDefinition @"
using System;using System.Runtime.InteropServices;
public static class ServiceAccessAudit { [DllImport("advapi32.dll",CharSet=CharSet.Unicode,SetLastError=true)] public static extern IntPtr OpenSCManager(string machine,string database,uint access); [DllImport("advapi32.dll")]public static extern bool CloseServiceHandle(IntPtr h); }
"@
$taskSvc=[ServiceAccessAudit]::OpenSCManager($null,$null,2);$taskServiceError=[Runtime.InteropServices.Marshal]::GetLastWin32Error();if($taskSvc -ne [IntPtr]::Zero){[ServiceAccessAudit]::CloseServiceHandle($taskSvc)|Out-Null}
$taskIdentity=[Security.Principal.WindowsIdentity]::GetCurrent();$taskPrincipal=[Security.Principal.WindowsPrincipal]::new($taskIdentity)
[ordered]@{audit_time=(Get-Date -Format o);session_id=(Get-Process -Id $PID).SessionId;os=[Environment]::OSVersion.Version.ToString();elevated=$taskPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator);scm_create_service_access=($taskSvc -ne [IntPtr]::Zero);win32_error=$taskServiceError;strict_no_login_runner='User confirmed unavailable';remote_ci_authorization='None';physical_ime_mixed_dpi_long_manual='Unavailable to automated session';services_modified=$false;users_logged_off=$false} | ConvertTo-Json | Set-Content -LiteralPath build/experience-20261007/environment-final.json -Encoding utf8
Get-Content build/experience-20261007/environment-final.json
