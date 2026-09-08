#include "../pch.h"
#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
#include "file_hook.h"
#include "config.h"
#include "utils.h"
#include "../detours.h"
#include "vfs.h"
#include "rioshiina_hook.h"
#include "font_hook.h"

#ifdef _WIN64
#pragma comment(lib, "detours_x64.lib")
#else
#pragma comment(lib, "detours.lib")
#endif
#pragma comment(lib, "Shlwapi.lib")
#include "internal/file/file_hook_state_paths.cppinc"
#include "internal/file/file_hook_io.cppinc"
#include "internal/file/file_hook_attributes.cppinc"
#include "internal/file/file_hook_directory.cppinc"
#include "internal/file/file_hook_install.cppinc"
