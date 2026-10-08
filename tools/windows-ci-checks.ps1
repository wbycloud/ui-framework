# Current SDK acceptance. Historical compatibility fixtures remain archived, unregistered.
$UiCiRuntimeTests = 'ui_webview2_|ui_component_layout_webview2|ui_component_widths_webview2|ui_component_experience_webview2|ui_component_scroll_webview2|ui_api7_integration_webview2|ui_visual_ui_webview2|ui_stateful_components_webview2|ui_instance_language_webview2'
$UiCiProviderTests = '^(ui_windowless_gl|ui_session0_interactive_control|ui_api7_integration|ui_visual_ui_light|ui_host_repaint|ui_stateful_components_light|ui_instance_language_light)$'
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
        if ($test.name -match '^ui_(api[1-6]_(compat|integration|integration_webview2|native_host|original_package)|original_api[1-6]_package|legacy_api[1-6]_(light|webview2)|application_versions)$') {
            throw "Historical compatibility test in current acceptance: $($test.name)"
        }
        if ($test.SelectSingleNode('failure')) { throw "Failed test: $($test.name)" }
        if ($test.status -ne 'run' -and !($test.status -eq 'notrun' -and $test.name -eq 'ui_monitor_transition')) {
            throw "Unexpected test status: $($test.name)=$($test.status)"
        }
    }
    $required = @('ui_workspace7','ui_workspace_layout','ui_public_headers_c','ui_language_core')
    if ($Configuration -ne 'native') { $required += @('ui_component_layout','ui_component_widths','ui_light_glyph_padding','ui_light_scroll','ui_component_scroll','ui_component_scroll_native','ui_menu_access','ui_application_contract') }
    if ($Configuration -in @('osmesa','webview2')) { $required += @('ui_visual_ui_light','ui_windowless_gl','ui_session0_interactive_control','ui_api7_integration','ui_host_repaint','ui_stateful_components_light','ui_instance_language_light') }
    if ($Configuration -eq 'webview2') {
        $required += @('ui_component_layout_webview2','ui_component_widths_webview2','ui_visual_ui_webview2','ui_component_scroll_webview2','ui_api7_integration_webview2','ui_component_experience_webview2','ui_webview2_render','ui_webview2_messages','ui_webview2_parity','ui_stateful_components_webview2','ui_instance_language_webview2')
    }
    foreach ($name in $required) { if ($name -notin $names) { throw "Required $Configuration test absent: $name" } }
}
