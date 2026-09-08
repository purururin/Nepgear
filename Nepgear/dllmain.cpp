#include "pch.h"
#include "Proxy.h"
#include "hooks/config.h"
#include "hooks/utils.h"
#include "hooks/font_hook.h"
#include "hooks/window_hook.h"
#include "hooks/file_hook.h"
#include "hooks/locale_emulator.h"
#include "hooks/vfs.h"
#include "hooks/crash_handler.h"
#include "hooks/codepage_hook.h"
#include "hooks/krkrz_hook.h"
#include "hooks/rioshiina_hook.h"

namespace {
bool g_helperProcess = false;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        wchar_t helperMarker[8] = {};
        if (GetEnvironmentVariableW(
                L"NEPGEAR_LE_HELPER", helperMarker, _countof(helperMarker)) > 0
            && wcscmp(helperMarker, L"1") == 0) {
            // rundll32 loads this DLL only to run the deferred LEP handoff.
            // Keep the proxy hooks out of that helper process, but load the
            // configuration so the same four architecture-specific DLLs are
            // used for the eventual HGB.exe launch.
            g_helperProcess = true;
            Config::LoadConfiguration(hModule);
            // The helper is a separate rundll32 process, so it has no VFS
            // state from the original game process. Initialize the archive
            // reader here so the architecture-specific LEP DLLs can be
            // staged even when the loose redirect folder is absent.
            VFS::Initialize(hModule);
            Utils::DeployPatchFiles(hModule);
            LocaleEmulator::getInstance().initialize();
            return TRUE;
        }

        InitHijack();
        Config::LoadConfiguration(hModule);

        if (!Config::IsSystemEnabled) {
            return TRUE;
        }

        Utils::InitConsole();
        CrashHandler::Install();
        VFS::Initialize(hModule);
        Utils::DeployPatchFiles(hModule);

        if (Config::EnableLE) {
            LocaleEmulator::getInstance().initialize();
            if (LocaleEmulator::getInstance().performLocaleEmulation()) {
                return TRUE;
            }
        }

        Utils::ShowStartupPopup();

        SetFontHookModule(hModule);
        Utils::Log("[Core] Loading packaged runtime modules...");
        Utils::LoadPackagedRuntimeModules();
        const bool packagedSBRio = Utils::IsPackagedRuntimeModuleLoaded(L"SBRioshiina.dll");
        Utils::Log("[Core] Installing packaged profile hook...");
        Hooks::InstallPatchProfileHook();
        if (packagedSBRio) {
            Utils::Log("[Core] Installing packaged SBRioshiina module VFS...");
            Hooks::InstallFileHookForModule(GetModuleHandleW(L"SBRioshiina.dll"));
        } else if (Config::EnableKrkrzHook) {
            Utils::Log("[Core] Installing selective KiriKiri XP3 VFS...");
            Hooks::InstallKrkrPatchVfsHook(GetModuleHandleW(nullptr));
            Hooks::InstallKrkrzHook();
        } else {
            Utils::Log("[Core] Installing file hook...");
            Hooks::InstallFileHook();
        }
        Utils::Log("[Core] Installing font hook...");
        Hooks::InstallFontHook();
        Utils::Log("[Core] Installing window hook...");
        Hooks::InstallWindowHook();
        Utils::Log("[Core] KiriKiri XP3 VFS initialization complete.");
        Utils::Log("[Core] Installing RioShiina hook...");
        Hooks::InstallRioShiinaHook();
        Utils::Log("[Core] All hooks installed.");
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        if (g_helperProcess) return TRUE;
        Utils::RequestShutdown();
        Free();
        Utils::CleanupPatchFiles();
        VFS::Shutdown();
    }
    return TRUE;
}
