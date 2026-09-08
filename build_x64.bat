@echo off
REM Thin wrapper: build only the 64-bit Release output.
call "%~dp0build.bat" x64 %*
exit /b %ERRORLEVEL%
