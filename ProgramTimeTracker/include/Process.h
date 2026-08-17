#pragma once

#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <chrono>
#include "Windows.h"
class AllProcesses;
#include <stdexcept>
#include <ios>
#include <cstdio>
#include <dwmapi.h>



using namespace std;



class Process{

private:
	//uint64_t totalTime = 0;
	uint64_t sessionTime = 0; //by sessionTime i mean time the program has been opened
	uint64_t todayTime = 0; //active time 
	uint64_t backgroundTodayTime = 0; //time where the mouse hasn't been centered on it, basically for multimonitor kinds of workloads
	
	/*uint64_t weeklyTime = 0;
	uint64_t* monthlyTimeArray = new uint64_t[12];
	uint64_t** yearlyArray = nullptr; // have it with each year from 2026 onwards 
	unsigned short lengthYearlyArray = 1;
	*/ 

	//probably should be calculated by consulting the file when needed

	DWORD PID;
	string processName;
	string logFileName;
	string pathName;
	//the process must not add time itself it must be done from allprocesses which knows if it's active

public:
	
	Process(); //default
	Process(string fileName, DWORD pidDef, string nameOfProcess = "", string pathNamed = ""); //loads file data


	//no need to create the copy constructor no dynamic memory


	//~Process();
	//no dynamic memory so far at least here

	uint64_t getTotalTime() const;
	uint64_t getSessionTime() const;
	uint64_t getTodayTime() const;
	uint64_t getBackgroundTodayTime() const;
	uint64_t getWeeklyTime() const;
	const uint64_t* getMonthlyTimeArray() const; //12 always
	const uint64_t* getYearlyTimeArray( int length) const;
	//better to create a matrix for each month and do the sum to know the yearlyTimeArray so that getYearlyTimeArray returns an array with only each year's time

	DWORD getPid() const;
	string getProcessName() const;
	string getLogFileName() const;
	string getPathName() const;



	void ResetTime(); //will reset allTimes, so need to delete everything from the files

	void fileCreator(string fileName, DWORD pidDef = -33, string nameOfProcess = "") const;

	void deleteFile(string fileName) ;

	void resetInitial();

	void saveTime() ; 

	void resetDayTime();


	bool doesProcessHaveSamePath(const string& path) const;

	
	
	
	friend class AllProcesses;

};



//don't know what use i could give them right now
ostream& operator>>(iostream exit, const Process& a);
istream& operator<<(iostream entry, Process&a);


int yearFrom2026();

int currDay();

int currMonth();

int currYear();

bool isWeekEnd();

void appendCurrentDateToFile(string fileName, const Process& a);


