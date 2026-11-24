@echo off
setlocal EnableExtensions DisableDelayedExpansion
for %%F in (framework_host.exe ui_framework.dll xuanshu_br2_meter.exe assistant-release.json vcruntime140.dll browser.uapp) do if not exist "%~dp0%%F" (
    echo A framework file is missing - %%F
    pause
    exit /b 1
)
start "" "%~dp0framework_host.exe"
exit /b %errorlevel%
