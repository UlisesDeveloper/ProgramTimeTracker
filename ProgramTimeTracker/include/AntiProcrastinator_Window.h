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




void AntiProcrastinationWindow(bool& show_antiprocrastination_window, bool& show_add_entry_procrastination_window, AllProcesses& tracker, ID3D11Device* d3dDevice);
bool loadAntiProcrastinationDataFromDir(string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle);
bool saveAntiProcrastinationDataFromDir(string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle);
int posProcrastinationData(string& path); //Returns pos -1 if didn't find anything, -2 if empty