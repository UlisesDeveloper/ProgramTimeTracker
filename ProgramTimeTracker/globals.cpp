#include "globals.h"

//IMPLEMENT FILE WHERE GLOBALS ARE SAVED AND I CAN LOAD CONFIG EACH TIME THE PROGRAM IS STARTED CAUSE IF NOT IT'S RAM BASED

int timeBeforeTimeOut = 3000; //in secs unless the name specifies otherwise

int numOfMultimediaProviders = 10;
//ts is temporary they should be loaded from the file and they should have these as defaults
string* multimediaProviders = new string[10]{ "youtube", "Microsoft.Media.Player", "netflix", "vlc", "hulu", "prime video", "disney+", "apple tv", "twitch", "mpv"};


