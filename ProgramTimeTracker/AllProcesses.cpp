#include "AllProcesses.h"
#include <iostream>
#include <dwmapi.h>


using namespace std;


void AllProcesses::getOpenedProcesses() {
    DWORD tempNumProcesses = 0;
    Process* tempCurrentProcessList = nullptr;
    
    
    
    //the standart is not to do 2 enumprocesses calls it's to just call it as much times as we need until we can get the correct size
    DWORD cap = 1024;
    DWORD* pidTemp = new DWORD[cap];
    DWORD numPID = 0;
    
    
    getAllPID(cap, pidTemp, numPID);
    
    


    for (int i = 0; i < numPID; i++) {

        //first thing is too openProcess if it fails it's a kernel/antivirus so i discard the pid
        HANDLE processOpenAttemptRes = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pidTemp[i]);
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
                DeletePID(i, numPID, pidTemp);
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
                processName = processName.substr(posLastSlash + 1);

                //shoud have at this point a PID with a good processName
                //still will have to check if they're invisible windows or they are not visible

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


            DeletePID(i, numPID, pidTemp);
            i--; //bcuz it moved the elements the i position has a new pid
        }


    }






    //I HAVE NOW A LIST OF PROCESSES WITH THEIR NAMES/PID NOW I HAVE TO CHECK IF THE WINDOW IS VISIBLE

    //enum windows gives you every window
    //the first parameter you do a callback where you pass a function, YOU PASS IT WITHOUT PARAMETERS cause it's a pointer to the function so for each window it will run that function, IF PARAMETERS WERE PUT like a normal funciton in the first parameter it will run it and pass the result as a parameter
    //because the data type of the callback is a WNDENUMPROC it expects the function having as it's parameter in the definition a HWND and an LPARAM and it has to be
    // a BOOL CALLBACK, callback is so that it can run the function at the time it wants
    
    //Second one is where the results are saved so an array, but it's an LPARAM so in one parameter you need to pass everything the pointer the size capacity, that's why it should be a struct, because it only takes one parameter
    


    //the crazy thing is that hwnd








    //This are the actual shown windows not the other pids which are a list for the names nothing more 
    //prob better to use normal ass vector but still wanna make the struct thingie
    
    pidList windowedPids;

    if (EnumWindows(filterNonPrimaryWindows,(LPARAM)&windowedPids) == 0) { //thought of passing this but it's only logical to have the struct as a mediator to not do hard drive operations, only when an active process has been closed or opened.
        //we cast it to an lparam which is a pointer and we pass the mem adress of the struct, that's the &'s purpose
        throw invalid_argument("couldn't get windows enumwindows");
    }

    //now windowedPids have the active pids.
    //if a pid is -1 should be ignored cuz GetWindowThreadProcessId failed


}



void DeletePID(int pos, int total, DWORD* array) {

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
