@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
set "PATH=%CD%\llvm-install\bin;%CD%\tools\flexbison;%CD%\tools\venv\Scripts;C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
set "CMAKE_PREFIX_PATH=D:\ui-p1-mesa-lifecycle-20261009\llvm-install"
set "LLVM_CONFIG=D:\ui-p1-mesa-lifecycle-20261009\llvm-install\bin\llvm-config.exe"
for %%P in (B C) do (
  meson setup --reconfigure --clearcache providers/%%P/build-ascii-llvm providers/%%P/source/mesa-24.3.4 --native-file=tools/meson-native.ini --cmake-prefix-path=D:/ui-p1-mesa-lifecycle-20261009/llvm-install --buildtype=release -Db_vscrt=mt -Db_ndebug=true -Dbuild-tests=false -Dosmesa=true -Dplatforms= -Dgallium-drivers=llvmpipe,softpipe -Dvulkan-drivers= -Dllvm=enabled -Dshared-llvm=disabled -Dshared-glapi=disabled -Dgles1=disabled -Dgles2=disabled -Dglx=disabled -Degl=disabled -Dgbm=disabled -Dgallium-d3d10umd=false -Dgallium-d3d12-video=disabled -Dzlib=disabled -Dzstd=disabled -Dc_args=/Zi -Dcpp_args=/Zi -Dc_link_args=/DEBUG -Dcpp_link_args=/DEBUG --wrap-mode=nofallback > recipe/provider-%%P-configure.log 2>&1
  if errorlevel 1 exit /b 1
  ninja -C providers/%%P/build-ascii-llvm -j 1 src/compiler/glsl/glcpp/glcpp-lex.c src/compiler/glsl/glsl_lexer.cpp > recipe/provider-%%P-lexers.log 2>&1
  if errorlevel 1 exit /b 1
  meson compile -C providers/%%P/build-ascii-llvm -j 8 > recipe/provider-%%P-build.log 2>&1
  if errorlevel 1 exit /b 1
)
