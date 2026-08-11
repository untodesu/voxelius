@echo off
cd /D "%~dp0\.."
call scripts\build-windows-x64-release.bat
if %errorlevel% neq 0 exit /b %errorlevel%
cpack --preset windows-x64-release-nsis
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
