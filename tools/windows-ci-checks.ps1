# Shared by the existing matrix runner and its isolated contract tests.
$UiCiRuntimeTests = 'ui_webview2_|ui_api5_integration|ui_api5_native_host|ui_original_api5_package|ui_legacy_api6_webview2|ui_component_experience_webview2|ui_api6_integration_webview2|ui_component_scroll_webview2|ui_api7_integration_webview2'
$UiCiProviderTests = '^(ui_windowless_gl|ui_session0_interactive_control|ui_api6_integration|ui_api7_integration|ui_legacy_api6_light)$'
function Assert-UiCiCTestVersion([string]$VersionText) {
    if ($VersionText -notmatch '^ctest version ([0-9]+\.[0-9]+\.[0-9]+)' -or [version]$Matches[1] -lt [version]'3.26.0') {
        throw 'CI evidence requires CTest >=3.26 (JUnit and explicit empty-test errors)'
    }
}
function Assert-UiCiResults([xml]$Results, [string[]]$Expected, [string]$Configuration) {
    $tests = @($Results.testsuite.testcase)
    if (!$Expected.Count -or !$tests.Count) { throw 'Empty CTest plan/results are not acceptance' }
    $names = @($tests | ForEach-Object name)
    if ($names.Count -ne $Expected.Count -or @($names | Select-Object -Unique).Count -ne $names.Count -or
        @(Compare-Object $Expected $names).Count) { throw 'CTest results do not match the exact configured test plan' }
    foreach ($test in $tests) {
        if ($test.SelectSingleNode('failure')) { throw "Failed test: $($test.name)" }
        if ($test.status -ne 'run' -and !($test.status -eq 'notrun' -and $test.name -eq 'ui_monitor_transition')) {
            throw "Unexpected test status: $($test.name)=$($test.status)"
        }
    }
    $required = @('ui_workspace7','ui_workspace_layout','ui_api5_compat','ui_public_headers_c')
    if ($Configuration -ne 'native') { $required += @('ui_light_scroll','ui_component_scroll','ui_menu_access','ui_api4_compat') }
    if ($Configuration -in @('osmesa','webview2')) { $required += @('ui_windowless_gl','ui_session0_interactive_control','ui_api6_integration','ui_api7_integration') }
    if ($Configuration -eq 'webview2') {
        $required += @('ui_component_scroll_webview2','ui_api7_integration_webview2','ui_component_experience_webview2','ui_api6_integration_webview2','ui_api5_integration','ui_api5_native_host','ui_webview2_render','ui_webview2_messages','ui_webview2_parity')
    }
    foreach ($name in $required) { if ($name -notin $names) { throw "Required $Configuration test absent: $name" } }
}
function Get-UiCiOriginalPackages([string]$BuildDirectory, [xml]$Results) {
    # These are recorded original snapshots, never newly built substitutes.
    $specs = @(
        @('UI_LEGACY_EDA_PACKAGE','minimal_eda.uapp','86988546516e17b4783c5a9ed1583dfaa9e73ecdfcd1f5b8fad9a4910a5a3503',@('ui_application_versions')),
        @('UI_LEGACY_API2_PACKAGE','web_counter.uapp','2cf7a9619365184211cb636db6537c6119f03bc40d41d92edfbe6c235d3ccb2f',@('ui_original_api2_package')),
        @('UI_LEGACY_COMPONENT_PACKAGE','generic_components.uapp','d024c191b6fcae0da5eecc71b29c3429e27a74db9ec339a118a9577c691c0745',@('ui_api3_original_package')),
        @('UI_LEGACY_API4_PACKAGE','framework_features.uapp','ccc1ddd03068aaf8d68682ffe9a6d16afbb93939ded65c6925e5deee60ddd568',@('ui_original_api4_package')),
        @('UI_LEGACY_API5_PACKAGE','api5_fixture.uapp','285431bddc0d00bebed153c6441c00fc9be98793a79a50dd0a31c5b800645f19',@('ui_original_api5_package')),
        @('UI_LEGACY_API6_PACKAGE','api6_fixture.uapp','b493adda01f6dc5d8562a52e3648b2ce8b4e233a07d9c8def28646241cf084e4',@('ui_legacy_api6_light','ui_legacy_api6_webview2'))
    )
    $cache = if (Test-Path "$BuildDirectory/CMakeCache.txt") { Get-Content "$BuildDirectory/CMakeCache.txt" } else { @() }
    for ($i=0; $i -lt $specs.Count; ++$i) {
        $spec = $specs[$i]
        $line = @($cache | Where-Object { $_ -match "^$($spec[0]):FILEPATH=" } | Select-Object -Last 1)
        $path = if ($line.Count) { ($line[0] -split '=',2)[1] } else { '' }
        $status = 'not_provided'; $sha = $null
        $executed = @($Results.testsuite.testcase | Where-Object { $_.name -in $spec[3] } | ForEach-Object { [ordered]@{name=$_.name; status=$_.status; failed=[bool]$_.SelectSingleNode('failure')} })
        if ($path) {
            $status = 'missing_file'
            if (Test-Path -LiteralPath $path -PathType Leaf) {
                $sha = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
                $status = 'unverified_identity'
                if ([IO.Path]::GetFullPath($path) -eq [IO.Path]::GetFullPath("$BuildDirectory/$($spec[1])")) { $status = 'rebuilt_fixture' }
                elseif ($sha -eq $spec[2]) {
                    $status = 'not_covered'
                    if ($executed.Count -eq $spec[3].Count -and !@($executed | Where-Object { $_.status -ne 'run' -or $_.failed }).Count) { $status = 'passed' }
                    elseif (@($executed | Where-Object { $_.status -ne 'run' -or $_.failed }).Count) { $status = 'failed_or_skipped' }
                    elseif ($executed.Count) { $status = 'partial' }
                }
            }
        }
        [ordered]@{api=$i+1; variable=$spec[0]; path=$path; sha256=$sha; expectedSha256=$spec[2]; status=$status; tests=$executed}
    }
}
