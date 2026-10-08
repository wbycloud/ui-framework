$ErrorActionPreference='Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class RoundDependencies {
 [DllImport("kernel32.dll",CharSet=CharSet.Unicode)] public static extern IntPtr LoadLibrary(string path);
 [DllImport("kernel32.dll")] public static extern IntPtr GetProcAddress(IntPtr module,string name);
 [DllImport("kernel32.dll")] public static extern bool FreeLibrary(IntPtr module);
 [UnmanagedFunctionPointer(CallingConvention.StdCall,CharSet=CharSet.Unicode)] public delegate int BrowserVersion(string folder,out IntPtr version);
}
'@
$loaderPath=(Resolve-Path '.deps/Microsoft.Web.WebView2.1.0.4129.50/build/native/x64/WebView2Loader.dll').Path
$library=[RoundDependencies]::LoadLibrary($loaderPath)
if($library -eq [IntPtr]::Zero){throw 'Loader load failed'}
try {
 $method=[RoundDependencies]::GetProcAddress($library,'GetAvailableCoreWebView2BrowserVersionString')
 $query=[Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer($method,[RoundDependencies+BrowserVersion])
 $pointer=[IntPtr]::Zero;$result=$query.Invoke($null,[ref]$pointer)
 $version=[Runtime.InteropServices.Marshal]::PtrToStringUni($pointer)
 if($pointer -ne [IntPtr]::Zero){[Runtime.InteropServices.Marshal]::FreeCoTaskMem($pointer)}
} finally {[void][RoundDependencies]::FreeLibrary($library)}
$manifest=[ordered]@{framework_commit=(& git rev-parse HEAD);lexbor=(& git -C .deps/lexbor rev-parse HEAD);quickjs=(& git -C .deps/quickjs rev-parse HEAD);webview2_sdk='1.0.4129.50';runtime_query_hresult=$result;runtime_version=$version;loader=@{path=$loaderPath;sha256=(Get-FileHash -LiteralPath $loaderPath).Hash.ToLowerInvariant()};osmesa=@()}
foreach($name in @('osmesa.dll','libglapi.dll')){$manifest.osmesa+=@{file=$name;sha256=(Get-FileHash -LiteralPath (Join-Path '.deps/mesa-24.3.4/x64' $name)).Hash.ToLowerInvariant()}}
$manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath build/stability-state-20261008/dependencies-final.json -Encoding utf8
if($result -ne 0 -or !$version){throw 'Installed Runtime version not established'}
Write-Output "Actual Runtime $version; fixed dependencies hashed"
