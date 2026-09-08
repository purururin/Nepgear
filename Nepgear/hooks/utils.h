#pragma once
#include <windows.h>
#include <stddef.h>

namespace Utils {
    enum LogLevel {
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR
    };

    BOOL LoadCustomFont(HMODULE hModule);
    void ShowStartupPopup();
    void Log(const char* format, ...);
    void Log(LogLevel level, const char* format, ...);
    void LogW(const wchar_t* format, ...);
    void LogW(LogLevel level, const wchar_t* format, ...);
    void Trace(const char* format, ...);
    void RequestShutdown();
    bool IsShuttingDown();
    void SaveConfig(HMODULE hModule);
    void InitConsole();
    BOOL DeployPatchFiles(HMODULE hModule);
    BOOL LoadPackagedRuntimeModules();
    bool IsPackagedRuntimeModuleLoaded(LPCWSTR moduleName);
    void PreservePatchFilesForRelaunch();
    void CleanupPatchFiles();
    bool ResolvePatchFilePathW(LPCWSTR requestedPath, LPWSTR resolvedPath, size_t resolvedPathCount);
    bool ResolvePatchFilePathA(LPCSTR requestedPath, LPSTR resolvedPath, size_t resolvedPathCount);
    bool ResolvePatchFilePathFromAnsi(LPCSTR requestedPath, LPWSTR resolvedPath, size_t resolvedPathCount);
    PVOID FindPattern(HMODULE hModule, const char* signature);
    PVOID FindPatternInBlock(void* startAddress, size_t size, const char* signature);
}
