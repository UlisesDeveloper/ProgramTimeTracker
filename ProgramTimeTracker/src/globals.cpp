#include "globals.h"
#include "windows.h"

//IMPLEMENT FILE WHERE GLOBALS ARE SAVED AND I CAN LOAD CONFIG EACH TIME THE PROGRAM IS STARTED CAUSE IF NOT IT'S RAM BASED

int timeBeforeTimeOut = 300; //in secs unless the name specifies otherwise
int secsBeforeVideoTimeOut = 600;
bool videoModeEnabled = false;

int numOfMultimediaProviders = 10;
//ts is temporary they should be loaded from the file and they should have these as defaults
std::string* multimediaProviders = new std::string[10]{ "youtube", "Microsoft.Media.Player", "netflix", "vlc", "hulu", "prime video", "disney+", "apple tv", "twitch", "mpv"};


bool g_RunStartup = true; //by default
const wchar_t* REGISTRY_APP_NAME = L"ProgramTimeTracker";



bool CheckIfRunsAtStartup() { //if user has changed it thorung the task manager checker
	bool res = false;
	HKEY hKey;
	LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS)
	{
		result = RegQueryValueExW(hKey, REGISTRY_APP_NAME, nullptr, nullptr, nullptr, nullptr);
		RegCloseKey(hKey);
		res = (result == ERROR_SUCCESS);
	}

	return res;

}


void RunAtStartup(bool enable)
{
    HKEY hKey;
    
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);

    if (result == ERROR_SUCCESS)
    {
        if (enable)
        {
            //full path to exactly where your .exe is currently running from
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            
            //the exepath we need to add "" + hidden flag
            wchar_t commandPath[MAX_PATH + 20];
            swprintf_s(commandPath, MAX_PATH + 20, L"\"%s\" --hidden", exePath);


            //Gets added
            RegSetValueExW(hKey, REGISTRY_APP_NAME, 0, REG_SZ, (BYTE*)exePath, (wcslen(exePath) + 1) * sizeof(wchar_t));
        }
        else
        {
            //to remove it
            RegDeleteValueW(hKey, REGISTRY_APP_NAME);
        }
        RegCloseKey(hKey);
    }
}
