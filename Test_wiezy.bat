@echo off
cd /d "%~dp0"
start "Emberfall - DEV" "dist\Emberfall.exe" --start tower-1 %*
