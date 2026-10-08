@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
dumpbin /exports build/ui-repair-20261009/baseline/ui_framework.dll >build/ui-repair-20261009/exports.log 2>&1
dumpbin /imports build/ui-repair-20261009/baseline/dialog_layout_example.exe >build/ui-repair-20261009/example-imports.log 2>&1
