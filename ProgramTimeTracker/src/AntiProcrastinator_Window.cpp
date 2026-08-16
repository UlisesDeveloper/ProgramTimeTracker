#include "AntiProcrastinator_Window.h"



void AntiProcrastinationWindow(bool& show_antiprocrastination_window, bool& show_add_entry_procrastination_window,  AllProcesses& tracker, ID3D11Device* d3dDevice) {

    static bool init_antiproc = false;
    static bool temp_antiProcrastination = false;


    //antiproc settings file
    
    if (show_antiprocrastination_window) {
        ImGui::SetNextWindowSize(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Anti-Procrastinator", &show_antiprocrastination_window))
        {

            ImGui::BeginDisabled(show_add_entry_procrastination_window); //add 

            if (!init_antiproc) {
                init_antiproc = antiProcrastination;


                init_antiproc = true;
            }
            if (ImGui::Checkbox("Anti Procastination Mode", &temp_antiProcrastination))
            {

                //so if it's enabled we need to create a file if it doesn't exist 
            }
            ImGui::BeginDisabled(!temp_antiProcrastination); //if it's off everything below is disabled


            if (ImGui::Button("New")) {
                show_add_entry_procrastination_window = true;
                //only when entered does it retrieve the processes 
            }


            ImGui::EndDisabled();

            ImGui::Separator();

            if (ImGui::Button("Apply (Restarts)")) {
            }

            ImGui::EndDisabled();

            //need an apply button restarts to save to the settings file the antiprocrastination being enabled
            //and also the file creation if it wasn't for antiprocrastination_settings.txt
        }
        ImGui::End();
    }


    static bool init_entryAntiProc = false;
    static bool isSelected = false;
    static string* listPaths = nullptr;
    static int numPrograms = 0;
    static int selected_index = -1;
    static string programName = "null";
    static string programExe = "null";
    static DWORD programPid = 0;
    static const Process* pathProcessPointer = nullptr;
    static ID3D11ShaderResourceView* currentIcon = nullptr;


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
                if (ImGui::Selectable(listPaths[i].c_str(), is_selected))
                {
                    selected_index = i;


                    if (tracker.getProcessFromPath(listPaths[i].c_str(), pathProcessPointer)) {
                        programName = (*pathProcessPointer).getProcessName();
                        programPid = (*pathProcessPointer).getPid();

                        if (currentIcon != nullptr) { //each time we change the selection if we get the path we can get the currentIcon
                            (*currentIcon).Release();
                            currentIcon = nullptr;
                        }
                        currentIcon = (ID3D11ShaderResourceView*)showIconFromProcess(*pathProcessPointer, d3dDevice);
                    }

                    //reset to defaults before loading so that it resorts to it if nothing is loaded 
                    temp_timeGlobalNotifProgram = 120;
                    temp_timeWeekEndNotifProgram = 180;
                    temp_timeWeekEndNotifProgramToggle = false;
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
            }
            else {
                ImGui::Text("Can't retrieve info");
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
            }
            ImGui::PopStyleColor(3);


            









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


        pathProcessPointer = nullptr;
        if (currentIcon != nullptr) {
            (*currentIcon).Release();
            currentIcon = nullptr;
        }
    }
}



//here load and save antiproc
//Create file when first thing we save
//Format should be number of programs with exceptions
//with its full directory: time: weekend exception bool: and finally exception time
//And then multimedia provider exceptions as well

bool loadAntiProcrastinationDataFromDir(string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle) {
    
    
    
    
    bool res = false;
    string curr = "";
    string pathTemp = "";
    int timeExtracted = -1;
    int posFirstColon = -1;
    int posSecondColon = -1;
    int posThirdColon = -1;
    ifstream file("procrastination.txt");
    
    if (file.is_open()) { //if couldn't open the save will be in charge of creating it
        getline(file, curr);

        int size = 0;
        try {
            size = stoi(curr);
        }
        catch (...) {
            file.close();
            throw invalid_argument("couldn't parse num of items on the antiprocrastination adder");
        }




        for (int i = 0; i < size; i++) {
            getline(file, curr);
            posFirstColon = curr.find(":", 0); //First location
            posSecondColon = curr.find(":", posFirstColon + 1);
            posThirdColon = curr.find(":", posSecondColon + 1);
            if (!(posFirstColon == string::npos ||
                posSecondColon == string::npos ||
                posThirdColon == string::npos
                )) {
                pathTemp = curr.substr(0, posFirstColon);
                if (path == pathTemp) {
                    temp_timeGlobalNotifProgram = stoi(curr.substr(posFirstColon + 1, posSecondColon- posFirstColon- 1));
                    temp_timeWeekEndNotifProgramToggle = stoi(curr.substr(posSecondColon + 1, posThirdColon - posSecondColon -1));
                    temp_timeWeekEndNotifProgram = stoi(curr.substr(posThirdColon + 1));
                    res = true;
                }
            }

        }
    }

    file.close();
    return res;
}



bool saveAntiProcrastinationDataFromDir(string& path, int& temp_timeGlobalNotifProgram, int& temp_timeWeekEndNotifProgram, bool& temp_timeWeekEndNotifProgramToggle) {
    //fist check if path is already there to change the data if not create new entry
    bool res = false;
    int posData = -1;
    //if it's empty or the program wasn't found then append mode otherwise modify the entry
    //problem is the first numSaved at the top which makes append not a posibility so ram rewrite






    posData = posProcrastinationData(path);

    //from this point i can't read anything so everything must be saved to ram atp
    ofstream file("procrastination.txt");
    
    if (file.is_open()) {
        posData = posProcrastinationData(path);
        if (posData >= 0) { //it's an entry

        }
        else if (posData== -1){ //it's not an entry 

        }
        else { //file empty

        }


    }




    return res;
}



//there's something wrong here i don't know what rn
int posProcrastinationData(string& path) { 
    int res = -1;
    ifstream file("procrastination.txt");
    string curr = "";
    string work = "";
    int posFirstColon = -1;

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
            throw invalid_argument("couldn't parse num of items on the antiprocrastination adder, posProcrastinationData");
        }

        for (int i = 0; i < size; i++) {
            getline(file, curr);
            posFirstColon = curr.find(":", 0); //First location
            if (!(posFirstColon == string::npos)) {
                work = curr.substr(0, posFirstColon);
                if (work == path) {
                    res = i;
                }
            }
        }
    }
    file.close();
    return res;

}