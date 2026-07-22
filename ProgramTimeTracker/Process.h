#pragma once

#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <chrono>
#include "Windows.h"
#include "AllProcesses.h"
#include <stdexcept>
#include <ios>



using namespace std;



class Process{

private:
	uint64_t totalTime = 0;
	uint64_t sessionTime = 0; //by sessionTime i mean time the program has been on / pc has been on
	uint64_t todayTime = 0;
	uint64_t weeklyTime = 0;
	uint64_t* monthlyTimeArray = new uint64_t[12];
	uint64_t** yearlyArray = nullptr; // have it with each year from 2026 onwards 
	unsigned short lengthYearlyArray = 0;


	DWORD PID;
	string processName;

	//the process must not add time itself it must be done from allprocesses which knows if it's active

public:
	
	Process(string fileName); //loads file data


	//need to create the copy constructor
	Process(Process& a);

	~Process();

	uint64_t getTotalTime() const;
	uint64_t getSessionTime() const;
	uint64_t getTodayTime() const;
	uint64_t getWeeklyTime() const;
	const uint64_t* getMonthlyTimeArray() const; //12 always
	const uint64_t* getYearlyTimeArray( int length) const;
	//better to create a matrix for each month and do the sum to know the yearlyTimeArray so that getYearlyTimeArray returns an array with only each year's time

	void ResetTime(); //will reset allTimes, so need to delete everything from the files

	void fileCreator(string fileName) const;

	void deleteFile(string fileName) const;


	
	friend class AllProcesses;
	friend iostream saveToFile(iostream exit, const Process& a);

};


ostream& operator>>(iostream exit, const Process& a);
istream& operator<<(iostream entry, Process&a);


int yearFrom2026();

int currDay();

int currMonth();

int currYear();