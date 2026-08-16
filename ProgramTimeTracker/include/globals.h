#pragma once
#include <string>

extern bool antiProcrastination;

extern int timeBeforeTimeOut;
extern int secsBeforeVideoTimeOut;
extern bool videoModeEnabled;

extern int numOfMultimediaProviders;
extern std::string* multimediaProviders;



extern bool g_RunStartup;	
extern bool g_DebugMode;

bool CheckIfRunsAtStartup();
void RunAtStartup(bool enable);


