#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include "AllProcesses.h"
#include "globals.h"
#include "Process.h"
#include "auxiliaryMainFunctions.h"
#include <filesystem>
#include <shellapi.h>
#include <fstream>
#include <sstream>
#include <stdexcept>


void SettingsWindow(bool& show_settings_window, bool& show_alias_window , AllProcesses& tracker, HANDLE& hMutex, bool& done);