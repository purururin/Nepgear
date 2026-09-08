#pragma once

namespace Hooks {
    void InstallFileHook();
    bool InstallFileHookForModule(HMODULE module);
    bool InstallKrkrPatchVfsHook(HMODULE module);
    FARPROC ResolveKrkrPatchVfsProcAddress(LPCSTR functionName);
}
