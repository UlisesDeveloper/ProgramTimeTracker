#pragma once
#include <iostream>
#include "AllProcesses.h"
#include "Process.h"
#include <string>
#include "imgui.h"
#include <filesystem>
#include "auxiliaryMainFunctions.h"

using namespace std;


struct statData {
    int size = 0; //same size for 3 things below, because it's for each entry
    Process* queryProcesses = nullptr;
    int* queryActiveSecs = nullptr;
    int* queryBackgroundSecs = nullptr;


    //retroactive viewing for graphs so if it's week then day, month then week and so on, necesary to get info for graphs
    bool retroActive = false; //if it's true, then it's not a day and theres retroactive information
    int retroActive_size;
    Process* retroActive_queryProcesses = nullptr; //pointer to pointer because if it's a week then we need to get all the days so pointer to day and then to all the days in week, or if it's month pointer to week and then to all the weeks
    int* retroActive_queryActiveSecs = nullptr;
    int* retroActive_queryBackgroundSecs = nullptr;


    //Functions
    ~statData() {
        delete[] queryProcesses;
        delete[] queryActiveSecs;
        delete[] queryBackgroundSecs;
        delete[] retroActive_queryProcesses;
        delete[] retroActive_queryActiveSecs;
        delete[] retroActive_queryBackgroundSecs;
    }

    statData& operator+=(const statData& other) { //Add to ours
        if (this != &other && other.size > 0) {
            Process* TEMPqueryProcesses = new Process[size + other.size];
            int* TEMPqueryActiveSecs = new int[size + other.size];
            int* TEMPqueryBackgroundSecs = new int[size + other.size];

            for (int i = 0; i < size; i++) {
                TEMPqueryProcesses[i] = queryProcesses[i];
                TEMPqueryActiveSecs[i] = queryActiveSecs[i];
                TEMPqueryBackgroundSecs[i] = queryBackgroundSecs[i];
            }

            //now copy the new contents
            for (int i = 0; i < other.size; i++) {
                TEMPqueryProcesses[size + i] = other.queryProcesses[i];
                TEMPqueryActiveSecs[size + i] = other.queryActiveSecs[i];
                TEMPqueryBackgroundSecs[size + i] = other.queryBackgroundSecs[i];
            }

            size += other.size;
            delete[] queryProcesses;
            delete[] queryActiveSecs;
            delete[] queryBackgroundSecs;

            queryProcesses = TEMPqueryProcesses;
            queryActiveSecs = TEMPqueryActiveSecs;
            queryBackgroundSecs = TEMPqueryBackgroundSecs;

            //know that i have to do something about the retroactive but not yet i have no idea still

        }
        return *this;
    }

    void sortActiveTime() {
        for (int i = 0; i < size; i++) {
            for (int j = i + 1; j < size; j++) {
                if (queryActiveSecs[i] < queryActiveSecs[j]) {

                    int TEMPqueryActiveSecs = queryActiveSecs[i];
                    int TEMPqueryBackgroundSecs = queryBackgroundSecs[i];
                    Process TEMPqueryProcesses = queryProcesses[i];

                    queryActiveSecs[i] = queryActiveSecs[j];
                    queryBackgroundSecs[i] = queryBackgroundSecs[j];
                    queryProcesses[i] = queryProcesses[j];

                    queryActiveSecs[j] = TEMPqueryActiveSecs;
                    queryBackgroundSecs[j] = TEMPqueryBackgroundSecs;
                    queryProcesses[j] = TEMPqueryProcesses;
                }
                else if (queryActiveSecs[i] == queryActiveSecs[j] ) {
                    if (queryBackgroundSecs[i] < queryActiveSecs[j]) {

                        int TEMPqueryActiveSecs = queryActiveSecs[i];
                        int TEMPqueryBackgroundSecs = queryBackgroundSecs[i];
                        Process TEMPqueryProcesses = queryProcesses[i];

                        queryActiveSecs[i] = queryActiveSecs[j];
                        queryBackgroundSecs[i] = queryBackgroundSecs[j];
                        queryProcesses[i] = queryProcesses[j];

                        queryActiveSecs[j] = TEMPqueryActiveSecs;
                        queryBackgroundSecs[j] = TEMPqueryBackgroundSecs;
                        queryProcesses[j] = TEMPqueryProcesses;
                    }
                }
            }
        }
    }
    


};





//main stats selector
void statsShower(AllProcesses& tracker, int selectedStat, ID3D11Device* d3dDevice);

statData* getEntriesFor(int selectedStat); //it has to be higher than 0,


statData* retrieveStatsFromFile(const string& filePath, int selectedStat);