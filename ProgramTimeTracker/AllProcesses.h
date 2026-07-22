#pragma once
#include <process.h>
#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <chrono>
#include "Windows.h"
#include <stdexcept>
#include <ios>
#include <cstdio>
#include <psapi.h>
//#include <TlHelp32.h>
//#include <shellapi.h> // eventually for icon


using namespace std;


class AllProcesses
{
private:
	DWORD* currentProcessList = nullptr;
	unsigned int numProcesses = 0;
	


public:

	void getOpenedProcesses(); //gets all the processes that are running NOT SERVICES like for example the windows antimalware whatever

	


};

