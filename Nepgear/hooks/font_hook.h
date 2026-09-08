#pragma once
#include <windows.h>

void SetFontHookModule(HMODULE hModule);

namespace Hooks {
    void InstallPatchProfileHook();
    void InstallFontHook();
}

namespace FontHooks {
    void NotifyConfigChanged(LONG version);
    void PrepareForProcessExit();
}

namespace FontEngineBridge {
    bool IsRequired();
    void SetOriginalFileFunctions(
        void* createFileA, void* createFileW,
        void* getFileAttributesA, void* getFileAttributesW,
        void* getFileAttributesExA, void* getFileAttributesExW,
        void* findFirstFileA, void* findFirstFileW,
        void* findFirstFileExA, void* findFirstFileExW);

    bool TryCreateFileA(LPCSTR fileName, DWORD desiredAccess, DWORD shareMode,
        LPSECURITY_ATTRIBUTES securityAttributes, DWORD creationDisposition,
        DWORD flagsAndAttributes, HANDLE templateFile, HANDLE* result);
    bool TryCreateFileW(LPCWSTR fileName, DWORD desiredAccess, DWORD shareMode,
        LPSECURITY_ATTRIBUTES securityAttributes, DWORD creationDisposition,
        DWORD flagsAndAttributes, HANDLE templateFile, HANDLE* result);
    bool TryGetFileAttributesA(LPCSTR fileName, DWORD* result);
    bool TryGetFileAttributesW(LPCWSTR fileName, DWORD* result);
    bool TryGetFileAttributesExA(LPCSTR fileName, GET_FILEEX_INFO_LEVELS infoLevel,
        LPVOID fileInformation, BOOL* result);
    bool TryGetFileAttributesExW(LPCWSTR fileName, GET_FILEEX_INFO_LEVELS infoLevel,
        LPVOID fileInformation, BOOL* result);
    bool TryFindFirstFileA(LPCSTR fileName, LPWIN32_FIND_DATAA findData, HANDLE* result);
    bool TryFindFirstFileW(LPCWSTR fileName, LPWIN32_FIND_DATAW findData, HANDLE* result);
    bool TryFindFirstFileFallbackA(LPCSTR fileName, LPWIN32_FIND_DATAA findData, HANDLE* result);
    bool TryFindFirstFileFallbackW(LPCWSTR fileName, LPWIN32_FIND_DATAW findData, HANDLE* result);
    bool TryFindFirstFileExA(LPCSTR fileName, FINDEX_INFO_LEVELS infoLevel,
        LPVOID findData, FINDEX_SEARCH_OPS searchOp, LPVOID searchFilter,
        DWORD additionalFlags, HANDLE* result);
    bool TryFindFirstFileExW(LPCWSTR fileName, FINDEX_INFO_LEVELS infoLevel,
        LPVOID findData, FINDEX_SEARCH_OPS searchOp, LPVOID searchFilter,
        DWORD additionalFlags, HANDLE* result);
    bool TryFindFirstFileExFallbackA(LPCSTR fileName, FINDEX_INFO_LEVELS infoLevel,
        LPVOID findData, FINDEX_SEARCH_OPS searchOp, HANDLE* result);
    bool TryFindFirstFileExFallbackW(LPCWSTR fileName, FINDEX_INFO_LEVELS infoLevel,
        LPVOID findData, FINDEX_SEARCH_OPS searchOp, HANDLE* result);
}
