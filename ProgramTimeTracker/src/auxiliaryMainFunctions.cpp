#include "auxiliaryMainFunctions.h"

#include <string>


void LoadSettingsFile() {
    ifstream file("settings.txt");
    if (file.is_open()) {

        try {
            //Format first timeBeforeTimeOut
            //secsbeforevideotimeout
            //videomodeenabled

            //num Multimedia providers
            //All multimedia providers from that point on
            string curr = "";
            getline(file, curr);
            timeBeforeTimeOut = stoi(curr);
            getline(file, curr);
            secsBeforeVideoTimeOut = stoi(curr);
            getline(file, curr); //1 is true 0 is false
            videoModeEnabled = stoi(curr);

            int prevNumMultimediaProv = numOfMultimediaProviders;
            getline(file, curr);
            numOfMultimediaProviders = stoi(curr);
            if (numOfMultimediaProviders != prevNumMultimediaProv) {

                delete[] multimediaProviders;
                multimediaProviders = new string[numOfMultimediaProviders];
            }

            for (int i = 0; i < numOfMultimediaProviders; i++) {
                getline(file, curr);
                multimediaProviders[i] = curr;
            }

            getline(file, curr); 
            g_DebugMode = stoi(curr);


            getline(file, curr);
            antiProcrastination = stoi(curr);
            getline(file, curr);
            snooze = stoi(curr);
            getline(file, curr);
            snoozeMins = stoi(curr);
            getline(file, curr);
            killAfterSnooze = stoi(curr);



        }
        catch (...) { //Catch exception that stoi may throw need to regenerate the file atp
            SaveSettingsFile(multimediaProviders);
            throw invalid_argument("stoi failed");
        }


        file.close();
    }
    else {
        SaveSettingsFile(multimediaProviders);
        throw invalid_argument("attempted to regenerate the settings file, try to reopen the program");
    }
}

void SaveSettingsFile(string* multimProvWritten) { //when apply has been pressed
    ofstream file("settings.txt");
    if (file.is_open()) {
        //Format first timeBeforeTimeOut
        //secsbeforevideotimeout
        //videomodeenabled

        //num Multimedia providers
        //All multimedia providers from that point on
        file << timeBeforeTimeOut << endl;
        file << secsBeforeVideoTimeOut << endl;
        file << videoModeEnabled << endl;

        //First should remove the garbage
        int validCount = 0;
        for (int i = 0; i < numOfMultimediaProviders; i++) {

            if (!(multimProvWritten[i].empty()) && !(multimProvWritten[i].find_first_not_of(" \t\n\v\f\r") == std::string::npos)) {
                validCount++;
            }
        }


        //Then when it has been removed i can safely save it 
        file << validCount << endl;
        for (int i = 0; i < numOfMultimediaProviders; i++) {
            if (!(multimProvWritten[i].empty()) && !(multimProvWritten[i].find_first_not_of(" \t\n\v\f\r") == std::string::npos)) {
                file << multimProvWritten[i] << endl; //Double check so that i didn't remove the previous ones from the actual multimediaProviders, i am going to kill it regardless to apply the settings
                //Even tho less efficient could be changing the array with a tempo one and then making that multimediaProviders and finally that is the one to be written
            }
        }



        file << g_DebugMode << endl;


        file << antiProcrastination << endl;
        file << snooze << endl;
        file << snoozeMins << endl;
        file << killAfterSnooze << endl;


        file.close();
    }
    else {
        file.close();
        throw invalid_argument("os rejected, replace it with the one on github");
    }
}


//that second parameter because its defined in main and its static, so we need to pass it
const ID3D11ShaderResourceView* showIconFromProcess(const Process& a, ID3D11Device* d3dDevice) {
   
    string targetPath = getRealIconPath(a);


    HICON hIconLarge = nullptr;
    UINT numIconExtracted = ExtractIconExA(targetPath.c_str(), 0, &hIconLarge, nullptr, 1);

    if (numIconExtracted = 0 || hIconLarge == nullptr) { //Safety check 
        return nullptr; //it makes sense to not do all that bullshit with directx if not i lose to much performance not worth the readability
    }


    ICONINFO iconInformation;
    //Extract color info
    if (!GetIconInfo(hIconLarge, &iconInformation)) { //gives iconInfo 2 bitmaps one color & colorless
        //need to get info from the bitmap
        DestroyIcon(hIconLarge); //Safety check
        return nullptr;
    }
    BITMAP infoHolder = {};
    
    if (GetObject(iconInformation.hbmColor, sizeof(BITMAP), &infoHolder) == 0) {
        // Clean up memory b4 aborting
        if (iconInformation.hbmColor) {
            DeleteObject(iconInformation.hbmColor);
        }
        if (iconInformation.hbmMask) {
            DeleteObject(iconInformation.hbmMask);
        }
        DestroyIcon(hIconLarge);
        return nullptr;
    }


    if (infoHolder.bmWidth <= 0 || infoHolder.bmHeight <= 0) { //Safety check 
        DeleteObject(iconInformation.hbmColor);
        DeleteObject(iconInformation.hbmMask);
        DestroyIcon(hIconLarge);
        return nullptr;
    }


    BYTE* pixelData = new BYTE[infoHolder.bmWidth * infoHolder.bmHeight * 4]; //Where the pixels get stored like a grid
    //*4 cause 4 bytes per pixel rgb alpha FOR WHATEVER REASON


    //We need a LPBITMAPINFO because the pixeldata needs properties stored somewhere and that's the struct function
    BITMAPINFO blueprint = {};
    //Fill
    blueprint.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    blueprint.bmiHeader.biWidth = infoHolder.bmWidth;
    blueprint.bmiHeader.biHeight = -infoHolder.bmHeight; // for whatever reason if height isn't negative it draws it upside down
    blueprint.bmiHeader.biPlanes = 1;
    blueprint.bmiHeader.biBitCount = 32;
    blueprint.bmiHeader.biCompression = BI_RGB; //uncompressed



    HDC dcgetter = GetDC(nullptr); //Temp screen canvas which get's released after the bitmap 
    GetDIBits(dcgetter, 
        iconInformation.hbmColor, 
        0, 
        infoHolder.bmHeight,
        pixelData,
        &(blueprint),
        DIB_RGB_COLORS

        
        );    //Gets the bits from the bitmap
    //first it needs the dc, second the bitmap it extracts the pixel from we want the color one, 3rd where it starts so row 0, 4th is how many rows so we want the size, 5th is where it will all be saved so a byte array with all the things needed, 5th is a blueprint with info/properties of the bitmap, and 6th is to select rgb instead of another option
    ReleaseDC(nullptr, dcgetter);


    //to clear ram
    DeleteObject(iconInformation.hbmColor);
    DeleteObject(iconInformation.hbmMask);
    DestroyIcon(hIconLarge);



    ID3D11ShaderResourceView* myIconTextureView = nullptr; //getter for the image for the shader
    ID3D11Texture2D* pTexture = nullptr; //image stored in vram

    D3D11_TEXTURE2D_DESC desc = {}; //Canvas in vram that gets filled with pixels (blueprint)
    desc.Width = infoHolder.bmWidth;
    desc.Height = infoHolder.bmHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //Format of windows icons
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;


    //To send pixeldata each byte propery to the gpu
    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = pixelData;             
    subResource.SysMemPitch = desc.Width * 4;



    //Create the texture
    HRESULT hr = (*d3dDevice).CreateTexture2D(&desc, &subResource, &pTexture);
    if (SUCCEEDED(hr)) //if can create the texture
    {
        //Create shader resource viewer
        (*d3dDevice).CreateShaderResourceView(pTexture, nullptr, &myIconTextureView);

        //no need for the texture anymore
        (*pTexture).Release();
    }
    
    delete[] pixelData; //cpu no longer needs it as the gpu has it


    return myIconTextureView;

}


string getRealIconPath(const Process& a) {
    string targetPath = a.getPathName();

    //if it's not an uwp_ we leave it as is
    if (targetPath.length() >= 4 && targetPath.substr(0, 4) == "uwp_") {

        // open process with the pid
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, a.getPid());

        if (hProcess != NULL) {
            char buffer[MAX_PATH];
            DWORD size = MAX_PATH;

            //get real path 
            if (QueryFullProcessImageNameA(hProcess, 0, buffer, &size)) {
                targetPath = string(buffer, size); //we have the real path.
            }
            else {
                targetPath = "C:\\Windows\\System32\\ApplicationFrameHost.exe"; //fallback
            }
            CloseHandle(hProcess);
        }
        else {
            targetPath = "C:\\Windows\\System32\\ApplicationFrameHost.exe";
        }
    }

    return targetPath;
}



ID3D11ShaderResourceView* showIconFromPath(std::string targetPath, ID3D11Device* d3dDevice) {

    if (targetPath.length() >= 4 && targetPath.substr(0, 4) == "uwp_") {
        targetPath = "C:\\Windows\\System32\\ApplicationFrameHost.exe";
    }

    HICON hIconLarge = nullptr;
    UINT numIconExtracted = ExtractIconExA(targetPath.c_str(), 0, &hIconLarge, nullptr, 1);

    if (numIconExtracted == 0 || hIconLarge == nullptr) {
        return nullptr;
    }

    

    ICONINFO iconInformation;
    if (!GetIconInfo(hIconLarge, &iconInformation)) {
        DestroyIcon(hIconLarge);
        return nullptr;
    }

    BITMAP infoHolder = {};
    if (GetObject(iconInformation.hbmColor, sizeof(BITMAP), &infoHolder) == 0) {
        if (iconInformation.hbmColor) DeleteObject(iconInformation.hbmColor);
        if (iconInformation.hbmMask) DeleteObject(iconInformation.hbmMask);
        DestroyIcon(hIconLarge);
        return nullptr;
    }

    if (infoHolder.bmWidth <= 0 || infoHolder.bmHeight <= 0) {
        DeleteObject(iconInformation.hbmColor);
        DeleteObject(iconInformation.hbmMask);
        DestroyIcon(hIconLarge);
        return nullptr;
    }

    BYTE* pixelData = new BYTE[infoHolder.bmWidth * infoHolder.bmHeight * 4];
    BITMAPINFO blueprint = {};
    blueprint.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    blueprint.bmiHeader.biWidth = infoHolder.bmWidth;
    blueprint.bmiHeader.biHeight = -infoHolder.bmHeight;
    blueprint.bmiHeader.biPlanes = 1;
    blueprint.bmiHeader.biBitCount = 32;
    blueprint.bmiHeader.biCompression = BI_RGB;

    HDC dcgetter = GetDC(nullptr);
    GetDIBits(dcgetter, iconInformation.hbmColor, 0, infoHolder.bmHeight, pixelData, &(blueprint), DIB_RGB_COLORS);
    ReleaseDC(nullptr, dcgetter);

    DeleteObject(iconInformation.hbmColor);
    DeleteObject(iconInformation.hbmMask);
    DestroyIcon(hIconLarge);

    ID3D11ShaderResourceView* myIconTextureView = nullptr;
    ID3D11Texture2D* pTexture = nullptr;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = infoHolder.bmWidth;
    desc.Height = infoHolder.bmHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = pixelData;
    subResource.SysMemPitch = desc.Width * 4;

    HRESULT hr = (*d3dDevice).CreateTexture2D(&desc, &subResource, &pTexture);
    if (SUCCEEDED(hr)) {
        (*d3dDevice).CreateShaderResourceView(pTexture, nullptr, &myIconTextureView);
        (*pTexture).Release();
    }

    delete[] pixelData;
    return myIconTextureView;
}



//uwp + file name shenanigans


string getStableTrackerID(const string& fullPath, const string& processName) { //We will refer to id if it's non uwp we will have the full path if not uwp prefix
    string stablePath = fullPath; //it's supposed to be use so that we get the full path or the uwp equivalent
    string res = "";

    for (char& c : stablePath) { //Same as foreach loop in c#
        c = std::tolower(static_cast<unsigned char>(c)); //the cast to unsigned char to remove accents 
    }
    res = stablePath;

    //detect if it's an uwp
    if (stablePath.find("windowsapps") != std::string::npos) { //basically 

        string lowerProcessName = processName;
        for (char& c : lowerProcessName) {
            c = tolower(static_cast<unsigned char>(c));
        }

        res = "uwp_" + lowerProcessName;
    }

    return res;
}

//file name cleaner
string sanitizePathForFileName(string pathOrId) { //basically to save the file
    std::string safeName = "";

    //replace illegal filename chars with safe ones
    for (char& c : pathOrId) { //Doesn't like constant reference for whatever reason

        // remove control characters (like \0, \n, \r)
        if (c >= 0 && c < 32) {
            continue;
        }

        if (c == '\\' || c == '/') { //Slashes are #
            safeName += '#';
        }
        else if (c == ':') { //drive selector
            safeName += '@';
        }
        else if (c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') { //jic
            safeName += '_';
        }
        else {
            safeName += c;
        
        }
    }


    //the max path is 260 chars if the path is deep we need to cut some info so that we can ofstream
    if (safeName.length() > 200) {
        //First 100 chars and the last 100 + ___
        safeName = safeName.substr(0, 100) + "___" + safeName.substr(safeName.length() - 100);
    }

    return safeName + ".pttl";
}











