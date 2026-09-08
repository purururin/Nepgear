#include "../pch.h"
#include "utils.h"
#include "config.h"
#include "vfs.h"
#include "../font/font_patcher.h"
#include <Shlwapi.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <mutex>
#include <map>
#include <ShlObj.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")

#include <set>
#include "internal/utils/utils_state.cppinc"

namespace Utils {
#include "internal/utils/utils_font_loading.cppinc"
#include "internal/utils/utils_startup.cppinc"
#include "internal/utils/utils_logging.cppinc"
#include "internal/utils/utils_patch_files.cppinc"
#include "internal/utils/utils_pattern_scan.cppinc"
} // namespace Utils
