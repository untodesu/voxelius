@echo off
cd /D "%~dp0"
set "PYTHON=python"
where py >nul 2>nul && set "PYTHON=py -3"
%PYTHON% scripts\setup_environment.py clangd %*
if /I %0 EQU "%~dpnx0" pause
exit /b %errorlevel%
