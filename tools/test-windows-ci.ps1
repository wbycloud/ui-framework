param([string]$ChecksPath = "$PSScriptRoot/windows-ci-checks.ps1")
$ErrorActionPreference='Stop'
. $ChecksPath
function Assert($condition,$message){if(!$condition){throw $message}}
function MustReject($action,$message){$rejected=$false;try{& $action}catch{$rejected=$true};Assert $rejected $message}
Assert-UiCiCTestVersion 'ctest version 3.26.0'
Assert-UiCiCTestVersion 'ctest version 4.1.2'
MustReject {Assert-UiCiCTestVersion 'ctest version 3.25.0'} 'Old CTest accepted'
MustReject {Assert-UiCiCTestVersion 'unknown version'} 'Unknown CTest accepted'
foreach($name in @('ui_component_layout_webview2','ui_component_widths_webview2','ui_webview2_render','ui_component_scroll_webview2','ui_component_experience_webview2','ui_api7_integration_webview2','ui_visual_ui_webview2','ui_stateful_components_webview2','ui_instance_language_webview2')){Assert ($name -match $UiCiRuntimeTests) "Runtime test misclassified: $name"}
foreach($name in @('ui_windowless_gl','ui_session0_interactive_control','ui_api7_integration','ui_visual_ui_light','ui_host_repaint','ui_stateful_components_light','ui_instance_language_light')){Assert ($name -match $UiCiProviderTests -and $name -notmatch $UiCiRuntimeTests) "Provider test misclassified: $name"}
Assert ('ui_api7_integration_webview2' -notmatch $UiCiProviderTests) 'Runtime in provider phase'
Assert ('ui_offscreen_msaa' -notmatch $UiCiProviderTests) 'WGL in provider phase'
foreach($name in @('ui_api5_integration','ui_api5_native_host','ui_api6_integration','ui_api6_integration_webview2','ui_original_api5_package','ui_legacy_api6_light','ui_legacy_api6_webview2')){Assert ($name -notmatch $UiCiRuntimeTests -and $name -notmatch $UiCiProviderTests) "Historical test still selected: $name"}
MustReject {Assert-UiCiResults ([xml]'<testsuite/>') @() native} 'Empty tests accepted'
MustReject {Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="notrun"/></testsuite>') @('ui_workspace7') native} 'Unexpected skip accepted'
MustReject {Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="fail"><failure/></testcase></testsuite>') @('ui_workspace7') native} 'Failure accepted'
MustReject {Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="run"/></testsuite>') @('ui_workspace7','ui_public_headers_c') native} 'Missing test accepted'
MustReject {Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_workspace7" status="run"/><testcase name="ui_workspace7" status="run"/></testsuite>') @('ui_workspace7') native} 'Duplicate test accepted'
$valid=[xml]'<testsuite><testcase name="ui_workspace7" status="run"/><testcase name="ui_workspace_layout" status="run"/><testcase name="ui_public_headers_c" status="run"/><testcase name="ui_language_core" status="run"/><testcase name="ui_monitor_transition" status="notrun"/></testsuite>'
Assert-UiCiResults $valid @($valid.testsuite.testcase|ForEach-Object name) native
$valid.testsuite.testcase[0].AppendChild($valid.CreateElement('failure'))|Out-Null
MustReject {Assert-UiCiResults $valid @($valid.testsuite.testcase|ForEach-Object name) native} 'Failure in complete plan accepted'
foreach($name in @('ui_api4_compat','ui_api5_compat','ui_application_versions','ui_original_api2_package','ui_api3_original_package','ui_original_api4_package','ui_original_api5_package','ui_legacy_api6_light','ui_legacy_api6_webview2','ui_api5_integration','ui_api5_native_host','ui_api6_integration','ui_api6_integration_webview2')){
 $old=[xml]("<testsuite><testcase name='$name' status='run'/><testcase name='ui_workspace7' status='run'/><testcase name='ui_workspace_layout' status='run'/><testcase name='ui_public_headers_c' status='run'/></testsuite>")
 MustReject {Assert-UiCiResults $old @($old.testsuite.testcase|ForEach-Object name) native} "Historical acceptance gate remains: $name"
}
MustReject {Assert-UiCiResults ([xml]'<testsuite><testcase name="ui_webview2_render" status="run"/></testsuite>') @('ui_component_widths_webview2','ui_webview2_render') webview2} 'Absent mandatory current Runtime tests accepted'
$lightNames=@('ui_component_layout','ui_component_widths','ui_workspace7','ui_workspace_layout','ui_public_headers_c','ui_light_glyph_padding','ui_light_scroll','ui_component_scroll','ui_component_scroll_native','ui_menu_access','ui_application_contract','ui_language_core')
$light=[xml]('<testsuite>'+($lightNames|ForEach-Object {"<testcase name='$_' status='run'/>"})+'</testsuite>')
Assert-UiCiResults $light $lightNames light
$runtimeNames=$lightNames+@('ui_visual_ui_light','ui_windowless_gl','ui_session0_interactive_control','ui_api7_integration','ui_host_repaint','ui_stateful_components_light','ui_instance_language_light','ui_component_layout_webview2','ui_component_widths_webview2','ui_visual_ui_webview2','ui_component_scroll_webview2','ui_api7_integration_webview2','ui_component_experience_webview2','ui_webview2_render','ui_webview2_messages','ui_webview2_parity','ui_stateful_components_webview2','ui_instance_language_webview2')
$runtime=[xml]('<testsuite>'+($runtimeNames|ForEach-Object {"<testcase name='$_' status='run'/>"})+'</testsuite>')
Assert-UiCiResults $runtime $runtimeNames webview2
$runtime.testsuite.RemoveChild(($runtime.testsuite.testcase | Where-Object name -eq 'ui_instance_language_webview2'))|Out-Null
MustReject {Assert-UiCiResults $runtime @($runtime.testsuite.testcase|ForEach-Object name) webview2} 'Missing actual Runtime instance language test accepted'
$light.testsuite.RemoveChild(($light.testsuite.testcase | Where-Object name -eq 'ui_light_glyph_padding'))|Out-Null
MustReject {Assert-UiCiResults $light @($light.testsuite.testcase|ForEach-Object name) light} 'Missing explicit padding pixel regression accepted'
Write-Output 'CI checks PASS: current-only acceptance, WGL/provider/Runtime partition, exact inventory, failures and physical-only skip'
