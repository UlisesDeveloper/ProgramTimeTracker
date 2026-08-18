#pragma once
#pragma comment(lib, "d3d11.lib")

#include <string>
#include <iostream>
#include <fstream>
#include "globals.h"
#include "process.h"
#include "d3d11.h"
#include "AllProcesses.h"
#include "imgui.h"
#include <string>

using namespace std;

void LoadSettingsFile();
void SaveSettingsFile(string* multimProvWritten);
const ID3D11ShaderResourceView* showIconFromProcess(const Process& a, ID3D11Device* d3dDevice);
string getRealIconPath(const Process& a);
ID3D11ShaderResourceView* showIconFromPath(string targetPath, ID3D11Device* d3dDevice);


//uwp + file name shenanigans
string getStableTrackerID(const string& fullPath, const string& processName);

string sanitizePathForFileName(string pathOrId);


