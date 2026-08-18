#include "AntiProcrastinator_Window.h"



void AntiProcrastinationWindow(bool& show_antiprocrastination_window, bool& show_modify_entry_procrastination_window, bool& show_add_entry_procrastination_window, AllProcesses& tracker, ID3D11Device* d3dDevice, HANDLE& hMutex, bool& done) {

    static bool init_antiproc = false;
    static bool temp_antiProcrastination = false;
    static bool temp_snooze = false;
    static bool temp_killAfterSnooze = false;
    static int temp_snoozeMins = 300;


    //antiproc settings file
    
    if (show_antiprocrastination_window) {
        ImGui::SetNextWindowSize(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Anti-Procrastinator", &show_antiprocrastination_window))
        {
            //would be horrible if add entry and modify entries are accessible at the same time restrict with no button presses while the child windows are open
            
            bool childWindowOpen = show_add_entry_procrastination_window || show_modify_entry_procrastination_window;

            if (childWindowOpen && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { //if child window mouse on the window and i click cause

                MessageBeep(MB_ICONWARNING); 

                //change focus to the other window
                if (show_add_entry_procrastination_window) {
                    ImGui::SetWindowFocus("Add Entry");
                }
                else if (show_modify_entry_procrastination_window) {
                    ImGui::SetWindowFocus("Modify Entry");
                }
            }

            //Disable any boxes jic the focus change ain't enough
            ImGui::BeginDisabled(childWindowOpen);





            if (!init_antiproc) {
                temp_antiProcrastination = antiProcrastination;
                temp_snooze = snooze;
                temp_snoozeMins = snoozeMins;
                temp_killAfterSnooze = killAfterSnooze;

                init_antiproc = true;

            }

            ImGui::Text("Anti Procrastination mode allows you to set a limit for weekdays / weekends for a program.");
            ImGui::Text("It will send a notification after the limit has been reached for the day");
            ImGui::Text("With the snooze option and it's minutes option you have the option to get reminded every selected minutes again");
            ImGui::Text("You also have the option of killing the program after the first snooze!");

            if (ImGui::Checkbox("Anti Procastination Mode", &temp_antiProcrastination))
            {

                //so if it's enabled we need to create a file if it doesn't exist 

                if (temp_antiProcrastination == false) {
                    temp_snooze = false;
                }


            }

            
            ImGui::BeginDisabled(!temp_antiProcrastination); //if it's off everything below is disabled


            if (ImGui::Checkbox("Snooze", &temp_snooze))
            {

            }

            ImGui::BeginDisabled(!temp_snooze); //add 

            ImGui::InputInt("Snooze for _ minutes", &temp_snoozeMins);


            if (ImGui::Checkbox("Kill Process after first Snooze", &temp_killAfterSnooze))
            {

            }

            ImGui::EndDisabled();


           

            ImGui::EndDisabled();

            ImGui::Separator();

            if (ImGui::Button("Apply (Restarts)")) {
                antiProcrastination = temp_antiProcrastination;

                snooze = temp_snooze;
                snoozeMins = temp_snoozeMins;
                killAfterSnooze = temp_killAfterSnooze;



                SaveSettingsFile(multimediaProviders);




                //Releases the mutex
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
                

                wchar_t exePath[MAX_PATH];
                GetModuleFileNameW(nullptr, exePath, MAX_PATH); //gets the exe path for the program
                ShellExecuteW(nullptr, L"open", exePath, nullptr, nullptr, SW_SHOW); //Runs it, by the time the program has saved and close this other instance will have opened
                done = true;
            }

            ImGui::SameLine(0.0f, 1.0f);

            if (ImGui::Button("Add entries")) {
                show_add_entry_procrastination_window = true;
                //only when entered does it retrieve the processes 
            }

            ImGui::SameLine(0.0f, 1.0f);

            if (ImGui::Button("Modify existing entries")) {
                show_modify_entry_procrastination_window = true;
                //only when entered does it retrieve the processes 
            }

            ImGui::EndDisabled();

            //need an apply button restarts to save to the settings file the antiprocrastination being enabled
            //and also the file creation if it wasn't for antiprocrastination_settings.txt
        }
        ImGui::End();
    }
    else {
        bool init_antiproc = false;
        bool temp_antiProcrastination = false;
        bool temp_snooze = false;
        bool temp_killAfterSnooze = false;
        int temp_snoozeMins = 300;
    }


    static bool init_entryAntiProc = false;
    static bool entryExist = false;


    static string* listPaths = nullptr;
    static int numPrograms = 0;
    static int selected_index = -1;
    static string programName = "null";
    static string programExe = "null";
    static DWORD programPid = 0;
    static const Process* pathProcessPointer = nullptr;
    static ID3D11ShaderResourceView* currentIcon = nullptr;

    static string currSelectionString = "";

    static int temp_timeGlobalNotifProgram = 120;
    static int temp_timeWeekEndNotifProgram = 180;
    static bool temp_timeWeekEndNotifProgramToggle = false;


    if (show_add_entry_procrastination_window)
    {

        ImGui::SetNextWindowSize(ImVec2(300.0f, 400.0f), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Add Entry", &show_add_entry_procrastination_window))
        {

            if (!init_entryAntiProc) {//might need to change the other one so i can have something that actualy resets the variables for sure

                //only when entered does it retrieve the processes 
                numPrograms = 0;
                tracker.getPathNameCurrentProcesses(listPaths, numPrograms);
                init_entryAntiProc = true;
            }

            ImGui::Text("Select program to restrict [ONLY OPENED PROGRAMS MAY APPEAR]:");

            ImGui::BeginChild("ProcessList", ImVec2(0, -250.0f), true);

            //Show selection
            for (int i = 0; i < numPrograms; i++)
            {
                //check if it is the same jic, 
                bool is_selected = (selected_index == i);



                string displayName = listPaths[i];
                if (displayName.empty()) {
                    displayName = "Unknown System Process";
                }

                // Append ##
                // imgui will display the path but use the number as hidden id
                string imguiLabel = displayName + "##" + to_string(i);


                //update to select
                if (ImGui::Selectable(imguiLabel.c_str(), is_selected))
                {
                    
                    selected_index = i;

                    //reset to defaults before loading so that it resorts to it if nothing is loaded 
                    temp_timeGlobalNotifProgram = 120;
                    temp_timeWeekEndNotifProgram = 180;
                    temp_timeWeekEndNotifProgramToggle = false;
                    currSelectionString = "";
                    entryExist = false;

                    if (tracker.getProcessFromPath(listPaths[i].c_str(), pathProcessPointer)) {
                        programName = (*pathProcessPointer).getProcessName();
                        programPid = (*pathProcessPointer).getPid();


                        //since a new selection has been done we should try to retrieve values
                        loadAntiProcrastinationDataFromDir(listPaths[i], temp_timeGlobalNotifProgram, temp_timeWeekEndNotifProgram, temp_timeWeekEndNotifProgramToggle);
                        currSelectionString = listPaths[i];


                        if (currentIcon != nullptr) { //each time we change the selection if we get the path we can get the currentIcon
                            (*currentIcon).Release();
                            currentIcon = nullptr;
                        }
                        currentIcon = (ID3D11ShaderResourceView*)showIconFromProcess(*pathProcessPointer, d3dDevice);
                    }

                    entryExist = entryExists(listPaths[i]);
                    

                    

                }
            }

            ImGui::EndChild();

            if (ImGui::SmallButton("Reload Path List")) {

                numPrograms = 0;
                tracker.getPathNameCurrentProcesses(listPaths, numPrograms);
            }
            






            if (pathProcessPointer != nullptr) { //in that case we don't have info at all 
                ImGui::Text("Program's Internal Name: %s", programName.c_str());
                ImGui::Text("Program's Alias: %s", "N/A");
                ImGui::Text("Program's PID: %lu", programPid);

                if (currentIcon != nullptr) {
                    ImGui::Image((void*)currentIcon, ImVec2(32.0f, 32.0f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    ImGui::Text("X [NO ICON AVAILABLE]");
                    ImGui::PopStyleColor();
                }





                ImGui::InputInt("Global minutes limit for this program", &temp_timeGlobalNotifProgram);



                ImGui::Checkbox("Toggle Weekend exceptions:", &temp_timeWeekEndNotifProgramToggle);

                ImGui::BeginDisabled(!temp_timeWeekEndNotifProgramToggle);

                ImGui::InputInt("Weekend minutes limit for this program", &temp_timeWeekEndNotifProgram);


                ImGui::EndDisabled();


                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.8f, 0.1f, 1.0f)); // red
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.9f, 0.2f, 1.0f)); //hover over
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.7f, 0.0f, 1.0f)); //when pressed

                if (ImGui::Button("Apply Restrictions"))
                {

                    if (currSelectionString.empty()) {
                        MessageBeep(MB_ICONWARNING);
                    }
                    else {
                        hasRestrictionsChanged = true;
                        saveAntiProcrastinationDataFromDir(currSelectionString, temp_timeGlobalNotifProgram, temp_timeWeekEndNotifProgram, temp_timeWeekEndNotifProgramToggle);
                        //load it again if they jic they don't change their position
                        loadAntiProcrastinationDataFromDir(currSelectionString, temp_timeGlobalNotifProgram, temp_timeWeekEndNotifProgram, temp_timeWeekEndNotifProgramToggle);

                    }


                }
                ImGui::PopStyleColor(3);


                if (entryExist) {



                    ImGui::SameLine(0.0f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.0f, 0.0f, 1.0f));

                    if (ImGui::Button("DELETE ENTRY")) {

                        string* ramSave = nullptr;
                        int sizeRamSave = 0;

                        if (!deleteEntry(currSelectionString, ramSave, sizeRamSave)) {
                            //if it fails something catastrophic happened i want an alert or something
                            //Cause if the entry exists something wrong is happening
                            MessageBeep(MB_ICONERROR);
                        }
                        else {
                            hasRestrictionsChanged = true;
                        }
                    }

                    ImGui::PopStyleColor(3);

                }



            }
            else {
                ImGui::Text("Can't retrieve info");
            }







        }
        ImGui::End();
        //When ends i need to delete the dyn array
    }
    else {
        init_entryAntiProc = false; //

        if (listPaths != nullptr) {
            delete[] listPaths;
            listPaths = nullptr;
            numPrograms = 0;
        }

        selected_index = -1;
        entryExist = false;
        currSelectionString = "";


        pathProcessPointer = nullptr;
        if (currentIcon != nullptr) {
            (*currentIcon).Release();
            currentIcon = nullptr;
        }
    }






    static bool mod_init_entryAntiProc = false;
    static bool mod_entryExist = false;


    

    static string* entries = nullptr;
    static int sizeEntriesArray = 0;


    static int mod_selected_index = -1;
    static string mod_programName = "null";
    static string mod_programExe = "null";
    static DWORD mod_programPid = 0;
    static const Process* mod_pathProcessPointer = nullptr;
    static ID3D11ShaderResourceView* mod_currentIcon = nullptr;

    static string mod_currSelectionString = "";

    static int mod_temp_timeGlobalNotifProgram = 120;
    static int mod_temp_timeWeekEndNotifProgram = 180;
    static bool mod_temp_timeWeekEndNotifProgramToggle = false;
    if (show_modify_entry_procrastination_window)
    {

        ImGui::SetNextWindowSize(ImVec2(300.0f, 400.0f), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Modify Entry", &show_modify_entry_procrastination_window))
        {

            if (!mod_init_entryAntiProc) {
                entries = nullptr;
                sizeEntriesArray = 0;
                
                allEntryPathsProcrastinatorFile(entries, sizeEntriesArray);
                mod_init_entryAntiProc = true;
            }

            ImGui::Text("Restricted Programs:");

            ImGui::BeginChild("ProcessList", ImVec2(0, -250.0f), true);

            //Show selection
            for (int i = 0; i < sizeEntriesArray; i++)
            {
                //check if it is the same jic, 
                bool is_selected = (mod_selected_index == i);



                string displayName = entries[i];

                int firstBarLoc = displayName.find('|');
                if (firstBarLoc != string::npos) {
                    displayName = displayName.substr(0, firstBarLoc);

                }



                if (displayName.empty()) {
                    displayName = "bugged entry";
                }

                


                //update to select
                if (ImGui::Selectable(displayName.c_str(), is_selected))
                {

                    mod_selected_index = i;

                    //reset to defaults before loading so that it resorts to it if nothing is loaded 
                    mod_temp_timeGlobalNotifProgram = 120;
                    mod_temp_timeWeekEndNotifProgram = 180;
                    mod_temp_timeWeekEndNotifProgramToggle = false;
                    
                    mod_currSelectionString = displayName;

                    
                    pathArrayPathInfoGetter(entries, sizeEntriesArray, displayName, mod_temp_timeGlobalNotifProgram, mod_temp_timeWeekEndNotifProgram, mod_temp_timeWeekEndNotifProgramToggle);

                    
                    mod_entryExist = entryExistsArray(entries, sizeEntriesArray, displayName);

                    //extract program name
                    size_t lastSlash = displayName.find_last_of("\\/");
                    if (lastSlash != string::npos) {
                        mod_programName = displayName.substr(lastSlash + 1);
                    }
                    else {
                        mod_programName = displayName;
                    }

                    //remove last icon
                    if (mod_currentIcon != nullptr) {
                        (*mod_currentIcon).Release();
                        mod_currentIcon = nullptr;
                    }

                    //if it's running 
                    if (tracker.getProcessFromPath(displayName.c_str(), mod_pathProcessPointer)) {
                        mod_programPid = (*mod_pathProcessPointer).getPid();
                        // If it's a live UWP app, this gets the real live icon!
                        mod_currentIcon = (ID3D11ShaderResourceView*)showIconFromProcess(*mod_pathProcessPointer, d3dDevice);
                    }
                    else { //app ain't open 
                        mod_programPid = 0;
                        mod_pathProcessPointer = nullptr;
                        
                        mod_currentIcon = showIconFromPath(displayName, d3dDevice);
                    }


                    /*
                    if (tracker.getProcessFromPath(displayName.c_str(), mod_pathProcessPointer)) {
                        mod_programName = (*mod_pathProcessPointer).getProcessName();
                        mod_programPid = (*mod_pathProcessPointer).getPid();


                        //since a new selection has been done we should try to retrieve values
                        loadAntiProcrastinationDataFromDir(displayName, mod_temp_timeGlobalNotifProgram, mod_temp_timeWeekEndNotifProgram, mod_temp_timeWeekEndNotifProgramToggle);
                        mod_currSelectionString = displayName;


                        if (mod_currentIcon != nullptr) { //each time we change the selection if we get the path we can get the mod_currentIcon
                            (*mod_currentIcon).Release(); //releases previous selection icon
                            mod_currentIcon = nullptr;
                        }
                        mod_currentIcon = (ID3D11ShaderResourceView*)showIconFromProcess(*mod_pathProcessPointer, d3dDevice);
                    }

                    mod_entryExist = entryExists(displayName);
                    */



                }
            }

            ImGui::EndChild();







            if (!mod_currSelectionString.empty()) { //in that case we don't have info at all 
                ImGui::Text("Program's Internal Name: %s", mod_programName.c_str());
                ImGui::Text("Program's Alias: %s", "N/A");

                if (mod_programPid > 0) {
                    ImGui::Text("Program's PID: %lu", mod_programPid);
                }
                else {
                    ImGui::Text("Program's PID: N/A CLOSED");
                }

                if (mod_currentIcon != nullptr) {
                    ImGui::Image((void*)mod_currentIcon, ImVec2(32.0f, 32.0f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    ImGui::Text("X [NO ICON AVAILABLE]");
                    ImGui::PopStyleColor();
                }

                ImGui::InputInt("Global minutes limit for this program", &mod_temp_timeGlobalNotifProgram);



                ImGui::Checkbox("Toggle Weekend exceptions:", &mod_temp_timeWeekEndNotifProgramToggle);

                ImGui::BeginDisabled(!mod_temp_timeWeekEndNotifProgramToggle);

                ImGui::InputInt("Weekend minutes limit for this program", &mod_temp_timeWeekEndNotifProgram);


                ImGui::EndDisabled();


                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.8f, 0.1f, 1.0f)); // red
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.9f, 0.2f, 1.0f)); //hover over
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.7f, 0.0f, 1.0f)); //when pressed

                if (ImGui::Button("Apply Restrictions"))
                {

                    if (mod_currSelectionString.empty()) {
                        MessageBeep(MB_ICONWARNING);
                    }
                    else {
                        hasRestrictionsChanged = true;
                        saveAntiProcrastinationDataFromDir(mod_currSelectionString, mod_temp_timeGlobalNotifProgram, mod_temp_timeWeekEndNotifProgram, mod_temp_timeWeekEndNotifProgramToggle);
                        //load it again if they jic they don't change their position
                        loadAntiProcrastinationDataFromDir(mod_currSelectionString, mod_temp_timeGlobalNotifProgram, mod_temp_timeWeekEndNotifProgram, mod_temp_timeWeekEndNotifProgramToggle);

                    }


                }
                ImGui::PopStyleColor(3);


                if (mod_entryExist) {



                    ImGui::SameLine(0.0f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.0f, 0.0f, 1.0f));

                    if (ImGui::Button("DELETE ENTRY")) {

                        string* ramSave = nullptr;
                        int sizeRamSave = 0;

                        if (!deleteEntry(mod_currSelectionString, ramSave, sizeRamSave)) {
                            //if it fails something catastrophic happened i want an alert or something
                            //Cause if the entry exists something wrong is happening
                            MessageBeep(MB_ICONERROR);
                        }
                        else {
                            hasRestrictionsChanged = true;


                            //if it deletes the entry we need to delete it from the list, refresh it really
                            mod_init_entryAntiProc = false; //causes the next frame to reget the list
                            //Reset selection variables as well
                            mod_selected_index = -1;
                            mod_entryExist = false;
                            mod_currSelectionString = "";
                            mod_pathProcessPointer = nullptr;


                            if (mod_currentIcon != nullptr) { //release the loaded icon from the process we deleted
                                (*mod_currentIcon).Release(); 
                                mod_currentIcon = nullptr;
                            }

                        }
                    }

                    ImGui::PopStyleColor(3);

                }





            }
            else if (mod_selected_index != -1) {
                ImGui::Text("Can't retrieve info");
            }






           




        }
        ImGui::End();
        //When ends i need to delete the dyn array
    }
    else {
        mod_init_entryAntiProc = false; //

        

        mod_selected_index = -1;
        mod_entryExist = false;
        mod_currSelectionString = "";


        mod_pathProcessPointer = nullptr;
        if (mod_currentIcon != nullptr) {
            (*mod_currentIcon).Release();
            mod_currentIcon = nullptr;
        }
    }



}

















//here load and save antiproc
//Create file when first thing we save
//Format should be number of programs with exceptions
//with its full directory: time: weekend exception bool: and finally exception time
//And then multimedia provider exceptions as well

bool loadAntiProcrastinationDataFromDir(const string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle) {
    
    
    
    
    bool res = false;
    bool exceptionOcurred = false;
    string curr = "";
    string pathTemp = "";
    int timeExtracted = -1;
    int posFirstBar = -1;
    int posSecondBar = -1;
    int posThirdBar = -1;
    ifstream file("procrastination.txt");
    
    if (file.is_open()) { //if couldn't open the save will be in charge of creating it
        getline(file, curr);

        int size = 0;
        try {
            size = stoi(curr);
        }
        catch (...) {
            file.close();
            //throw invalid_argument("couldn't parse num of items on the antiprocrastination adder");
            exceptionOcurred = true;
        }



        if (!exceptionOcurred) {
            for (int i = 0; i < size && !res; i++) {


                //if user deletes entry without changing a centinel value
                if (!getline(file, curr)) {
                    size = i;
                    break;
                }


                posFirstBar = curr.find("|", 0); //First location
                posSecondBar = curr.find("|", posFirstBar + 1);
                posThirdBar = curr.find("|", posSecondBar + 1);
                if (!(posFirstBar == string::npos ||
                    posSecondBar == string::npos ||
                    posThirdBar == string::npos
                    )) {
                    pathTemp = curr.substr(0, posFirstBar);
                    if (path == pathTemp) {

                        try {
                            temp_timeGlobalNotifProgram = stoi(curr.substr(posFirstBar + 1, posSecondBar - posFirstBar - 1));
                            temp_timeWeekEndNotifProgramToggle = stoi(curr.substr(posSecondBar + 1, posThirdBar - posSecondBar - 1));
                            temp_timeWeekEndNotifProgram = stoi(curr.substr(posThirdBar + 1));
                            res = true;
                        }
                        catch (...) {
                            //user didn't write correct entry somewhere
                            res = false;
                        }
                    }
                }

            }
        }
    }

    file.close();
    return res;
}



bool saveAntiProcrastinationDataFromDir(const string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle) {
    //fist check if path is already there to change the data if not create new entry
    bool res = false;
    int posData = -1;
    //if it's empty or the program wasn't found then append mode otherwise modify the entry
    //problem is the first numSaved at the top which makes append not a posibility so ram rewrite


    string* ramSave = nullptr;
    int sizeRamSave = 0;
    string curr = "";

    
    posData = posProcrastinationDataAndRamWrite(path, ramSave, sizeRamSave);
    //atp we know if we found the entry and we have the entire procrastination.txt in the ramSave

        
   
    string resString = path + '|' + to_string(temp_timeGlobalNotifProgram) +'|' + to_string(temp_timeWeekEndNotifProgramToggle) + '|' + to_string(temp_timeWeekEndNotifProgram);
    //now leave everything as it was in the file except the hijack
    



    

    //from this point i can't read anything so everything must be saved to ram atp
    ofstream file("procrastination.txt");
    
    if (file.is_open()) {
        
        
        if (posData >= 0 && posData < sizeRamSave) { //Good pos so we hijack
            file << sizeRamSave << endl;
            for (int i = 0; i < sizeRamSave; i++) {
                if (i == posData) {
                    file << resString << endl;
                }
                else {
                    file << ramSave[i] << endl;
                }
            }
            res = true;
        }
        else if (posData == -1 || posData == -2 || posData == -3){
            sizeRamSave++;
            file << sizeRamSave << endl;
            for (int i = 0; i < sizeRamSave; i++) {
                if (i == (sizeRamSave - 1)) {
                    file << resString << endl;
                }
                else {
                    file << ramSave[i] << endl;
                }
            }
            res = true;
        }



    }



    delete[] ramSave; //Finished using it already and it's saved so it's useless atp
    return res;
}



//there's something wrong here i don't know what rn
int posProcrastinationDataAndRamWrite(const string& path, string*& ramSave, int& sizeRamSave) {
    bool exceptionOcurred = false;
    int res = -1;
    ifstream file("procrastination.txt");
    string curr = "";
    string work = "";
    int posFirstBar = -1;

    if (file.is_open() && file.peek() == std::ifstream::traits_type::eof()) { //peek checks for next char without actually movint into that position
        res = -2;
    } else if (file.is_open()) { //if couldn't open the save will be in charge of creating it
        getline(file, curr);

        int size = 0;
        try {
            size = stoi(curr);
        }
        catch (...) {
            file.close();
            //throw invalid_argument("couldn't parse num of items on the antiprocrastination adder, posProcrastinationData");
            exceptionOcurred = true;
        }

        if (!exceptionOcurred) {
            if (sizeRamSave != size) { //we're gonna overwrite everything so it doesn't need to be saved
                delete[] ramSave;
                sizeRamSave = size;
                ramSave = new string[sizeRamSave];

            }



            for (int i = 0; i < sizeRamSave; i++) {
                //if user deletes entry without changing a centinel value
                if (!getline(file, curr)) {
                    sizeRamSave = i;
                    break;
                }
                ramSave[i] = curr;

                posFirstBar = curr.find("|", 0); //First location
                if (!(posFirstBar == string::npos)) {
                    work = curr.substr(0, posFirstBar);
                    if (work == path) {
                        res = i;
                    }
                }
            }
        }
    }
    else {
        res = -3;
    }
    file.close();
    return res;

}



bool entryExists(const string& path) { //the button will only be activated if entry does exists so deleteEntry doesn't need a check
    bool res = false;
    bool exceptionOcurred = false;
    ifstream file("procrastination.txt");
    int posFirstBar = -1;
    string curr = "";
    int size = -1;
    if(file.is_open()) {
        getline(file, curr);
        
        try {
            size = stoi(curr);
        }
        catch (...) {
            file.close();
            exceptionOcurred = true;
        }

        if (!exceptionOcurred) {
            for (int i = 0; i < size; i++) {
                if (!getline(file, curr)) {
                    break; //stop searching atp 
                }

                posFirstBar = curr.find('|');
                if (posFirstBar != string::npos) {
                    curr = curr.substr(0, posFirstBar);
                    if (curr == path) {
                        res = true;
                        break; //no time to waste
                        
                    }
                }

            }
        }

    }

    file.close();
    return res;
}


bool deleteEntry(const string& path, string*& ramSave, int& sizeRamSave) {
    //if it has to be deleted we are going to have to find it so more time wasted, prob more efficient to just recieve line to delete and decrease by 1 the top value
    //ramsave needed
    bool res = false; //true if it succeeds
    bool exceptionOcurred = false;;
    ifstream file("procrastination.txt");
    string curr = "";
    int size = -1;
    int posFirstBar = -1;
    string pathEntry = "";



    if (file.is_open()) {
        getline(file, curr);
        try {
            size = stoi(curr);
        }
        catch (...) {
            file.close();
            exceptionOcurred = true;
        }

        if (!(exceptionOcurred) && size > 0) { //if not we have nothing to read
            
            ramSave = new string[size - 1]; //if size is 0 we have a problem cause we allocate -1
            

            int wIndex = 0;
            for (int i = 0; i < size; i++) {
                if (getline(file, curr)) {
                    
                    posFirstBar = curr.find('|');
                    if (posFirstBar != string::npos) {
                        pathEntry = curr.substr(0, posFirstBar);

                        if (pathEntry == path) {
                            //doesn't get added
                            res = true; //got deleted
                        }
                        else {
                            if (wIndex < size - 1) {
                                ramSave[wIndex] = curr;
                                wIndex++;
                                //Size -1 is what is to be written
                            }
                            
                        }

                    }
                    
                }
                
            }
        }






        


    }
    file.close();

    if (res && ramSave != nullptr) { //only need to write things to the file if we have deleted it res, and if it's nullptr then we directly have nothing to save
        ofstream fileSave("procrastination.txt");
        if (fileSave.is_open()) {
            fileSave << size - 1 << endl;
            for (int i = 0; i < size - 1; i++) {
                fileSave << ramSave[i] << endl;
            }
        }
        fileSave.close();
    }

    delete[] ramSave;
    
    return res;

}


void allEntryPathsProcrastinatorFile(string*& entries, int& sizeEntriesArray) {
    bool exceptionOcurred = false;
    string curr = "";
    int firstBarLoc = -1;
    
    ifstream file("procrastination.txt");

    if (file.is_open()) { //if couldn't open the save will be in charge of creating it
        getline(file, curr);

        int size = 0;
        try {
            size = stoi(curr);
        }
        catch (...) {
            file.close();
            //throw invalid_argument("couldn't parse num of items on the antiprocrastination adder");
            exceptionOcurred = true;
        }

        //i already do know the number of entries it's in the top number no need to resize 
        

        if (!exceptionOcurred && size > 0) {
            //i already do know the number of entries it's in the top number no need to resize
            if (entries != nullptr) {
                delete[] entries;
            }
            entries = new string[size];
            int actualCount = 0;
            for (int i = 0; i < size; i++) {


                //if user deletes entry without changing a centinel value
                if (!getline(file, curr)) {
                    size = i; //Always is one down so makes sense than the actual num of items
                    break;
                }
                else {
                    
                        entries[actualCount] = curr; 
                        actualCount++;
                    
                }

                
                

            }
            sizeEntriesArray = actualCount;
        }
    }

    file.close();
}


bool entryExistsArray(const string* pathArray, const int& sizeEntriesArray, const string& path) {
    bool res = false;
    bool exceptionOcurred = false;
    int posFirstBar = -1;
    string curr = "";

    for (int i = 0; i < sizeEntriesArray && !res; i++) {
        curr = pathArray[i];

        posFirstBar = curr.find('|');
        if (posFirstBar != string::npos) {
            curr = curr.substr(0, posFirstBar);
            if (curr == path) {
                res = true;

            }
        }

    }
    
        
    return res;
}



bool pathArrayPathInfoGetter(const string* pathArray, const int& sizeEntriesArray, const string& path, int& currentActiveProcessGlobalLimit, int& currentActiveProcessWeekEndLimit, bool& currentActiveProcessWeekEndLimitStatus) {
    bool res = false;

    
    string curr = "";
    string pathTemp = "";
    int timeExtracted = -1;
    int posFirstBar = -1;
    int posSecondBar = -1;
    int posThirdBar = -1;
    
    for (int i = 0; i < sizeEntriesArray && !res; i++) {


        curr = pathArray[i];


        posFirstBar = curr.find("|", 0); //First location
        posSecondBar = curr.find("|", posFirstBar + 1);
        posThirdBar = curr.find("|", posSecondBar + 1);
        if (!(posFirstBar == string::npos ||
            posSecondBar == string::npos ||
            posThirdBar == string::npos
            )) {
            pathTemp = curr.substr(0, posFirstBar);
            if (path == pathTemp) {

                try {
                    currentActiveProcessGlobalLimit = stoi(curr.substr(posFirstBar + 1, posSecondBar - posFirstBar - 1));
                    currentActiveProcessWeekEndLimitStatus = stoi(curr.substr(posSecondBar + 1, posThirdBar - posSecondBar - 1));
                    currentActiveProcessWeekEndLimit = stoi(curr.substr(posThirdBar + 1));
                    res = true;
                }
                catch (...) {
                    //user didn't write correct entry somewhere
                    res = false;
                }
            }
        }

    }

    return res;
}



