@echo off
cd /D "%~dp0\.."
cmake -B build\windows-x32 -A Win32
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build build\windows-x32 --config Debug --parallel
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
