#include "Process.h"
#include <fstream>

Process::Process(string fileName) { //filename will be the process

	ifstream file(fileName);


	if (file.is_open()) {
		string currentLine = "";
		while (getline(file, currentLine)) {
			if (currentLine == "- LOGS:") {

			}
		}

		file.close();
	}
	else { //file doesn't exist so no tracking data for it
		fileCreator(fileName);
	}

}


//better idea the logs save each day with its day data and that can be consulted later from the file itself
//so the file has each day saved,

//each get should probably calculate it but unsure still

uint64_t Process::getTotalTime() const {
	return totalTime;
}
uint64_t Process::getSessionTime() const {
	return sessionTime;
}
uint64_t Process::getTodayTime() const {
	return todayTime;
}
uint64_t Process::getWeeklyTime() const {
	return weeklyTime;
}
const uint64_t* Process::getMonthlyTimeArray() const {
	return monthlyTimeArray;
}
const uint64_t* Process::getYearlyTimeArray(int length) const {
	//return 
	uint64_t* res = new uint64_t[lengthYearlyArray];


	return res;
}

void Process::ResetTime() {
	//first delete time content from the fila


	//reset all time so that tracking now is done correctly
	totalTime = sessionTime = todayTime = weeklyTime = 0;
	
	for (int i = 0; i < lengthYearlyArray; i++) {
		delete yearlyArray[i];
	}
	delete yearlyArray;



	//probably better to just delete the entire file so that it's "restarted"
	
}


//format has to be like 
//-HEAD 
//content
//-HEAD
//content and so on

//will have logs as the starter
//and then each day it's logged in 
void fileCreator(string fileName){
	//string fileName = processName + "__" + to_string(PID) + ".pttl";
	ofstream file(fileName);


	file << "- PID:" << endl << PID << endl << "- PROGRAM NAME:" << endl << processName << endl << "- LOGS:" << endl;



	file.close();

}


void deleteFile(string fileName) {

}


int yearFrom2026() {

	time_t now = time(nullptr);
	tm* local = localtime(&now);//this transforms the time to the local structure

	int currYear = (*local).tm_year + 1900;

	if ((currYear - 2026) < 0) {
		throw out_of_range("Year can't be before 2026, Correct System Time");
		//has to be catched eventually and show an error box
	}

	return (currYear - 2026);
}

int currDay() {
	time_t now = time(nullptr);
	tm* local = localtime(&now);
	int day = (*local).tm_mday; //ret 1 to 31 no 0 start 

	return day;
}

int currMonth() {
	time_t now = time(nullptr);
	tm* local = localtime(&now);
	int month = (*local).tm_mon + 1; //so no 0 based 
	return month;
}

int currYear() {
	time_t now = time(nullptr);
	tm* local = localtime(&now);//this transforms the time to the local structure

	int currYear = (*local).tm_year + 1900;
	return currYear;
}
