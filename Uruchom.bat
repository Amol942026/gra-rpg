@echo off
setlocal
cd /d "%~dp0"
set "game=%~dp0Emberfall.exe"
if not exist "%game%" set "game=%~dp0dist\Emberfall.exe"
start "Emberfall" "%game%" %*
