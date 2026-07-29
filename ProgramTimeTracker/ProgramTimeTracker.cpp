// ProgramTimeTracker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "AllProcesses.h"
#include "globals.h"
#include "Process.h"


using namespace std;

int main()
{
    AllProcesses tracker;
    tracker.getOpenedProcesses();
    cout << "starting";

    while (true) {
        int idleTime = tracker.getIdleSecondsFocusedProcess(videoModeEnabled, secsBeforeVideoTimeOut);
 
        if (idleTime == -1) {
            //save tracked time, here and appart from that if it's oon the else 1 min with a physical timer of 60secs that each iteration it sums 1 and then resets when there's a save, and when removeProcessWPID i should save 

        }
        else {
            tracker.getOpenedProcesses();
            if (idleTime < timeBeforeTimeOut) {
                //add 1 active time to active process
                //add 1 to background time to all
                tracker.addTimeActiveProcess(1);
                tracker.addTimeBackgroundProcesses(1, false);

            }
            else {
                
                tracker.addTimeBackgroundProcesses(1000, true);
            }

        }


        Sleep(1000);
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
