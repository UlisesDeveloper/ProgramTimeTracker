#include "AllProcesses.h"
#include <iostream>


using namespace std;


void AllProcesses::getOpenedProcesses() {

    DWORD num_bytes = 0;

    if (!EnumProcesses(NULL, 0, &num_bytes)) {
        throw out_of_range("couldn't get num of processes");
    }

    numProcesses = num_bytes / sizeof(DWORD); //to get the number of processes i am gonna need to know the size of a dword

    
    //now create the array and fill it with each PID 
    //need to make sure nothing has changed from the numProcesses saved and what is obtained in the second call
}