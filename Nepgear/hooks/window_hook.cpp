#include "../pch.h"
#include <commctrl.h>
#include "window_hook.h"
#include "config.h"
#include "utils.h"
#include "../detours.h"

#ifdef _WIN64
#pragma comment(lib, "detours_x64.lib")
#else
#pragma comment(lib, "detours.lib")
#endif
#pragma comment(lib, "comctl32.lib")

#include "internal/window/window_hook_state.cppinc"
#include "internal/window/window_hook_title.cppinc"
#include "internal/window/window_hook_message.cppinc"
#include "internal/window/window_hook_api.cppinc"
#include "internal/window/window_hook_event.cppinc"
#include "internal/window/window_hook_install.cppinc"
