#pragma once

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
#include "Process.h"
//#include <TlHelp32.h>
//#include <shellapi.h> // eventually for icon


using namespace std;


class AllProcesses
{
private:
	DWORD numProcesses = 0;
	Process* currentProcessList = nullptr;

	//could be interesting that there is a process var here that has currentWin
	//Which is the cursor one and in 5mins inactivity none for the clock but im
	//not sure this is the best option and allwindows should be obtaining it 

	
	


public:

	void getOpenedProcesses(); //gets all the processes that are running NOT SERVICES like for example the windows antimalware whatever

	


};



void DeletePID(int pos, int total, DWORD* array);
void getAllPID(DWORD& cap, DWORD*& array, DWORD& numPID); //returns numProcesses




