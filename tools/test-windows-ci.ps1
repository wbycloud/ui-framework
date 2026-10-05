param([string]$ChecksPath = "$PSScriptRoot/windows-ci-checks.ps1")
$ErrorActionPreference = 'Stop'
. $ChecksPath
function Assert($condition, $message) { if (!$condition) { throw $message } }
function MustReject($action, $message) {
    $rejected = $false
    try { & $action } catch { $rejected = $true }
    Assert $rejected $message
}
foreach ($name in @('ui_api5_integration','ui_api5_native_host','ui_original_api5_package','ui_legacy_api6_webview2','ui_component_scroll_webview2','ui_api7_integration_webview2')) {
    Assert ($name -match $UiCiRuntimeTests) "Runtime test misclassified: $name"
}
Assert ('ui_legacy_api6_light' -notmatch $UiCiRuntimeTests) 'Light test misclassified'
MustReject { Assert-UiCiResults ([xml]'<testsuite/>') @() native } 'Empty tests accepted'
MustReject { Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="notrun"/></testsuite>') @('ui_workspace7') native } 'Unexpected skip accepted'
MustReject { Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="fail"><failure/></testcase></testsuite>') @('ui_workspace7') native } 'Failure accepted'
MustReject { Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="run"/></testsuite>') @('ui_workspace7','ui_api5_compat') native } 'Missing test accepted'
MustReject { Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="run"/><testcase name="ui_workspace7" status="run"/></testsuite>') @('ui_workspace7') native } 'Duplicate test accepted'
$valid = [xml]'<testsuite><testcase name="ui_workspace7" status="run"/><testcase name="ui_workspace_layout" status="run"/><testcase name="ui_api5_compat" status="run"/><testcase name="ui_public_headers_c" status="run"/><testcase name="ui_monitor_transition" status="notrun"/></testsuite>'
Assert-UiCiResults $valid @($valid.testsuite.testcase | ForEach-Object name) native
$valid.testsuite.testcase[0].AppendChild($valid.CreateElement('failure')) | Out-Null
MustReject { Assert-UiCiResults $valid @($valid.testsuite.testcase | ForEach-Object name) native } 'Failure in a complete plan accepted'
Assert ($UiCiRuntimeTests -notmatch 'ui_legacy_api6_light') 'Invalid Runtime filter'
MustReject { Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_webview2_render" status="run"/></testsuite>') @('ui_webview2_render') webview2 } 'Absent mandatory Runtime tests accepted'
$directory = Join-Path ([IO.Path]::GetTempPath()) ('ui-ci-check-' + [Guid]::NewGuid().ToString('N'))
[void][IO.Directory]::CreateDirectory($directory)
try {
    'rebuilt' | Set-Content "$directory/minimal_eda.uapp"
    "UI_LEGACY_EDA_PACKAGE:FILEPATH=$directory/minimal_eda.uapp" | Set-Content "$directory/CMakeCache.txt"
    $rows = @(Get-UiCiOriginalPackages $directory ([xml]'<testsuite><testcase name="ui_application_versions" status="run"/></testsuite>'))
    Assert ($rows.Count -eq 6) 'Original API1-6 entries missing'
    Assert ($rows[0].status -eq 'rebuilt_fixture') 'Rebuilt API1 fixture claimed as original'
    Assert ($rows[1].status -eq 'not_provided') 'Missing API2 original not reported'
    Assert ($rows[0].sha256.Length -eq 64) 'Provided package not hashed'
    $outside = Join-Path $directory 'original-api6.uapp'
    'different' | Set-Content $outside
    "UI_LEGACY_API6_PACKAGE:FILEPATH=$outside" | Add-Content "$directory/CMakeCache.txt"
    $rows = @(Get-UiCiOriginalPackages $directory ([xml]'<testsuite><testcase name="ui_legacy_api6_webview2" status="run"/></testsuite>'))
    Assert ($rows[5].status -eq 'unverified_identity') 'Unknown original hash claimed as accepted'
    'UI_LEGACY_API5_PACKAGE:FILEPATH=missing-original.uapp' | Add-Content "$directory/CMakeCache.txt"
    $rows = @(Get-UiCiOriginalPackages $directory ([xml]'<testsuite/>'))
    Assert ($rows[4].status -eq 'missing_file') 'Missing configured original not reported'
} finally {
    # Only this invocation's fresh temporary directory is removed.
    Remove-Item -LiteralPath $directory -Recurse -Force
}
Write-Output 'CI checks PASS: partition, empty/missing/duplicate/failure/skip, Runtime requirements, original provenance'
