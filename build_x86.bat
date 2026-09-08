@echo off
REM Thin wrapper: build only the 32-bit Release output.
call "%~dp0build.bat" x86 %*
exit /b %ERRORLEVEL%
