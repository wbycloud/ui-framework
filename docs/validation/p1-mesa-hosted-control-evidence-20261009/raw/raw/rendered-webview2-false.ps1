./tools/test-windows-ci.ps1
if ('false' -eq 'true') {
  if (!$env:UI_P1_CANDIDATE_URL) { throw 'Candidate URL required for the authorized P1 run' }
  Add-Type -TypeDefinition 'using System.Runtime.InteropServices; public static class UiP1Desktop { [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i); [DllImport("user32.dll")] public static extern uint GetDpiForSystem(); }'
  $desktopBefore = @([UiP1Desktop]::GetSystemMetrics(0),[UiP1Desktop]::GetSystemMetrics(1))
  if ($env:GITHUB_ACTIONS -eq 'true') { Set-DisplayResolution -Width 1920 -Height 1080 -Force }
  @{before=$desktopBefore;after=@([UiP1Desktop]::GetSystemMetrics(0),[UiP1Desktop]::GetSystemMetrics(1));dpi=[UiP1Desktop]::GetDpiForSystem()} | ConvertTo-Json | Set-Content 'build/ci-evidence-webview2/lifecycle-desktop.json'
  $candidateRoot = 'build/ci-evidence-webview2/lifecycle-candidate'
  New-Item -ItemType Directory -Path $candidateRoot | Out-Null
  Invoke-WebRequest $env:UI_P1_CANDIDATE_URL -OutFile "$candidateRoot/candidate.zip"
  if ((Get-FileHash "$candidateRoot/candidate.zip").Hash.ToLowerInvariant() -ne 'afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae') { throw 'Candidate archive hash mismatch' }
  Expand-Archive -LiteralPath "$candidateRoot/candidate.zip" -DestinationPath "$candidateRoot/unpacked"
  Invoke-WebRequest 'https://github.com/pal1000/mesa-dist-win/releases/download/24.3.4/mesa3d-24.3.4-debug-info-msvc.7z' -OutFile "$candidateRoot/original-debug.7z"
  if ((Get-FileHash "$candidateRoot/original-debug.7z").Hash.ToLowerInvariant() -ne '60b2ccf9d81646aab53ce50bb1117c292e314049799b58f620d6838d134b48b1') { throw 'Original debug archive hash mismatch' }
  & 7z x "$candidateRoot/original-debug.7z" "-o$candidateRoot/original-symbols" 'x64/osmesa.pdb' -y | Out-File "$candidateRoot/original-symbols-extraction.log"
  if ($LASTEXITCODE) { throw 'Original symbol extraction failed' }
  if ((Get-FileHash "$candidateRoot/original-symbols/x64/osmesa.pdb").Hash.ToLowerInvariant() -ne '42659470e93b8305d514a956e830d1455318b326b2d2829e61426ca265967ee7') { throw 'Original PDB hash mismatch' }
  if ((Get-FileHash "$candidateRoot/unpacked/osmesa.pdb").Hash.ToLowerInvariant() -ne 'c5c15c5cc6673b795d21fa721d87a6abb382005417129685903937e744f73bbd') { throw 'Candidate PDB hash mismatch' }
  python tests/run_osmesa_lifecycle_control.py --build build/ci-webview2 --original .deps/mesa-24.3.4/x64/osmesa.dll --candidate "$candidateRoot/unpacked/osmesa.dll" --evidence build/ci-evidence-webview2/lifecycle-control
  if ($LASTEXITCODE) { throw 'Original/candidate diagnostic includes failures; inspect both retained results' }
} else {
  ./tools/windows-ci.ps1 -Configuration webview2 -Stage test
}
