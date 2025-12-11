@echo off
cd /D "%~dp0\.."
cmake -B build\win64 -A x64
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build build\win64 --config Release --parallel
if %errorlevel% neq 0 exit /b %errorlevel%
if /I %0 EQU "%~dpnx0" pause
