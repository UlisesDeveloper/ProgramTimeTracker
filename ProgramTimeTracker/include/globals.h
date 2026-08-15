#pragma once
#include <string>



extern int timeBeforeTimeOut;
extern int secsBeforeVideoTimeOut;
extern bool videoModeEnabled;

extern int numOfMultimediaProviders;
extern std::string* multimediaProviders;



extern bool g_RunStartup;	

bool CheckIfRunsAtStartup();
void RunAtStartup(bool enable);


