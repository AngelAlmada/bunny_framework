@echo off
REM ============================================================================
REM Bunny Framework - Setup Environment Variables for Windows CMD
REM ============================================================================
REM This script adds the Bunny Framework directory to the user's PATH
REM so you can use 'bunny flash' from any location
REM ============================================================================

setlocal enabledelayedexpansion

REM Get the script's directory (bunny_framework root)
set BUNNY_DIR=%~dp0

REM Remove trailing backslash
if "%BUNNY_DIR:~-1%"=="\" set BUNNY_DIR=%BUNNY_DIR:~0,-1%

echo.
echo ============================================================================
echo Bunny Framework - Environment Setup
echo ============================================================================
echo.
echo Bunny directory: %BUNNY_DIR%
echo.

REM Check if SETX is available
where setx >nul 2>&1
if errorlevel 1 (
    echo Error: SETX command not found. This script requires Windows Vista or later.
    echo.
    exit /b 1
)

REM Check if already in PATH
echo Checking if Bunny is already in PATH...
for /f "tokens=*" %%A in ('reg query "HKCU\Environment" /v Path 2^>nul ^| find /i "Path" ^| findstr /i /c:"%BUNNY_DIR%"') do (
    echo.
    echo ✓ Bunny is already in PATH!
    echo You can now use 'bunny flash' from any location.
    echo.
    exit /b 0
)

REM Add to PATH
echo.
echo Adding Bunny to PATH...
setx Path "!PATH!;%BUNNY_DIR%"

if errorlevel 1 (
    echo.
    echo ✗ Error: Failed to add Bunny to PATH
    echo Try running this script as Administrator
    echo.
    exit /b 1
)

echo.
echo ============================================================================
echo ✓ Success! Bunny has been added to PATH
echo ============================================================================
echo.
echo Note: You need to RESTART your Command Prompt for the changes to take effect.
echo.
echo After restarting, you can use:
echo   bunny flash
echo   bunny flash clean
echo   bunny --help
echo.
echo From any location on your computer.
echo.
pause
