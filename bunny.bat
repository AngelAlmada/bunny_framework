@echo off
REM Bunny Framework command wrapper for Windows CMD
REM This wrapper allows executing: bunny flash, bunny flash clean, etc.

python "%~dp0bunny" %*
exit /b %ERRORLEVEL%
