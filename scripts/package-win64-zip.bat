@echo off
cd /D "%~dp0\.."
call scripts\build-win64-release.bat
if %errorlevel% neq 0 exit /b %errorlevel%
cpack -G ZIP --config build\win64\CPackConfig.cmake
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
