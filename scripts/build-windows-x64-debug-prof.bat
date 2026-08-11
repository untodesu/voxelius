@echo off
cd /D "%~dp0\.."
cmake --preset windows-x64-profiling
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build --preset windows-x64-profiling-debug
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
