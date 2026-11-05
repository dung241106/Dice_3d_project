@echo off
cd /d "%~dp0"
if not exist "build\Dice3D_Edu.exe" (
    call build_windows.bat
    if errorlevel 1 exit /b 1
)
"build\Dice3D_Edu.exe" %*
if errorlevel 1 pause
