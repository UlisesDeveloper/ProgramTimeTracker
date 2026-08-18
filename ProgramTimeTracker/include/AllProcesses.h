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
#include "globals.h"
#include "auxiliaryMainFunctions.h"
//#include <TlHelp32.h>
//#include <shellapi.h> // eventually for icon


using namespace std;

struct pidList;

class AllProcesses
{
private:
	DWORD numProcesses = 0;
	Process* currentProcessList = nullptr;
	Process systemAndMisc = Process("logs/systemAndMisc.pttl", 0, "systemAndMisc");



	//could be interesting that there is a process var here that has currentWin
	//Which is the cursor one and in 5mins inactivity none for the clock but im
	//not sure this is the best option and allwindows should be obtaining it 





public:

	void getOpenedProcesses(); //gets all the processes that are running NOT SERVICES like for example the windows antimalware whatever

	Process getFocusedProcess() const; //by focused the one in focus with mouse/keyboard control

	//make somethink like time since focusedProcess had an actual press of something
	//it should return like seconds if it's 5mins then stop, if it has the mode video activated then it will return 0
	int getIdleSecondsFocusedProcess(bool videoModeEnabled, int secsBeforeVideoTimeOut = 0) const;
	//the 5 minutes thing whenever i check should be a global variable that the user can change to define what is afk

	void addProcess(Process& a);  

	void removeProcessWPID(DWORD PID);



	bool isWindowFullScreen(HWND& main) const;
	


	void addTimeActiveProcess(int toAdd);
	void addTimeBackgroundProcesses(int toAdd, bool timeOut);


	void saveTime();
	void resetDayTime();
	void addTimeToSystemProcess(int a);

	void resetTime(); //The files deletes all their data 


	void getPathNameCurrentProcesses(string*& a, int& size) const; //automatically resizes the array to what is needed

	bool getProcessFromPath(const string& path, const Process*& res) const;

};



void DeletePID(int pos, size_t &total, DWORD* array); //doesn't resize capacity
void getAllPID(DWORD& cap, DWORD*& array, DWORD& numPID); //returns numProcesses
void getActiveWindows(pidList& a);
void getMetadataForPids(pidList& pidLs, AllProcesses& allP);
bool isWindowMultimediaTitle(HWND main);
DWORD getPidFromHWND(HWND main);
bool isWindowUsingAudio(HWND main);





