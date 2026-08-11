@echo off
cd /D "%~dp0\.."
cmake --preset windows-x32
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build --preset windows-x32-release
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
