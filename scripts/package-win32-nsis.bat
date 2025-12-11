@echo off
cd /D "%~dp0\.."
call scripts\build-win32-release.bat
if %errorlevel% neq 0 exit /b %errorlevel%
cpack -G NSIS --config build\win32\CPackConfig.cmake
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
