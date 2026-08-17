#pragma once
#include <string>

//Flags not save in file ts is for execution

extern bool hasRestrictionsChanged;


//SETTINGS WINDOW GLOBALS

extern int timeBeforeTimeOut;
extern int secsBeforeVideoTimeOut;
extern bool videoModeEnabled;

extern int numOfMultimediaProviders;
extern std::string* multimediaProviders;



extern bool g_RunStartup;	
extern bool g_DebugMode;

//ANTIPROCRASTINATION WINDOW GLOBALS
extern bool antiProcrastination;

extern bool snooze;
extern int snoozeMins;
extern bool killAfterSnooze;


bool CheckIfRunsAtStartup();
void RunAtStartup(bool enable);


