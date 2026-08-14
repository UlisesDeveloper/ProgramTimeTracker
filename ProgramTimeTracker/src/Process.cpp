#include "Process.h"
#include <fstream>
#include <filesystem>

//PID on files is for diagnostic it changes a lot so it's not important

Process::Process() {

}



Process::Process(string fileName, DWORD pidDef, string nameOfProcess, string pathNamed) { //horrid idea how does it get the pid has to have the pid name and whatever else

	ifstream file(fileName);
	logFileName = fileName;
	pathName = pathNamed;


	PID = pidDef;
	if (nameOfProcess != "") {
		processName = nameOfProcess;
	}


	if (file.is_open()) {
		string currentLine = "";
		string lastLine = "";
		

		//this should be replaced to something that gets the last line and is saved in currentLine
		while (getline(file, currentLine)) {
			if (currentLine != "") {
				lastLine = currentLine;
			}
		}

		currentLine = lastLine;


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

string Process::getProcessName() const {
	return processName;
}
string Process::getLogFileName() const {
	return logFileName;
}
string Process::getPathName() const {
	return pathName;
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

	file.close();

	appendCurrentDateToFile(fileName, *this);


}


void Process::deleteFile(string fileName) { //so that it's safe it deletes the file but also resets all values so that it's initialized properly
	remove(fileName.c_str());
	resetDayTime();
	sessionTime = 0;
}



void Process::saveTime() {
	//atp files have been created and everything and it has todays date and everything so only have to modify the time
	ifstream file(logFileName); //append mode doesn't work cause it takes us to the end of last line, so we use standart rw mode
	string currentLine = "";
	
	
	if (file.is_open()) {
		if (!(filesystem::is_empty(logFileName))) {
			string* fileContents = new string[300];
			int stringsFileContents = 300;
			int stringsUsed = 0;


			while (getline(file, currentLine)) {
				//Cannot delete lines nor do thinks like modifying specific things need ram rewrite
				
				//a vector/array of strings can make sense due to not having to lose ur mind finding the last line
				
				if (stringsUsed == stringsFileContents) {
					string* temp = new string[stringsFileContents * 2];
					for (int i = 0; i < stringsFileContents; i++) {
						temp[i] = fileContents[i];
					}
					stringsFileContents *= 2;
					delete[] fileContents;
					fileContents = temp;
				}


				fileContents[stringsUsed] = currentLine;
				stringsUsed++;
			}
			stringsUsed--; //no neglines problem cause we checked that the file wasn't empty

			//StringsUsed is the centinel value to know where the array starts to have empty values and also to have the last day tracked
			
			//atp we got everything we wanted from the ifstream
			file.close();


			size_t posFirstColon = fileContents[stringsUsed].find(':');
			size_t posSecondColon = fileContents[stringsUsed].find(':', posFirstColon + 1);
			string date = fileContents[stringsUsed].substr(0, posFirstColon);
			//find gives first pos
			size_t posFirstSlash = date.find('/');
			size_t posSecondSlash = date.find('/', posFirstSlash + 1);

			if (posFirstColon == string::npos || posSecondColon == string::npos ||
				posFirstSlash == string::npos || posSecondSlash == string::npos) {
				delete[] fileContents;
				throw invalid_argument("wrong format in process file");
			}


			if ((stoi(date.substr(0, posFirstSlash)) != currDay())
				|| (stoi(date.substr(posFirstSlash + 1, posSecondSlash - posFirstSlash - 1)) != currMonth())
				|| (stoi(date.substr(posSecondSlash + 1)) != currYear())
				) {
				stringsUsed++;

				 
			}
				//Doesn't matter if currentDate is in the string or not we are gonna overwrite it anyways
				//only need to make sure that if it's not to put it in the next string, but if it's there then overwrite the currentOne
				//so with stringsUsed growing by 1 if the last entry isn't today, we can now modify the one in stringsUsed

			string temp = to_string(currDay()) + '/' + to_string(currMonth()) + '/' + to_string(currYear()) + ':' + to_string(todayTime) + ':' + to_string(backgroundTodayTime);
			fileContents[stringsUsed] = temp;


			//now rewrite to the file
			ofstream fileO(logFileName); //no append mode we intend to overwrite
			for (int i = 0; i <= stringsUsed; i++) {
				fileO << fileContents[i] << endl;
			}

			fileO.close();

			delete[] fileContents;

		}
		else {
			fileCreator(logFileName, PID, processName);
			(*this).saveTime(); //recursion after file has been filled with contents  so it enters the 2nd empty 
		}
	}
	else {
		throw invalid_argument("file couldn't open");
	}


}


void Process::resetDayTime() {
	todayTime = backgroundTodayTime = 0;
}

int yearFrom2026() {

	time_t now = time(nullptr);
	tm local;
	localtime_s(&local, &now);//this transforms the time to the local structure
	

	int currYear = local.tm_year + 1900;

	if ((currYear - 2026) < 0) {
		throw out_of_range("Year can't be before 2026, Correct System Time");
		//has to be catched eventually and show an error box
	}

	return (currYear - 2026);
}

int currDay() {
	time_t now = time(nullptr);
	tm local;
	localtime_s(&local, &now);
	int day = local.tm_mday; //ret 1 to 31 no 0 start 

	return day;
}

int currMonth() {
	time_t now = time(nullptr);
	tm local;
	localtime_s(&local, &now);
	int month = local.tm_mon + 1; //so no 0 based 
	return month;
}

int currYear() {
	time_t now = time(nullptr);
	tm local;
	localtime_s(&local, &now);//this transforms the time to the local structure

	int currYear = local.tm_year + 1900;
	return currYear;
}


void appendCurrentDateToFile(string fileName, const Process& a) {
	ofstream file(fileName, std::ios::app); //append mode 
	if (!(file.is_open())) {
		throw invalid_argument("file couldn't open");
	}

	file << currDay() << '/' << currMonth() << '/' << currYear() << ':' << a.getTodayTime() << ':' << a.getBackgroundTodayTime() << endl; //Do the endl so always a date is added to the file append mode will be in the empty end line
	file.close();
}