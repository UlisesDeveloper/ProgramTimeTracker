#include "AllProcesses.h"
#include <iostream>
#include <dwmapi.h>
#include "globals.h"
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <comdef.h>
#include <endpointvolume.h>

using namespace std;



struct pidList {
    size_t count = 0;
    size_t capacity = 0;
    DWORD* pids = nullptr;

    pidList() {
        count = 0;
        capacity = 16; //feel like 16 windows isn't a crazy number
        pids = new DWORD[capacity];
    }

    pidList(const pidList& a) { //jic
        //having it initialized by default causes a leak so better to have the def values in the def constructor even tho it makes no sense
        pids = nullptr;
        capacity = count = 0;
        *this = a;

    }

    pidList& operator=(const pidList& a) { //jic
        if (this != &(a)) {
            capacity = a.capacity;
            count = a.count;
            DWORD* temp = new DWORD[a.capacity];
            for (int i = 0; i < count; i++) {
                temp[i] = a.pids[i];
            }
            delete[] pids;
            pids = temp;
        }
        return *this;
    }

    ~pidList() {
        delete[] pids;
    }

    void addPid(DWORD pid) {
        if (count + 1 > capacity) {
            capacity *= 2;
            DWORD* temp = new DWORD[capacity];
            for (int i = 0; i < count; i++) {
                temp[i] = pids[i];
            }
            delete[] pids;
            pids = temp;
        }

        pids[count] = pid;
        count++;
    }
};





void AllProcesses::getOpenedProcesses() {
    DWORD tempNumProcesses = 0;
    Process* tempCurrentProcessList = nullptr;
    
    
    /*
    //the standart is not to do 2 enumprocesses calls it's to just call it as much times as we need until we can get the correct size
    DWORD cap = 1024;
    DWORD* pidTemp = new DWORD[cap];
    DWORD numPID = 0;
    
    
    getAllPID(cap, pidTemp, numPID);
    */
    


    pidList windowedPids;
    getActiveWindows(windowedPids);

    if (currentProcessList == nullptr) {
        getMetadataForPids(windowedPids, *this);
    }
    else {
        



        //we have to get the metadata only for the new processes
        pidList activeWindows;
        bool found = false;
        for (int i = 0; i < windowedPids.count; i++) {
            found = false;
            for (int j = 0; j < numProcesses && !found; j++) {
                if (windowedPids.pids[i] == currentProcessList[j].getPid()) {
                    found = true;
                }
            }

            if (!found) {
                activeWindows.addPid(windowedPids.pids[i]);
            }
        }

        getMetadataForPids(activeWindows, *this); //have to make it so that if there's already things in the allprocess to add not to remove

        //before or after getting the metadata i should remove all the old currentProcessList
        //prob in the while loop for time tracking i need to check the currentMouseProcess and if it has changed obv change the stopwatch
        

        //the metadata getter should only add processes because i am gonna pass only the newProcesses shouldn't have it doing bs
        //have to remove after adding the other
        
        //have to remove oldProcesses after adding the new ones
        
        bool remains = false;
        for (int i = 0; i < numProcesses; i++) {
            remains = false;
            for (int j = 0; j < windowedPids.count && !remains; j++) {
                if (currentProcessList[i].getPid() == windowedPids.pids[j]) {
                    remains = true;
                }
            }

            if(!remains) {
                removeProcessWPID(currentProcessList[i].getPid());
                i--;
            }
        }
    }
        

}



Process AllProcesses::getFocusedProcess() const {
    
    HWND main = GetForegroundWindow();
    DWORD pidFocused;
    GetWindowThreadProcessId(main, &pidFocused);
    int pos = -1;
    bool found = false;


    for (int i = 0; i < numProcesses && !found; i++) {
        if (currentProcessList[i].getPid() == pidFocused) {
            found = true;
            pos = i;
        }
    }

    if (found) {
        return currentProcessList[pos];
    }
    else {
        return systemAndMisc; //because if taskbar has control or start menu it would crash without this
    }
}
    

int AllProcesses::getIdleSecondsFocusedProcess(bool videoModeEnabled, int secsBeforeVideoTimeOut) const { //secsBeforeVideoTimeOut should be always higher than 5mins NEEDS TO BE CONTROLED or bigger than the global variable for timeout
    int idleTimeS = -1; //used as sentinel value because when main returns null it's by a windows protected prompt so i don't get anything therefore i cannot track it so i should standby and continue tracking when this isn't the case anymore
    HWND main = GetForegroundWindow();


    if (main != NULL) {
        
        LASTINPUTINFO lastInput;
        lastInput.cbSize = sizeof(LASTINPUTINFO);
        int res;

        if (!GetLastInputInfo(&lastInput)) { //bool to see if it could get it 
            throw invalid_argument("couldn't get lastinput info");
        }
        idleTimeS = (GetTickCount64() - lastInput.dwTime) / 1000; //dw time returns in ms, get tick info64 works cause it gives us time since the computer is on


        if (videoModeEnabled) { //0 is that no timeout so constantly primary if it's the focused process

            if (isWindowFullScreen(main) &&
                isWindowMultimediaTitle(main) &&
                isWindowUsingAudio(main) //prob better to put last because it has to do a lot of stuff
                ) {
                

                if (secsBeforeVideoTimeOut == 0) {
                    idleTimeS = 0;
                } //this is last inputs so i should hijack and show 1 sec less than the globalTimeOut constantly if it's under the secsbeforevideotimeout 
                else if (idleTimeS < secsBeforeVideoTimeOut && timeBeforeTimeOut < secsBeforeVideoTimeOut) {
                    idleTimeS = timeBeforeTimeOut - 1;
                }


            }

        }
    } 
    
    
    return idleTimeS;
}



void AllProcesses::addProcess(Process& a) {
    int newS = numProcesses + 1;
    Process* temp = new Process[newS]; 

    for (int i = 0; i < numProcesses; i++) {
        temp[i] = currentProcessList[i];
    }

    
    temp[newS - 1] = a;

    
    delete[] currentProcessList;
    currentProcessList = temp;
    numProcesses = newS;    
}



void AllProcesses::removeProcessWPID(DWORD PID) {
    
    //find pid first
    bool found = false;
    int pos = -1 ;
    for (int i = 0; i < numProcesses && !found; i++) {
        if (currentProcessList[i].getPid() == PID) {
            found = true;
            pos = i;
        }
        //dunno if i should add an exception if it wasn't found with the whole numProcesses - 1 == i thingie
    }


    if (pos >= 0 && pos < numProcesses){
        //overwrite that process by shifting
        for (int i = pos; i < numProcesses - 1; i++) {
            currentProcessList[i] = currentProcessList[i + 1];
        }
        numProcesses--;
        if (numProcesses != 0) {
            Process* temp = new Process[numProcesses];
            for (int i = 0; i < numProcesses; i++) {
                temp[i] = currentProcessList[i];
            }
            delete[] currentProcessList;
            currentProcessList = temp;
        }
        else {
            delete[] currentProcessList;
            currentProcessList = nullptr;
        }
    }
    
}


bool AllProcesses::isWindowFullScreen(HWND& main) const {
    HMONITOR hMonitor = MonitorFromWindow(main, MONITOR_DEFAULTTONEAREST); //gETS THE MONITOR

    MONITORINFO monInf;
    monInf.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &monInf); //saves into monInf the properties of hMonitor, hmonitor is like a dword for a monitor, and to get it's info we get it in a MONITORINFO thanks to GetMonitorInfo()

    int monitorWidth = monInf.rcMonitor.right - monInf.rcMonitor.left;
    int monitorHeight = monInf.rcMonitor.bottom - monInf.rcMonitor.top;

    RECT rectangle;//rectangle data type holds the 4 corners left right top bottom each edge
    GetWindowRect(main, &rectangle);
    //we need width and height which is done by doing r-l t-b
    int width = rectangle.right - rectangle.left;
    int height = rectangle.top - rectangle.bottom;

    //getsystemmetrics with the parameter of sm_cvscreen gets the physical monitor max width, but it gets 
    bool isFullScreen = (width >= monitorWidth) && (height >= monitorHeight);
}



void DeletePID(int pos, int &total, DWORD* array) {

    for (int j = pos; j < total - 1; j++) {
        array[j] = array[j + 1];
    }
    total--;

}





//To get all PIDs, we pass our arraycapacity by reference to nodify it if needed, then the actual array where all the PIDs will be, and a DWORD that holds the total number of PIDS
//Bascially what we do is call EnumProcess that works by giving it our array the capacity of it in bytes, and the pointer to the variable where we wanna store the number the number of bytes used (which we can extract the num of PIDS from)
//basically if the number of PIDS == cap we can almost be sured that the pids exceed the limit so we widen the array until we manage to fit all PIDS
void getAllPID(DWORD& cap, DWORD*& array, DWORD& numPID) {
    DWORD num_bytes = 0;
    bool fits = false;
    numPID = 0;


    while (!fits) {

        if (!EnumProcesses(array, cap * sizeof(DWORD), &num_bytes)) { //first parameter is the num of processes, the second is the size on bytes so that it doesn't overflow and the 3rd is the num of bytes actually used
            delete[] array; //to not leak memory
            throw out_of_range("couldn't get num of processes");
        }

        numPID = num_bytes / sizeof(DWORD); //to get the number of processes i am gonna need to know the size of a dword
        if (numPID == cap) { //would be crazy if 1024 isn't enough, and we have to make sure we have empty space because if num of processes is = to the capacity we should suspect that it didn't register all of them
            cap *= 2;
            delete[] array;
            DWORD* temp = new DWORD[cap];
            array = temp;
        }
        else {
            fits = true;
        }


    }
}




//Callback function to get the windows, enumwindows runs it for each individual window
//the callback is so that instead of c++ doing the memory cleanup, the old windows memory cleanup does it
//having it outside of the class means no need for it to be static, so it works like a global function pretty much 
BOOL CALLBACK filterNonPrimaryWindows(HWND hwnd, LPARAM lparam) { //normal c++ practices of efficiency like const & don't apply here cause they're too small and it has to match exactly the def
    //the lparam from enumwindow get's summoned to the callback as a parameter as well
   

    long exstyle = GetWindowLong(hwnd, GWL_EXSTYLE); //returns a long of binary due to the gwl each 0 or 1 is a property 
    //we are interested on a bit in concrete


    //this is for uwp apps because when as secondary processes they show as visible by the first check when they're not rendered
    int cloaked = 0;
    HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));//int where 0 is success, pos warnings, neg errors
    //cloaked will be 0 if it's visible 1 if it's not
    //hresult we can use succeeded to check if it did fine if not we have a problem

    //the hwnd is everything that is a window only so if firefox has a window per tab or save as window, then those count as a hwnd that will pass through the function
        if((IsWindowVisible(hwnd)) && //visible is that i can summon it, not that it's covered by another one
            (GetWindowTextLength(hwnd) != 0) && //if it's nothing it has no title so not important prob
            (GetWindow(hwnd, GW_OWNER) == 0) && //this basically returns the hierarchy thanks to the gw_owner if it's not 0 it's not the leader [Getwindow get's a tag result of a window]
            !(exstyle & WS_EX_TOOLWINDOW) //we use the & bitwise and operator which basically searches that property position in the integer
            //and ws ex toolwindow is one that doesn't appear on the taskbar nor alt tab
            ) {

            

            if (SUCCEEDED(hr) && cloaked == 0) {
                //with all these checks we can be mostly sure it's an active app
                DWORD pidRes = -1;
                
                
                GetWindowThreadProcessId(hwnd, &pidRes);

                //add pid to lparam
                //the lparam is our struct we have to unmask it
                pidList* unmaskedLparam = (pidList*)lparam; //lparam is a mem adress so i have to work with it

                (*unmaskedLparam).addPid(pidRes);
                
                
            }



        }
        

        return true; //if warnings show just change it to TRUE
}
//CAN'T THROW EXCEPTIONS IN CALLBACKS, so i have to basically just obtain the valid pids


//has to be after the struct declaration
void getActiveWindows(pidList& a) {
    //I HAVE NOW A LIST OF PROCESSES WITH THEIR NAMES/PID NOW I HAVE TO CHECK IF THE WINDOW IS VISIBLE

    //enum windows gives you every window
    //the first parameter you do a callback where you pass a function, YOU PASS IT WITHOUT PARAMETERS cause it's a pointer to the function so for each window it will run that function, IF PARAMETERS WERE PUT like a normal funciton in the first parameter it will run it and pass the result as a parameter
    //because the data type of the callback is a WNDENUMPROC it expects the function having as it's parameter in the definition a HWND and an LPARAM and it has to be
    // a BOOL CALLBACK, callback is so that it can run the function at the time it wants

    //Second one is where the results are saved so an array, but it's an LPARAM so in one parameter you need to pass everything the pointer the size capacity, that's why it should be a struct, because it only takes one parameter

    //This are the actual shown windows not the other pids which are a list for the names nothing more 
    //prob better to use normal ass vector but still wanna make the struct thingie


    if (EnumWindows(filterNonPrimaryWindows, (LPARAM)&a) == 0) { //thought of passing this but it's only logical to have the struct as a mediator to not do hard drive operations, only when an active process has been closed or opened.
        //we cast it to an lparam which is a pointer and we pass the mem adress of the struct, that's the &'s purpose
        throw invalid_argument("couldn't get windows enumwindows");
    }
}



void getMetadataForPids(const pidList& pidLs, AllProcesses& allP) {


    //do these one ive gotten the active windows and only for the ones that are new not the ones i already know the names off
    for (int i = 0; i < pidLs.count; i++) {

        //first thing is too openProcess if it fails it's a kernel/antivirus so i discard the pid
        HANDLE processOpenAttemptRes = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pidLs.pids[i]);
        // open process first argument is intention (read only here), second is if child processes of the program do they inherit the property, and third the pid
        //might want to add PROCESS_VM_READ with an or to the intention later on


        //all of this just to get the name of the process if it has been able to open it ofc
        if (processOpenAttemptRes) {


            DWORD bufferCharSize = MAX_PATH;
            DWORD oldBufferCharSize = bufferCharSize;
            char* bufferChar = new char[bufferCharSize];
            bool works = false;

            bool queryNameAttemptRes = false;
            while (!works) {
                queryNameAttemptRes = QueryFullProcessImageNameA(processOpenAttemptRes, 0, bufferChar, &bufferCharSize);
                //first we check if we have opened the process if not it cannot query the name
                //second is format so we put 0 as the default one, third is the name itself but it needs a buffer (char array) 
                //windows has one called tchar with MAX_PATH, and the last parameter is input/output you put the size of the buffer and it returns
                //how much it used so we need to make sure it didn't get chopped off


                if (bufferCharSize == oldBufferCharSize && !queryNameAttemptRes) { //if it has returned the same size it probably didn't fit it in it's entirety
                    oldBufferCharSize = bufferCharSize;             //basically also go into else to stop looping constantly if we can't get the name
                    bufferCharSize *= 2;
                    delete[] bufferChar;
                    char* temp = new char[bufferCharSize];
                    bufferChar = temp;
                }
                else {
                    works = true;
                }

            }

            if (!queryNameAttemptRes) {
                //couldn't get the name so no acces prob therefore not an active one that matters
                DeletePID(i, pidLs.count, pidLs.pids);
                i--; //bcuz it moved the elements the i position has a new pid
            }
            else {


                //!!
                //!!
                //!! 
                //!!
                //!!
                //!!! change asap ts should only be done when the comparison between currentProcessList pids and the ones obtained from the function called are confirmed!!!
                //ts is a bad idea not nice to grab the pids exe names for no reason if im gonna get rid of a good sum


                //get full name from it
                string processName(bufferChar, bufferCharSize); //we initialize the string with the constructor with an array pointer and it's size
                //rfind is the same as a find but reverse
                size_t posLastSlash = processName.rfind('\\'); //using \ as escape and then the actual char

                if (posLastSlash == string::npos) {
                    throw invalid_argument("couldn't disect exe file name from directory");
                }
                processName = processName.substr(posLastSlash + 1); //includes exe

                //shoud have at this point a PID with a good processName

                if (processName.rfind('.') == std::string::npos) {
                    //has no extension weird
                    Process temp(processName, pidLs.pids[i], processName);
                    allP.addProcess(temp);
                }
                else {
                    string processNameWOExtension = processName.substr(0,  processName.rfind('.'));
                    Process temp(processName, pidLs.pids[i], processNameWOExtension);
                    allP.addProcess(temp);
                }
                //with this i think i have succesfully added the process




                /*
                //we always want for numProcesses to be accurate so it will have to be resized constantly

                Process* temp = new Process[tempNumProcesses + 1];
                for (int i = 0; i < tempNumProcesses; i++) {
                    temp[i] = tempCurrentProcessList[i];
                }

                temp[tempNumProcesses].PID = pidTemp[i];
                temp[tempNumProcesses].processName = processName;


                //
                // IMPORTANT
                // 
                // 
                // 
                //have to make sure to add the filename and open it so that data is loaded ig



                tempNumProcesses++;
                delete[] tempCurrentProcessList;
                tempCurrentProcessList = temp;
                */

            }



            delete[] bufferChar; //free up dyn memory used to extract name
            //once everything has been done we need to closeProcess obv only has it open if we have entered the if
            CloseHandle(processOpenAttemptRes);
        }
        else {
            //need to move all to the left you know, probably a protected process NOT IMPORTANT
            /* for (int j = i; j < currNumProcesses - 1; j++) {
                pidTemp[j] = pidTemp[j + 1];
            }
            currNumProcesses--; */


            DeletePID(i, pidLs.count, pidLs.pids);
            i--; //bcuz it moved the elements the i position has a new pid
        }


    }


}




bool isWindowMultimediaTitle(HWND main) {
    bool res = false;
    int size = 256;
    char* winName = new char[size];

    int charsUsed = GetWindowTextA(main, winName, size);

    while (charsUsed == size) { //This is most likely that it got cut off
        size *= 2;
        delete[] winName;
        charsUsed = GetWindowTextA(main, winName, size);
    }

    string windowName = string(winName);

    for (size_t i = 0; i < windowName.length(); i++) {
        windowName[i] = std::tolower(static_cast<unsigned char>(windowName[i]));
    }
    for (int i = 0; i < numOfMultimediaProviders; i++) {

        //no native to lower so i have to do it myself
        for (size_t j = 0; j < multimediaProviders[i].length(); j++) { //CAN BE REMOVED IF I DO A CHECK IN GLOBALS THAT ALL THE INPUTTED AND addProvider is automatically converted to lowercase
            multimediaProviders[i][j] = std::tolower(static_cast<unsigned char>(multimediaProviders[i][j]));
        }

        size_t matchBegin = windowName.find(multimediaProviders[i]);
        if (matchBegin != string::npos) {
            res = true;
        }
    }

    delete[] winName;

    return res;
}


DWORD getPidFromHWND(HWND main) {
    DWORD pid;
    if (GetWindowThreadProcessId(main, &pid) == 0) { //returns 0 on fail
        throw invalid_argument("couldn't get pid from hwnd");
    }
    return pid;
}



bool isWindowUsingAudio(HWND main) {
    bool res = false;
    //apparently the popup api for default audio management isn't that reliable i have to check the actual waves produced smh
    
    
    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) { //Starts component object model for the thread of this program, com is used to talk to drivers with objects without it i can't create audio objects
        throw invalid_argument("couldn't sart com engine");       //the first parameter is NULL always cause it's never used
    }                                                             //second parameter is a flag slot with the COINIT_APARTMENTTHREADED we are assuring it that the objects will only be in a thread (apartment) and that only the thread that created it will use them 
    else {
        IMMDeviceEnumerator* pEnumerator = NULL; //multimedia datatype pointer
        //imm is used to interact with mm because u can't 

        //here we create an object for audio
        HRESULT audioObjectCreator = CoCreateInstance(
            __uuidof(MMDeviceEnumerator), //first parameter we indicate the kind of object we want so uuidof recognizes what immdeviceenumerator is multimedia type and creates a blueprint
            NULL, // second parameter used to combine it with another COM object not the case so null
            CLSCTX_ALL, //third parameter is where the code for the object is stored clsctx_all is an idc
            __uuidof(IMMDeviceEnumerator), //fourth parameter is the kind of object we are going to interact with 
            (void**)&pEnumerator //fifth parameter the object itself which it works with (void**) is the cast so that it works in an LPVOID
        );


        IMMDevice* pDevice = NULL; //Device being used to listen
        HRESULT audioDeviceGetter = (*pEnumerator).GetDefaultAudioEndpoint(
            eRender, //to indicate u care about the output if it was eCapture u care about the input
            eConsole, //master System Audio
            &pDevice //Returns the device IMMDevice
        ); //now with this we have the device that is using the master audio


        //now we disect the master audio coming out from the output device,  we are gonna create a master audio manager
        IAudioSessionManager2* pManager = NULL; //wtf why is there 2 and have to continue
        //2 because the first one was bad, and getsessionenumerator is from the 2nd version

        HRESULT hrManager = (*pDevice).Activate(
            __uuidof(IAudioSessionManager2), //we want to access the audio manager for the speakers
            CLSCTX_ALL, //where the code is stores idc
            NULL, //for network activation, because it's local it doesn't matter
            (void**)&pManager //output of the manager 
        );



        //now we have to get from the manager all the "cables" so apps that are transmitting audio
        IAudioSessionEnumerator* pSessionList = NULL; //This holds all the apps that are transmitting audio throught the specificed deviced of the manager

        HRESULT hrList = (*pManager).GetSessionEnumerator(&pSessionList); //only takes an argument where it returns the apps that are using audio session which is active
        
        DWORD winPid = 0;
        GetWindowThreadProcessId(main, &winPid);
        if (GetWindowThreadProcessId(main, &winPid) != 0 && winPid > 0) {
            
            int count = 0;
            (*pSessionList).GetCount(&count);
            
            bool found = false;
            for (int i = 0; i < count && !false; i++) {
                IAudioSessionControl* pSessionControl = NULL; //this is for a specific session (process) not the whole list
                (*pSessionList).GetSession(i, &pSessionControl); //we get the session from the i position in the lsit
                

                //crazy to understand this
                //but iaudiosessioncontrol doesn't have the capacity to return a pid, so we need iaudiosessioncontrol2 BUT TO GET THE SESSION we need the first version
                IAudioSessionControl2* pSessionControl2 = NULL;
                (*pSessionControl).QueryInterface( //This makes pSessionControl2 have the features required for the pid but it requires an audiosessioncontrol1
                    __uuidof(IAudioSessionControl2), //specify the data type of the output object
                    (void**)&pSessionControl2); //with void we strip the data type of psessioncontrol (output) hence the first parameter
                
                //now we extract the pid
                DWORD currentPidSession = 0;
                (*pSessionControl2).GetProcessId(&currentPidSession);
                if (currentPidSession == winPid ) {
                    
                    //now we have found in the pSessionList the process from the window but just because it's in the audioList deostn't mean it's actively using audio it can be paused or whatever

                    IAudioMeterInformation* pMeter = NULL; //audio information from a session
                    (*pSessionControl).QueryInterface( //We do a query interface of the session control of the actual process from the window
                        __uuidof(IAudioMeterInformation), //data type of output
                        (void**)&pMeter //output 
                    );


                    if (pMeter != NULL) { //something was obtained
                        float vol = 0.0f;

                        (*pMeter).GetPeakValue(&vol);
                        
                        if (vol > 0.0f) {
                            //there's audio playing 
                            res = true;
                            found = true;
                        }
                    }

                }




                //need to release each pSessionControl because if not it leaks ram
                (*pSessionControl2).Release();
                (*pSessionControl).Release();


            }



        }
        else {
            throw invalid_argument("couldn't get pid from the window audioWindow or pid returned != > 0");
        }


        //not only the sessioncontrol, but also the enumerators, managers, device enumerator, and the coInitialize, similar to a .close() when dealing with files
        if (pSessionList) pSessionList->Release();
        if (pManager) pManager->Release();
        if (pDevice) pDevice->Release();
        if (pEnumerator) pEnumerator->Release();

    }

    
    CoUninitialize();

    return res;

}