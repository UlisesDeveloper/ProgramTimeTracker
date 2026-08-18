#include "auxiliaryMainStatShower.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Process.h"

namespace fs = filesystem; //to not put filesystem too long
using namespace std;


    



void statsShower(AllProcesses& tracker, int selectedStat, ID3D11Device* d3dDevice) {



    static int previousStat = -1;
    static statData* cachedData = nullptr; //URGENT to not waste cpu and io cycles
    static ID3D11ShaderResourceView** cachedIcons = nullptr;

    //if user changed tabs
    if (previousStat != selectedStat) {
        if (cachedData != nullptr) {
            delete cachedData;
            cachedData = nullptr;
        }
        previousStat = selectedStat;


        if (cachedIcons != nullptr && cachedData != nullptr) {
            for (int i = 0; i < (*cachedData).size; i++) {
                if (cachedIcons[i]) {
                    (*cachedIcons[i]).Release(); //release the vram
                }
            }
            delete[] cachedIcons;
            cachedIcons = nullptr;
        }


    }



    if (selectedStat == 0) {
        Process activeApp = tracker.getFocusedProcess();
        ImGui::Text("Currently Focused PID: %lu", activeApp.getPid());
        ImGui::Text("Currently Focused Name: %s", activeApp.getProcessName().c_str());
        ImGui::Text("Currently Focused Path: %s", activeApp.getLogFileName().c_str());
        ImGui::Text("Currently Focused Path: %s", activeApp.getPathName().c_str());
        ImGui::Text("Active Time Today: %llu seconds", activeApp.getTodayTime());
        ImGui::Text("Background Time Today: %llu seconds", activeApp.getBackgroundTodayTime());
    }
    else if (selectedStat >= 1 && selectedStat <= 5) {

        //Cache loading
        if (cachedData == nullptr) {
            cachedData = getEntriesFor(selectedStat);

            if (cachedData != nullptr && cachedData->size > 0) {
                (*cachedData).sortActiveTime();

                //load textures once
                cachedIcons = new ID3D11ShaderResourceView * [(*cachedData).size];
                for (int i = 0; i < (*cachedData).size; i++) {
                    const Process* processPointer = nullptr;
                    if (tracker.getProcessFromPath((*cachedData).queryProcesses[i].getPathName(), processPointer)) { //[cite: 5]
                        cachedIcons[i] = (ID3D11ShaderResourceView*)showIconFromProcess(*processPointer, d3dDevice);
                    }
                    else {
                        cachedIcons[i] = showIconFromPath((*cachedData).queryProcesses[i].getPathName(), d3dDevice);
                    }
                }
            }
        }

        
        if (cachedData != nullptr && (*cachedData).size > 0) {

            // 4 if it's day rest 5
            int numColumns = (selectedStat == 1) ? 4 : 5;

            if (ImGui::BeginTable("StatsTable", numColumns, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {

                ImGui::TableSetupColumn("Program Name");
                ImGui::TableSetupColumn("Active Time (Hours:Mins:Secs)");
                ImGui::TableSetupColumn("Background Time (Hours:Mins:Secs)");

                //only have the tab is selectedstat isn't 1
                if (selectedStat > 1) {
                    ImGui::TableSetupColumn("Most Popular Day");
                }

                ImGui::TableSetupColumn("Original Path");
                ImGui::TableHeadersRow();

                int totalSecs = 0;

                for (int i = 0; i < cachedData->size; i++) {
                    ImGui::TableNextRow();

                    //name icon and selection if >1
                    ImGui::TableSetColumnIndex(0);

                    if (selectedStat > 1) {
                        string rowLabel = "##row_" + to_string(i);
                        if (ImGui::Selectable(rowLabel.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {

                            ImGui::OpenPopup(("GraphPopup_" + to_string(i)).c_str());
                        }


                        if (ImGui::BeginPopup(("GraphPopup_" + to_string(i)).c_str())) {
                            ImGui::Text("Detailed Statistics: %s", (*cachedData).queryProcesses[i].getProcessName().c_str());
                            ImGui::Separator();
                            ImGui::Text("Graph functionality coming soon!"); // We will add the PlotHistogram here tomorrow!
                            ImGui::EndPopup();
                        }
                    }
                    //draw icons
                    ImGui::SameLine(0.0f, 1.0f);
                    if (cachedIcons != nullptr && cachedIcons[i] != nullptr) {
                        ImGui::Image((void*)cachedIcons[i], ImVec2(24.0f, 24.0f));
                    }
                    else {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                        ImGui::Text("[X] N/A");
                        ImGui::PopStyleColor();
                    }

                    
                    ImGui::SameLine(0.0f, 1.0f);
                    ImGui::Text(" %s", (*cachedData).queryProcesses[i].getProcessName().c_str());

                    
                    ImGui::TableSetColumnIndex(1);
                    int aSecs = (*cachedData).queryActiveSecs[i];
                    totalSecs += aSecs; // Add to total
                    ImGui::Text("%d:%02d:%02d", aSecs / 3600, (aSecs % 3600) / 60, aSecs % 60);

                    
                    ImGui::TableSetColumnIndex(2);
                    int bSecs = (*cachedData).queryBackgroundSecs[i];
                    ImGui::Text("%d:%02d:%02d", bSecs / 3600, (bSecs % 3600) / 60, bSecs % 60);

                    //most popular retroactive
                    int nextCol = 3;
                    if (selectedStat > 1) {
                        ImGui::TableSetColumnIndex(nextCol);
                        ImGui::Text("Data Pending...");
                        nextCol++; 
                    }

                    
                    ImGui::TableSetColumnIndex(nextCol);
                    ImGui::Text("%s", (*cachedData).queryProcesses[i].getPathName().c_str());
                }
                ImGui::EndTable();

                ImGui::Dummy(ImVec2(0.0f, 10.0f));


                ImGui::Text("Total Active time: %d:%02d:%02d", totalSecs / 3600, (totalSecs % 3600) / 60, totalSecs % 60);

                if (ImGui::Button("Reload data")) {
                    tracker.saveTime(); //[cite: 5]
                    if (cachedIcons != nullptr && cachedData != nullptr) {
                        for (int i = 0; i < cachedData->size; i++) {
                            if (cachedIcons[i]) cachedIcons[i]->Release();
                        }
                        delete[] cachedIcons;
                        cachedIcons = nullptr;
                    }
                    if (cachedData != nullptr) {
                        delete cachedData;
                        cachedData = nullptr;
                    }
                }
            }
        }
        else {
            ImGui::Text("No data recorded for selected tab yet!");
        }

        //removed delete here cause why would i delete it already deletes when changing tab

    }
}










statData* getEntriesFor(int selectedStat) {
    
    if (selectedStat < 1 || selectedStat > 5) {
        throw out_of_range("serious memory corruption, cause selectedStat has to be bigger than 0 and less than 6");
    }

    statData* res = new statData();

    string folderPath = "logs";

    
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        throw out_of_range("logs folder non existant for getEntriesFor in statShower");
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) { //foreach loop and directory iterator of the path allows us to iterate through a folder's 

       
        if (entry.is_regular_file()) { //if it's a file and not an actual directory

            //extract info 
            std::string fullPath = entry.path().string();            // e.g., "logs/c@#windows#explorer.exe.pttl"
            std::string extension = entry.path().extension().string(); // e.g., ".pttl"

            
            if (extension == ".pttl") {
                statData* fileData = retrieveStatsFromFile(fullPath, selectedStat);
                (*res) += (*fileData); //get returned a one AllProcess array simply with 1 entry and that's it 

                delete fileData; //already copied so delete it 

            }
        }
    }


    return res;
}


statData* retrieveStatsFromFile(const string& filePath, int selectedStat) {
    ifstream currFile(filePath);
    string curr = "";
    statData* res = new statData();
    bool hasProcessBeenAddedTostatData = false;


    if (currFile.is_open()) {
        string savedPID = "0";
        string savedName = "Unknown";
        


        while (getline(currFile, curr) && curr != "- LOGS:") {
            if (curr == "- PID:") {
                getline(currFile, savedPID); 
            }
            else if (curr == "- PROGRAM NAME:") {
                getline(currFile, savedName); 
            }
        }

       
        




        //date reading
        while (getline(currFile, curr)) { //first valid entry
            
            size_t posFirstColon = curr.find(':');
            size_t posSecondColon = curr.find(':', posFirstColon + 1);
            string date = curr.substr(0, posFirstColon);
            
            size_t posFirstSlash = date.find('/');
            size_t posSecondSlash = date.find('/', posFirstSlash + 1);

            if (posFirstColon == string::npos || posSecondColon == string::npos ||
                posFirstSlash == string::npos || posSecondSlash == string::npos) {
                continue;
            }

            int fDay = stoi(date.substr(0, posFirstSlash));
            int fMonth = stoi(date.substr(posFirstSlash + 1, posSecondSlash - posFirstSlash - 1));
            int fYear = stoi(date.substr(posSecondSlash + 1));

            int activeSecs = stoi(curr.substr(posFirstColon + 1, posSecondColon - posFirstColon - 1));
            int backgroundSecs = stoi(curr.substr(posSecondColon + 1));
            bool matchFound = false;

            if (selectedStat == 1) {

                if (fDay == currDay() && fMonth == currMonth() && fYear == currYear()) {

                    matchFound = true;
                }
            }else if (selectedStat == 2) { //week
                    if (isDateInThisWeek(fDay, fMonth, fYear)) {
                        matchFound = true;
                    } 
             } else if (selectedStat == 3) { //month
                    if (isDateInThisMonth(fDay, fMonth, fYear)) {
                        matchFound = true;
                    }
             }else if (selectedStat == 4) { 
                    if (isDateInThisYear(fDay, fMonth, fYear)) {
                        matchFound = true;
                    }
             } else if (selectedStat == 5) { //month
                    
                     matchFound = true;
              }
           






            if (matchFound) {

                int activeSecs = stoi(curr.substr(posFirstColon + 1, posSecondColon - posFirstColon - 1));
                int backgroundSecs = stoi(curr.substr(posSecondColon + 1));
                
                
                if (activeSecs == 0 && backgroundSecs == 0) continue; //to avoid 0:0:0 entries
                
                if (hasProcessBeenAddedTostatData) { //if it has been added it will always be in size - 1
                    


                    (*res).queryActiveSecs[(*res).size - 1] = (*res).queryActiveSecs[(*res).size - 1] + activeSecs;
                    (*res).queryBackgroundSecs[(*res).size - 1] = (*res).queryBackgroundSecs[(*res).size - 1] + backgroundSecs;

                }
                else {
                    DWORD pid = 0;
                    try {
                        pid = stoul(savedPID);
                    }
                    catch (...) {
                    }



                    string ogPath = filePath;

                    // remove logs
                    size_t lastSlash = ogPath.find_last_of("\\/");
                    if (lastSlash != string::npos) {
                        ogPath = ogPath.substr(lastSlash + 1);
                    }

                    //remove the extension
                    size_t extPos = ogPath.rfind(".pttl");
                    if (extPos != string::npos) {
                        ogPath = ogPath.substr(0, extPos);
                    }

                    // remove important obfuscated characters
                    for (char& c : ogPath) {
                        if (c == '#') c = '\\';
                        else if (c == '@') c = ':';
                    }







                    Process loadedProcess(filePath, pid, savedName, ogPath);

                    

                    statData tempAdd;
                    tempAdd.size = 1;
                    tempAdd.queryProcesses = new Process[1]{ loadedProcess };
                    tempAdd.queryActiveSecs = new int[1] {activeSecs};
                    tempAdd.queryBackgroundSecs = new int[1] {backgroundSecs};

                    (*res) += tempAdd;
                    hasProcessBeenAddedTostatData = true;
                }
            }
            


        }




    }


    currFile.close();
    hasProcessBeenAddedTostatData = false;
    return res;
}










/*
* else if (selectedStat == 1) {
        //Show day information 
        //best idea is to go through all the files and the ones who have entries for today show them
        //if user clicks new button when they come back things may have changed so io access is necessary

        //makes sense in the day to get background time entries 

        

       

        if (cachedData == nullptr ) {

            tracker.saveTime(); //save b4 loading
            //Wipe
            if (cachedIcons != nullptr && cachedData != nullptr) {
                for (int i = 0; i < (*cachedData).size; i++) {
                    if (cachedIcons[i]) {
                        (*cachedIcons[i]).Release(); //release the vram
                    }
                }
                delete[] cachedIcons;
                cachedIcons = nullptr;
            }
            if (cachedData != nullptr) { //clean b4 reload
                delete cachedData;
            }



            //reload
            cachedData = getEntriesFor(selectedStat); 

            if (cachedData != nullptr && cachedData->size > 0) {
                (*cachedData).sortActiveTime();

                // load textures once
                cachedIcons = new ID3D11ShaderResourceView* [(*cachedData).size];
                for (int i = 0; i < (*cachedData).size; i++) {
                    const Process* processPointer = nullptr;
                    //if it's running show it from process otherwise from path
                    if (tracker.getProcessFromPath((*cachedData).queryProcesses[i].getPathName(), processPointer)) {
                        // It is running! Use the live tracker to pull the real UWP icon!
                        cachedIcons[i] = (ID3D11ShaderResourceView*)showIconFromProcess(*processPointer, d3dDevice);
                    }
                    else {
                        
                        cachedIcons[i] = showIconFromPath((*cachedData).queryProcesses[i].getPathName(), d3dDevice);
                    }
                    
                }
            }
            
            
        }


        
        if ((*cachedData).size > 0) {

            if (ImGui::BeginTable("DayStatsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {

                
                ImGui::TableSetupColumn("Program Name");
                ImGui::TableSetupColumn("Active Time (Hours:Mins:Secs)");
                ImGui::TableSetupColumn("Background Time (Hours:Mins:Secs)");
                ImGui::TableSetupColumn("Original Path");
                ImGui::TableHeadersRow();

                
                int todayTotalSecs = 0;
                
                for (int i = 0; i < cachedData->size; i++) {
                    ImGui::TableNextRow();


                    
                    ImGui::TableSetColumnIndex(0);
                    if (cachedIcons != nullptr && cachedIcons[i] != nullptr) {
                        ImGui::Image((void*)cachedIcons[i], ImVec2(24.0f, 24.0f));
                    }
                    else {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                        ImGui::Text("[X] ICON N/A   "); // Fallback if no icon exists
                        ImGui::PopStyleColor();
                    }




                    ImGui::SameLine(0.0f, 1.0f);
                    ImGui::Text(" %s", (*cachedData).queryProcesses[i].getProcessName().c_str());

                    ImGui::TableSetColumnIndex(1);
                    
                    int todayHours = (*cachedData).queryActiveSecs[i] / 3600;
                    int todayMins = ((*cachedData).queryActiveSecs[i] % 3600) / 60;
                    int todaySecs = (*cachedData).queryActiveSecs[i] % 60;

                    
                    todayTotalSecs += (*cachedData).queryActiveSecs[i];

                    ImGui::Text("%d:%02d:%02d", todayHours,todayMins, todaySecs);

                    ImGui::TableSetColumnIndex(2);

                    int todayBHours = (*cachedData).queryBackgroundSecs[i] / 3600;
                    int todayBMins = ((*cachedData).queryBackgroundSecs[i] % 3600) / 60;
                    int todayBSecs = (*cachedData).queryBackgroundSecs[i] % 60;

                    ImGui::Text("%d:%02d:%02d", todayBHours, todayBMins, todayBSecs); //the 02 cause it shows the 0 prenumber if <10

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%s", (*cachedData).queryProcesses[i].getPathName().c_str());
                }
                ImGui::EndTable();

                ImGui::Dummy(ImVec2(0.0f, 10.0f));



                ImGui::Text("Total Active time: %d:%02d:%02d", todayTotalSecs / 3600, (todayTotalSecs % 3600) / 60, todayTotalSecs % 60);
                if (ImGui::Button("Reload data")) {
                    //clear everything 
                    tracker.saveTime();
                    if (cachedIcons != nullptr && cachedData != nullptr) {
                        for (int i = 0; i < cachedData->size; i++) {
                            if (cachedIcons[i]) cachedIcons[i]->Release();
                        }
                        delete[] cachedIcons;
                        cachedIcons = nullptr;
                    }
                    if (cachedData != nullptr) {
                        delete cachedData;
                        cachedData = nullptr;
                    }






                }
            }
        }
*/