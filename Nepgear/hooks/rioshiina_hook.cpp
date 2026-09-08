#include "../pch.h"
#include "rioshiina_hook.h"
#include "utils.h"
#include "config.h"
#include "../detours.h"
#include "vfs.h"

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <psapi.h>

#include "internal/engines/rioshiina/rioshiina_state.cppinc"
#include "internal/engines/rioshiina/rioshiina_unpack.cppinc"
#include "internal/engines/rioshiina/rioshiina_virtual_dvd.cppinc"
#include "internal/engines/rioshiina/rioshiina_redirect.cppinc"
#include "internal/engines/rioshiina/rioshiina_install.cppinc"
