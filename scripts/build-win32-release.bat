@echo off
cd /D "%~dp0\.."
cmake -B build\win32 -A Win32
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build build\win32 --config Release --parallel
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
