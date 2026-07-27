#include "Process.h"
#include <fstream>

Process::Process() {

}



Process::Process(string fileName, DWORD pidDef, string nameOfProcess) { //horrid idea how does it get the pid has to have the pid name and whatever else

	ifstream file(fileName);
	logFileName = fileName;


	if (file.is_open()) {
		string currentLine = "";
		

		//this should be replaced to something that gets the last line and is saved in currentLine
		while (getline(file, currentLine)) {
		}




		size_t posFirstColon = currentLine.find(':');
		size_t posSecondColon = currentLine.find(':', posFirstColon + 1);
		string date = currentLine.substr(0, posFirstColon);
		//find gives first pos
		size_t posFirstSlash = date.find('/');
		size_t posSecondSlash = date.find('/', posFirstSlash + 1);

		if (posFirstColon == string::npos || posSecondColon == string::npos ||
			posFirstSlash == string::npos || posSecondSlash == string::npos) {
			throw invalid_argument("wrong format in process file");
		}


		if ((stoi(date.substr(0, posFirstSlash)) != currDay())
			|| (stoi(date.substr(posFirstSlash + 1, posSecondSlash - posFirstSlash - 1)) != currMonth()) 
			|| (stoi(date.substr(posSecondSlash + 1)) != currYear())
		) {
			appendCurrentDateToFile(fileName, *this);
		} else { //current day is in file
				
			todayTime = stoi(string(currentLine.substr(posFirstColon + 1, posSecondColon - posFirstColon - 1)));
			backgroundTodayTime = stoi(string(currentLine.substr(posSecondColon + 1)));
		}
		
		





		file.close();
	}
	else { //file doesn't exist so no tracking data for it
		fileCreator(fileName, pidDef, nameOfProcess);
	}

}




//better idea the logs save each day with its day data and that can be consulted later from the file itself
//so the file has each day saved,

//each get should probably calculate it but unsure still

/*uint64_t Process::getTotalTime() const {
	return totalTime;
}
*/
uint64_t Process::getSessionTime() const {
	return sessionTime;
}
uint64_t Process::getTodayTime() const {
	return todayTime;
}
uint64_t Process::getBackgroundTodayTime() const {
	return backgroundTodayTime;
}

/*uint64_t Process::getWeeklyTime() const {
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
*/


DWORD Process::getPid() const {
	return PID;
}


void Process::ResetTime() {
	//probably better to just delete the entire file so that it's "restarted"
	deleteFile(logFileName);
	fileCreator(logFileName);
}


//format has to be like 
//-HEAD 
//content
//-HEAD
//content and so on

//will have logs as the starter
//and then each day it's logged in 
void Process::fileCreator(string fileName, DWORD pidDef, string nameOfProcess) const {
	//string fileName = processName + "__" + to_string(PID) + ".pttl";
	ofstream file(fileName); //doesn't need appendmode 

	if (pidDef == -33 && nameOfProcess == "") {
		file << "- PID:" << endl << PID << endl << "- PROGRAM NAME:" << endl << processName << endl << "- LOGS:" << endl;
	}
	else if (pidDef == -33) {
		file << "- PID:" << endl << PID << endl << "- PROGRAM NAME:" << endl << nameOfProcess << endl << "- LOGS:" << endl;
	}
	else if (nameOfProcess == "") {
		file << "- PID:" << endl << pidDef << endl << "- PROGRAM NAME:" << endl << processName << endl << "- LOGS:" << endl;
	}
	else {
		file << "- PID:" << endl << pidDef << endl << "- PROGRAM NAME:" << endl << nameOfProcess << endl << "- LOGS:" << endl;
	}
	appendCurrentDateToFile(fileName, *this);
	
	file.close();

}


void Process::deleteFile(string fileName) { //so that it's safe it deletes the file but also resets all values so that it's initialized properly
	remove(fileName.c_str());
	resetInitial();
}

void Process::resetInitial() {
	sessionTime = todayTime = backgroundTodayTime = 0;
};


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


void appendCurrentDateToFile(string fileName, const Process& a) {
	ofstream file(fileName, std::ios::app); //append mode 
	file << currDay() << '/' << currMonth() << '/' << currYear() << ':' << a.getTodayTime() << ':' << a.getBackgroundTodayTime();
	file.close();
}