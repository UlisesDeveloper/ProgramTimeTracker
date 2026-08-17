#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include "AllProcesses.h"
#include "globals.h"
#include "Process.h"
#include "auxiliaryMainFunctions.h"
#include <filesystem>
#include <shellapi.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <shlobj.h>
#include <unordered_map>


#include "AntiProcrastinator_Window.h"
#include "Settings_Window.h"


//custom Windows message ID for tray interactions
#define WM_APP_TRAYMSG (WM_APP + 1)
#define ID_TRAY_ICON 1001
#define ID_TRAY_EXIT 3001

static NOTIFYICONDATAW s_NID = {};



using namespace std;


AllProcesses tracker;


// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


void exceedLimitNotification(const Process& a, bool killed = false);


// Main code
int main(int argc, char** argv)
{
    try {
        //Assign usermodelid cause window hates portable exe sending notifications
        SetCurrentProcessExplicitAppUserModelID(L"UlisesDev.ProgramTimeTracker");

        HANDLE hMutex = CreateMutexW(NULL, TRUE, L"ProgramTimeTracker_MUTEX");
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            //if it enters another instance is already running of the program
            HWND hExisting = FindWindowW(L"ProgramTimeTrackerClass", nullptr); //Finds the other instance
            if (hExisting)
            {
                //make it as if we pressed the traybar icon 
                PostMessageW(hExisting, WM_APP_TRAYMSG, 0, WM_LBUTTONUP);
                SetForegroundWindow(hExisting);
            }


            return 0; //kills this instance that tried to appear
        }




        // Make process DPI aware and obtain main monitor scale
        ImGui_ImplWin32_EnableDpiAwareness();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

        HINSTANCE hInstance = GetModuleHandle(nullptr);

        HICON customIcon = (HICON)LoadImageW(
            nullptr,
            L"misc\\ProgramTimeTracker.ico",
            IMAGE_ICON,
            0, 0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE
        );


        // Create application window
        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, hInstance, customIcon, nullptr, nullptr, nullptr, L"ProgramTimeTrackerClass", customIcon };
        ::RegisterClassExW(&wc);
        HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Program Time Tracker", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

        // Initialize Direct3D
        if (!CreateDeviceD3D(hwnd))
        {
            CleanupDeviceD3D();
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        // Show the window


        //We hijack the show the window to not show it if there's a hidden flag
        bool startHidden = false;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--hidden") == 0) { //if they are the same string comparator
                startHidden = true;
            }
        }
        if (startHidden) {
            ::ShowWindow(hwnd, SW_HIDE);
        }
        else {
            ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        }
        ::UpdateWindow(hwnd);


        //Set system tray
        ZeroMemory(&s_NID, sizeof(s_NID));
        s_NID.cbSize = sizeof(s_NID);
        s_NID.hWnd = hwnd;
        s_NID.uID = ID_TRAY_ICON;
        s_NID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        s_NID.uCallbackMessage = WM_APP_TRAYMSG;
        s_NID.hIcon = customIcon; 
        wcscpy_s(s_NID.szTip, L"Program Time Tracker");
        Shell_NotifyIconW(NIM_ADD, &s_NID);


        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        // Load Fonts
        // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
        //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
        // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //style.FontSizeBase = 20.0f;
        //io.Fonts->AddFontDefaultVector();
        //io.Fonts->AddFontDefaultBitmap();
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
        //IM_ASSERT(font != nullptr);

        // Our state
        bool show_demo_window = true;
        bool show_credits_window = false;
        bool show_settings_window = false;
        bool show_alias_window = false;
        bool show_antiprocrastination_window = false;
        bool show_add_entry_procrastination_window = false;
        bool show_modify_entry_procrastination_window = false;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);




        //filesystem::create_directory("logs");
        //Error prone
        LoadSettingsFile();
        bool firstStartup = false;
        std::error_code ec;
        if (!std::filesystem::exists("logs"))
        {
            firstStartup = true;
            std::filesystem::create_directories("logs", ec);
            if (ec)
            {
                throw invalid_argument("Warning: Could not create logs directory, close the program and create it yourself");
            }
        }




        //AllProcesses tracker;
        //moved it to the top
        tracker.getOpenedProcesses();
        //cout << "starting";
        bool stillUAC = false;
        int startDay = currDay();
        int timeSinceAutoSave = 0;
        uint64_t lastTrackerTick = GetTickCount64(); //Stopwatch


        if (firstStartup) {

            g_RunStartup = true;
            RunAtStartup(true);
            firstStartup = false;
        }
        else {
            g_RunStartup = CheckIfRunsAtStartup();
        }


        string* entries = nullptr;
        int sizeEntriesArray = 0;

        allEntryPathsProcrastinatorFile(entries, sizeEntriesArray);
        int currentActiveProcessGlobalLimit = -1;
        bool currentActiveProcessWeekEndLimitStatus = false;
        int currentActiveProcessWeekEndLimit = -1;



        // Main loop
        bool done = false;
        while (!done)
        {
            // Poll and handle messages (inputs, window resize, etc.)
            // See the WndProc() function below for our to dispatch events to the Win32 backend.
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    done = true;
            }
            if (done)
                break;

            // Handle window being minimized or screen locked
            if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
            {
                ::Sleep(10);
                continue;
            }
            g_SwapChainOccluded = false;

            // Handle window resize (we don't resize directly in the WM_SIZE handler)
            if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
            {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
                g_ResizeWidth = g_ResizeHeight = 0;
                CreateRenderTarget();
            }





            //Code here
            if (hasRestrictionsChanged) {
                //acknowledge
                allEntryPathsProcrastinatorFile(entries, sizeEntriesArray);
                hasRestrictionsChanged = false;
            }

            if (GetTickCount64() - lastTrackerTick >= 1000) { //This is the "Sleep" for a second thing
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
                        //add 1 to background time to all background processes
                        tracker.addTimeActiveProcess(1);
                        tracker.addTimeBackgroundProcesses(1, false);



                        //here is where i have to do my checks for antiprocrastination
                        if (antiProcrastination) {
                            

                            //loadAntiProcrastinationDataFromDir(tracker.getFocusedProcess().getPathName(), currentActiveProcessGlobalLimit, currentActiveProcessWeekEndLimit, currentActiveProcessWeekEndLimitStatus);
                            //need to use the array i created too much of an io penalty
                            if (pathArrayPathInfoGetter(entries, sizeEntriesArray, tracker.getFocusedProcess().getPathName(), currentActiveProcessGlobalLimit, currentActiveProcessWeekEndLimit, currentActiveProcessWeekEndLimitStatus)) {

                                uint64_t currentSecondsActive = tracker.getFocusedProcess().getTodayTime();
                                uint64_t limitToEnforceInSecs = 0;


                                static string snoozedPath = ""; 

                                if (entryExistsArray(entries, sizeEntriesArray, tracker.getFocusedProcess().getPathName())) {
                                    if (isWeekEnd() && currentActiveProcessWeekEndLimitStatus) {
                                        limitToEnforceInSecs = currentActiveProcessWeekEndLimit * 60; //transform to secs
                                    }
                                    else {
                                        limitToEnforceInSecs = currentActiveProcessGlobalLimit * 60; //transform to secs
                                    }


                                    if (currentSecondsActive >= limitToEnforceInSecs ) {


                                        string currentPath = tracker.getFocusedProcess().getPathName();

                                        //dictionary seems good
                                        static unordered_map<string, uint64_t> appSnoozeExpirations;

                                        //check if app is in the map
                                        if (appSnoozeExpirations.find(currentPath) == appSnoozeExpirations.end() || currentSecondsActive >= appSnoozeExpirations[currentPath]) {
                                            //only possible if second condition is true, != not end so it found something 
                                            bool isSnoozeRunningOut = (appSnoozeExpirations.find(currentPath) != appSnoozeExpirations.end());

                                            if (isSnoozeRunningOut && killAfterSnooze) {
                                                //snooze runned out 
                                                
                                                tracker.saveTime(); //save time so the active time can get saved mostly for low minutes

                                                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, tracker.getFocusedProcess().getPid());
                                                if (hProcess != NULL) {
                                                    exceedLimitNotification(tracker.getFocusedProcess(), true);
                                                    TerminateProcess(hProcess, 0);
                                                    CloseHandle(hProcess);
                                                }

                                                //timer up so if they reopen it gets rekilled
                                                appSnoozeExpirations[currentPath] = 1;

                                            }
                                            else {

                                                exceedLimitNotification(tracker.getFocusedProcess(), false);

                                                if (snooze) {
                                                    appSnoozeExpirations[currentPath] = currentSecondsActive + (snoozeMins * 60);

                                                    
                                                }
                                                else {

                                                    appSnoozeExpirations[currentPath] = currentSecondsActive + 999999;
                                                }
                                            }
                                        }
                                    }

                                }
                            }
                        }



                    }
                    else {

                        tracker.addTimeBackgroundProcesses(1, true); //basically also adds background time to "active" process after the timeout
                    }

                }


                timeSinceAutoSave++;
                if (timeSinceAutoSave >= 300) {
                    tracker.saveTime();
                    timeSinceAutoSave = 0;
                }

                lastTrackerTick = GetTickCount64(); // Reset stopwatch
            }


            // Start the Dear ImGui frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();


            //fill os window
            ImGuiIO& io = ImGui::GetIO();

            //Force the ImGui window to start at top-left (0,0) and match the full screen size
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(io.DisplaySize);

            // 2. Remove internal padding so widgets touch the actual window borders
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

            // 3. Strip away the fake title bar, borders, and resize handles
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus;


            //Window customization
            ImGui::Begin("Program Time Tracker v.1.0.0", nullptr, flags);

            ImGui::Text("Program Version: 1.0.0  ");


            //ADD GIANT ICON LATER




            ImGui::SameLine(0.0f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f)); // red
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); //hover over
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.0f, 0.0f, 1.0f)); //when pressed
            if (ImGui::SmallButton("Kill")) {
                tracker.saveTime();
                done = true; //Stops the continuous loop
            }
            ImGui::PopStyleColor(3);

            if (g_DebugMode) {
                if (ImGui::SmallButton("excepetion test")) {
                    throw out_of_range("marinero de los mares con las olas de cartulina");
                }
                ImGui::SameLine(0.0f, 1.0f);
                if (ImGui::SmallButton("graceful exit")) {
                    exit(0);
                }

            }

            ImGui::Text("Tracking Engine: RUNNING");
            ImGui::Text("Seconds until next Auto-Save: %d", timeBeforeTimeOut - timeSinceAutoSave);
            if (ImGui::Button("Save Now")) {
                tracker.saveTime();
            }

            ImGui::Separator();
            if (ImGui::Button("Current")) {
            }
            ImGui::SameLine(0.0f, 1.0f);
            if (ImGui::Button("Day")) {
            }
            ImGui::SameLine(0.0f, 1.0f);
            if (ImGui::Button("Week")) {
            }
            ImGui::SameLine(0.0f, 1.0f);
            if (ImGui::Button("Month")) {
            }
            ImGui::SameLine(0.0f, 1.0f);
            if (ImGui::Button("Year")) {
            }

            ImGui::Separator();

            Process activeApp = tracker.getFocusedProcess();
            ImGui::Text("Currently Focused PID: %lu", activeApp.getPid());
            ImGui::Text("Currently Focused Name: %s", activeApp.getProcessName().c_str());
            ImGui::Text("Currently Focused Path: %s", activeApp.getLogFileName().c_str());
            ImGui::Text("Currently Focused Path: %s", activeApp.getPathName().c_str());
            ImGui::Text("Active Time Today: %llu seconds", activeApp.getTodayTime());
            ImGui::Text("Background Time Today: %llu seconds", activeApp.getBackgroundTodayTime());

            ImGui::Separator();

            if (ImGui::Button("Anti-Procrastinator")) {
                show_antiprocrastination_window = true;
            }
            ImGui::SameLine(0.0f, 1.0f);
            

            if (ImGui::Button("Settings")) {
                show_settings_window = true;
            }
            ImGui::SameLine(0.0f, 1.0f);
            

            if (ImGui::Button("Credits")) {
                show_credits_window = true;
            }



            ImGui::PopStyleVar();
            ImGui::End();

            AntiProcrastinationWindow(show_antiprocrastination_window, show_add_entry_procrastination_window, show_modify_entry_procrastination_window, tracker, g_pd3dDevice, hMutex, done);

            SettingsWindow(show_settings_window, show_alias_window, tracker, hMutex, done);


            //credits window
            if (show_credits_window)
            {
                // size for next window
                ImGui::SetNextWindowSize(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);

                // Begin the window, passing the boolean so the "X" button on it actually closes it
                if (ImGui::Begin("Credits", &show_credits_window))
                {
                    ImGui::Text("Program Time Tracker v1.0.0");
                    ImGui::Separator();
                    ImGui::Text("Created by: Ulises Romero López ");
                    ImGui::SameLine(0.0f, 1.0f);

                    if (ImGui::SmallButton("WebPage"))
                    {
                        // Opens the URL in the default browser
                        ShellExecuteW(nullptr, L"open", L"https://ulis.es", nullptr, nullptr, SW_SHOWNORMAL);
                    }
                    ImGui::SameLine(0.0f, 1.0f);
                    if (ImGui::SmallButton("GitHub"))
                    {
                        // Opens the URL in the default browser
                        ShellExecuteW(nullptr, L"open", L"https://github.com/UlisesDeveloper", nullptr, nullptr, SW_SHOWNORMAL);
                    }

                    ImGui::Text("Built with C++, Win32 API & Dear ImGui.");
                    ImGui::Text("Icon by Google (Apache 2.0 License)");
                    ImGui::SameLine(0.0f, 1.0f);
                    if (ImGui::SmallButton("->##icon_link"))
                    {
                        // Opens the URL in the default browser
                        ShellExecuteW(nullptr, L"open", L"https://www.iconarchive.com/show/noto-emoji-travel-places-icons-by-google/42608-stopwatch-icon.html", nullptr, nullptr, SW_SHOWNORMAL);
                    }

                    ImGui::Dummy(ImVec2(0.0f, 20.0f));

                    if (ImGui::Button("GitHub Repo")) {
                        ShellExecuteW(nullptr, L"open", L"https://github.com/UlisesDeveloper/ProgramTimeTracker", nullptr, nullptr, SW_SHOWNORMAL);
                    }

                    /*
                    if (ImGui::Button("Close")) {
                        show_credits_window = false;
                    }
                    */
                }
                ImGui::End();
            }

            // Rendering
            ImGui::Render();
            const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // Present
            HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
            //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
            g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
        }

        // Cleanup
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);


        // Remove Tray Icon on exit
        Shell_NotifyIconW(NIM_DELETE, &s_NID);
        return 0;
    }
    catch (const std::exception& e) {
        Shell_NotifyIconW(NIM_DELETE, &s_NID); //Removes tray icon before exiting
        MessageBoxA(nullptr, e.what(), "Program Time Tracker - Error", MB_ICONERROR | MB_OK); //Error 
        return -1;
    }
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    #ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_CLOSE:
        //HIDES WINDOW INSTEAD OF FULL ON CLOSING IT TO BE ABLE TO MINIMIZE IT
        ::ShowWindow(hWnd, SW_HIDE);
        tracker.saveTime();
        return 0;
    case WM_APP_TRAYMSG:
    {
        // Handle both Left-Click and Right-Click to open the menu
        if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);

            SetForegroundWindow(hWnd);
            //puts it on front of the taskbar icon if not there can be fighting

            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, 1001, L"Open UI");
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit Program");

            // Display the menu at the mouse coordinates
            int clicked = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hWnd, nullptr);
            DestroyMenu(hMenu);

            if (clicked == 1001)
            {
                // opens the ui
                ::ShowWindow(hWnd, SW_SHOW);
                ::SetForegroundWindow(hWnd);
            }
            else if (clicked == ID_TRAY_EXIT)
            {
                // exit program 
                tracker.saveTime();
                Shell_NotifyIconW(NIM_DELETE, &s_NID);
                ::PostQuitMessage(0);
            }
        }
        return 0;
    }
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}



void exceedLimitNotification(const Process& a, bool killed) {
    // extract icon to show
    HICON hTargetIcon = nullptr;
    ExtractIconExA(a.getPathName().c_str(), 0, &hTargetIcon, nullptr, 1);

    MessageBeep(MB_ICONASTERISK);

    //configuration of notif
    s_NID.uFlags = NIF_INFO | NIF_ICON | NIF_MESSAGE | NIF_TIP;
    wcscpy_s(s_NID.szInfoTitle, L"Anti-Procrastinator");
    //needs wide arrays
    

    /**
    if (hTargetIcon != nullptr) {
        //use icon instead of warning
        s_NID.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
        s_NID.hBalloonIcon = hTargetIcon;
    }
    else {
    */
        // yellow triangle fallback if no icon
    if (killed) {
        string narrowName = a.getProcessName();
        wstring wideName(narrowName.begin(), narrowName.end()); //basically get the normal array and call a constructor

        wstring finalMessage = wideName + L" got killed after it exceeded first snooze notification";


        wcscpy_s(s_NID.szInfo, finalMessage.c_str());

        s_NID.dwInfoFlags = NIIF_ERROR | NIIF_LARGE_ICON;
        s_NID.hBalloonIcon = nullptr;
    }
    else {
        string narrowName = a.getProcessName();
        wstring wideName(narrowName.begin(), narrowName.end()); //basically get the normal array and call a constructor

        wstring finalMessage = L"Time's up! You have exceeded your limit for " + wideName;


        wcscpy_s(s_NID.szInfo, finalMessage.c_str());

        s_NID.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
        s_NID.hBalloonIcon = nullptr;
    }
    
    /*
    }
    */
    

    // notif send
    Shell_NotifyIconW(NIM_MODIFY, &s_NID);

    

    if (hTargetIcon != nullptr) {
        DestroyIcon(hTargetIcon);
    }
}

