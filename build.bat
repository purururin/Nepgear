@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul 2>nul
cd /d "%~dp0"

REM Usage: build.bat [x86|x64|all] [rebuild] [debug]
REM   Release builds emit an already UPX-compressed winmm.dll; no pack step.
REM   Set NEPGEAR_SKIP_PACK=1 to keep the DLL uncompressed for PDB debugging.

set "ARCH=all"
set "BUILD_TARGET=Build"
set "CONFIG=Release"

:parse_args
if "%~1"=="" goto :args_done
if /I "%~1"=="x86"     ( set "ARCH=x86"          & shift & goto :parse_args )
if /I "%~1"=="Win32"   ( set "ARCH=x86"          & shift & goto :parse_args )
if /I "%~1"=="x64"     ( set "ARCH=x64"          & shift & goto :parse_args )
if /I "%~1"=="all"     ( set "ARCH=all"          & shift & goto :parse_args )
if /I "%~1"=="rebuild" ( set "BUILD_TARGET=Rebuild" & shift & goto :parse_args )
if /I "%~1"=="debug"   ( set "CONFIG=Debug"      & shift & goto :parse_args )
if /I "%~1"=="release" ( set "CONFIG=Release"    & shift & goto :parse_args )
echo ERROR: Unknown argument "%~1".
echo Usage: %~nx0 [x86^|x64^|all] [rebuild] [debug]
exit /b 2

:args_done

call :find_msbuild || exit /b 1

set "BUILD_RESULT=0"
if /I "!ARCH!"=="all" (
    call :build_one x86
    if errorlevel 1 set "BUILD_RESULT=1"
    if "!BUILD_RESULT!"=="0" call :build_one x64
    if errorlevel 1 set "BUILD_RESULT=1"
) else (
    call :build_one !ARCH!
    if errorlevel 1 set "BUILD_RESULT=1"
)

echo.
echo ============================================
if "!BUILD_RESULT!"=="0" (
    echo !CONFIG! build succeeded ^(!BUILD_TARGET!^).
    call :report x86 "Release"
    call :report x64 "x64\Release"
) else (
    echo !CONFIG! build FAILED.
)
echo ============================================
if not defined NEPGEAR_NO_PAUSE pause
exit /b !BUILD_RESULT!

REM ---------------------------------------------------------------- helpers

:find_msbuild
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo ERROR: vswhere.exe not found. Install Visual Studio with the C++ workload.
    exit /b 1
)

set "VS_PATH="
for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_PATH=%%i"
if not defined VS_PATH (
    echo ERROR: No Visual Studio installation with the C++ toolset was found.
    exit /b 1
)

set "MSBUILD=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"
if not exist "!MSBUILD!" (
    echo ERROR: MSBuild.exe not found under !VS_PATH!.
    exit /b 1
)
echo Visual Studio: !VS_PATH!
exit /b 0

:build_one
echo.
echo [INFO] Building Nepgear + Packer + Unpacker ^(!CONFIG!^|%~1^)...
REM Suppress LNK4099 (detours.lib ships without a PDB).
set "LINK=/IGNORE:4099"
"!MSBUILD!" "Nepgear.sln" -p:Configuration=!CONFIG! -p:Platform=%~1 -t:!BUILD_TARGET! -v:minimal -m -nologo
exit /b !ERRORLEVEL!

:report
if exist "%~2\winmm.dll" (
    for %%F in ("%~2\winmm.dll") do echo   %~1: %~2\winmm.dll ^(%%~zF bytes^)
)
exit /b 0
