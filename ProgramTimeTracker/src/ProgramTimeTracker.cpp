// ProgramTimeTracker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "AllProcesses.h"
#include "globals.h"
#include "Process.h"
#include <filesystem>


using namespace std;

int main()
{
    filesystem::create_directory("logs");
    AllProcesses tracker;
    tracker.getOpenedProcesses();
    cout << "starting";
    bool stillUAC = false;
    int startDay = currDay();
    int timeSinceAutoSave = 0;




    while (true) {
        int idleTime = tracker.getIdleSecondsFocusedProcess(videoModeEnabled, secsBeforeVideoTimeOut); //Ts shouldn't take arguments they're globals
        
        if (idleTime == -1) {
            //save tracked time, here and appart from that if it's oon the else 1 min with a physical timer of 60secs that each iteration it sums 1 and then resets when there's a save, and when removeProcessWPID i should save 
            //-1 is because i can't access the window cause it's protected, so we save time to file, if it is constantly -1 then it's stuck in UAC so we should do nothing
            
            //add second to systemAndMisc process as well;

            if (!stillUAC) {
                tracker.saveTime();
            }

            //the time for system in my opinion should only be able to be saved after it has exited system time imo, because if not im gonna have to be saving it constantly
            tracker.addTimeToSystemProcess(1); //only saved by autosave
            stillUAC = true;
        }
        else {
            stillUAC = false;
            if (startDay != currDay()) {
                tracker.saveTime();
                tracker.resetDayTime();
                startDay = currDay();
            }

            tracker.getOpenedProcesses();
            if (idleTime < timeBeforeTimeOut) {
                //add 1 active time to active process
                //add 1 to background time to all
                tracker.addTimeActiveProcess(1);
                tracker.addTimeBackgroundProcesses(1, false);

            }
            else {
                
                tracker.addTimeBackgroundProcesses(1, true); //basically also adds background time to "active" process after the timeout
            }

        }


        Sleep(1000);
        timeSinceAutoSave++;
        if (timeSinceAutoSave >= 300) {
            tracker.saveTime();
            timeSinceAutoSave = 0;
        }
    }

    

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
