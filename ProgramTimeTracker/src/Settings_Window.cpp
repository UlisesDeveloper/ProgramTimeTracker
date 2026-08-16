#include "Settings_Window.h"


void SettingsWindow(bool& show_settings_window, bool& show_alias_window, AllProcesses& tracker, HANDLE& hMutex, bool& done) {


    //We need them static so that they don't change each frame
    static bool init_settings = false;
    static int temp_timeBefore = 300;
    static int temp_secsBeforeVideo = 600;
    static bool temp_videoMode = false;
    static char* temp_providers_buffer = nullptr;
    static int current_buffer_size = 0;
    if (show_settings_window)
    {
        ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Settings", &show_settings_window))
        {

            if (ImGui::Button("Set Aliases")) {
                show_alias_window = true;
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.0f, 0.0f, 1.0f));


            ImGui::SameLine(0.0f, 1.0f);
            if (ImGui::Button("DELETE ALL TRACKING DATA")) {
                ImGui::OpenPopup("Confirm Deletion");
            }

            ImGui::PopStyleColor(3);

            //opens confirmation prompt
            if (ImGui::BeginPopupModal("Confirm Deletion", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure you want to delete/reset all tracking data and exit?");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                if (ImGui::Button("Yes, Exit", ImVec2(120, 0))) {
                    tracker.resetDayTime();
                }

                ImGui::SetItemDefaultFocus(); // Makes pressing 'Enter' on the keyboard default to 'Yes'
                ImGui::SameLine();

                // 4. The "Cancel" Button just closes the modal and does nothing else
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }


            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            if (ImGui::Checkbox("Launch at Windows Startup", &g_RunStartup))
            {
                RunAtStartup(g_RunStartup);
            }
            ImGui::Text("Recommended to keep on for accurate tracking since startup, doesn't need apply to save");



            ImGui::Separator();
            ImGui::Text("Program Settings, Apply to save them!");
            ImGui::Text("- TIMEOUT SETTINGS:");
            if (!init_settings) { //we do this to load the globals if anything important has changed
                temp_timeBefore = timeBeforeTimeOut;
                temp_secsBeforeVideo = secsBeforeVideoTimeOut;
                temp_videoMode = videoModeEnabled;
                //load the multimedia providers as well


                //minimum characters needed to show the providers
                int charCount = 0;
                for (int i = 0; i < numOfMultimediaProviders; i++) {

                    charCount += multimediaProviders[i].length() + 1; //+1 for the /n 
                }

                if (charCount <= 21666) {
                    charCount += 65536;

                }
                else {
                    charCount *= 4; //minimum 4 times the ones already on the provider list
                }
                //jic delete a previous buffer if there was any
                delete[] temp_providers_buffer;


                current_buffer_size = charCount;
                temp_providers_buffer = new char[charCount];

                temp_providers_buffer[0] = '\0';
                for (int i = 0; i < numOfMultimediaProviders; i++) {
                    strcat_s(temp_providers_buffer, current_buffer_size, multimediaProviders[i].c_str());
                    strcat_s(temp_providers_buffer, current_buffer_size, "\n"); //Adds each provider + the /c
                }
                init_settings = true;
            }
            ImGui::InputInt("Global Timeout (secs)", &temp_timeBefore);
            ImGui::Checkbox("Enable Video Mode", &temp_videoMode);

            ImGui::BeginDisabled(!temp_videoMode); //disabled if it's off

            ImGui::InputInt("Video Timeout (secs)", &temp_secsBeforeVideo);

            ImGui::Text("Multimedia Providers (Enter for new entry):");
            ImGui::InputTextMultiline("##Providers", temp_providers_buffer, current_buffer_size, ImVec2(-FLT_MIN, 150.0f));


            ImGui::EndDisabled();

            
            
            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            ImGui::Text("- MISC SETTINGS:");
            ImGui::Checkbox("Debug Mode", &g_DebugMode);
            ImGui::Text("Used for diagnostics, Don't enable it if you don't know what it means");

            ImGui::Dummy(ImVec2(0.0f, 10.0f));

            if (ImGui::Button("Apply (Restarts)"))
            {
                //save temp to globals
                timeBeforeTimeOut = temp_timeBefore;
                secsBeforeVideoTimeOut = temp_secsBeforeVideo;
                videoModeEnabled = temp_videoMode;

                string curr = "";
                int count = 0;
                int currString = 0;
                //istringstream makes us able to treat the char array as if it were a file stream so we can getline it automatically gives us lines separated by the \n and obbv the stopper \0
                istringstream stream(temp_providers_buffer);
                while (getline(stream, curr)) { //Returns false when no more lines to read
                    count++;
                }

                delete[] multimediaProviders;
                multimediaProviders = new string[count];

                //Resets the stream to the beginning b4 \0
                stream.clear();
                stream.seekg(0);

                while (getline(stream, curr)) { //Returns false when no more lines to read

                    if (!curr.empty() && curr.back() == '\r') curr.pop_back(); //Carriage returns blocker 

                    multimediaProviders[currString] = curr;
                    currString++;
                }




                numOfMultimediaProviders = count;
                SaveSettingsFile(multimediaProviders);
                //Releases the mutex
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);


                wchar_t exePath[MAX_PATH];
                GetModuleFileNameW(nullptr, exePath, MAX_PATH); //gets the exe path for the program
                ShellExecuteW(nullptr, L"open", exePath, nullptr, nullptr, SW_SHOW); //Runs it, by the time the program has saved and close this other instance will have opened

                done = true;
            }


        }
        ImGui::End();






        if (show_alias_window) {
            ImGui::SetNextWindowSize(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Aliases", &show_alias_window))
            {
            }

            ImGui::End();
        }
    }
    else
    {
        // reset flag if user pressed x instead of apply
        init_settings = false;
    }

}