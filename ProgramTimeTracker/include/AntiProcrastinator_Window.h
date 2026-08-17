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




void AntiProcrastinationWindow(bool& show_antiprocrastination_window, bool& show_add_entry_procrastination_window,  bool& show_modify_entry_procrastination_window, AllProcesses& tracker, ID3D11Device* d3dDevice, HANDLE& hMutex, bool& done);
bool loadAntiProcrastinationDataFromDir(const string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle);
bool saveAntiProcrastinationDataFromDir(const string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle);
int posProcrastinationDataAndRamWrite(const string& path, string*& ramSave, int& sizeArraySave); //Returns pos -1 if didn't find anything, -2 if empty, -3 if couldn't open the file
bool entryExists(const string& path);
bool deleteEntry(const string& path, string*& ramSave, int& sizeRamSave);
void allEntryPathsProcrastinatorFile(string*& entries, int& sizeEntriesArray);

//getter from string array to avoid io penalty
bool entryExistsArray(const string* pathArray, const int& sizeEntriesArray, const string& path);
bool pathArrayPathInfoGetter(const string* pathArray, const int& sizeEntriesArray, const string& path, int& currentActiveProcessGlobalLimit, int& currentActiveProcessWeekEndLimit, bool& currentActiveProcessWeekEndLimitStatus);

//getter for a path seconds, weekend mode and weekend seconds
//so that the program loop checks first entryexist for the active process
//Then checks that the (weekend mode is enabled and if it's a weekend) if it is then check if the weekend time has been excedeed FOR TODAY
//if it's not a weekend or weekend mode isn't enable resort to normal limit, as always check the day hasn't changed cause then the timer for it resets


