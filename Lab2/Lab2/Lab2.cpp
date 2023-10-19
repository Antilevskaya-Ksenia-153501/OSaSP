#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

struct AppContext {
    int currentData = 0;
};

// Save data in file
void SaveAppContext(const AppContext& context) {
    std::ofstream file("appcontext.dat", std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&context), sizeof(context));
        file.close();
    }
}

// Get data from file
AppContext LoadAppContext() {
    AppContext context{};
    std::ifstream file("appcontext.dat", std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(&context), sizeof(context));
        file.close();
    }
    return context;
}


int DoWork(AppContext& context) {
    int data = context.currentData;
    context.currentData++;
    SaveAppContext(context);
    return data;
    
}

void StartNewProcess() {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    TCHAR szCommandLine[MAX_PATH] = TEXT("C:\\Users\\Notebook\\Desktop\\OSaSP\\Lab2\\x64\\Debug\\Lab2.exe");

    if (CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

void UpdateWindowText(HWND hwnd, const std::wstring& text) {
    SetWindowText(hwnd, text.c_str());
}

static HWND hwndOutput;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        hwndOutput = CreateWindowEx(
            0, TEXT("STATIC"), NULL,
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            10, 50, 200, 30,
            hwnd, NULL, NULL, NULL
        );
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1001) {
            ExitProcess(0);
        }
    case WM_CLOSE:
        StartNewProcess();
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("SelfRecoveryWindowClass");
    if (!RegisterClass(&wc))
        return 1;

    HWND hwnd = CreateWindowEx(
        0,
        TEXT("SelfRecoveryWindowClass"),
        TEXT("Self-Recovery Window"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    HWND hwndButton = CreateWindow(
        TEXT("BUTTON"),     
        TEXT("Завершить процесс"),  
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  
        10, 10, 150, 30,    
        hwnd,               
        reinterpret_cast<HMENU>(1001),  
        hInstance,          
        NULL
    );


    AppContext context = LoadAppContext();
    int data =  DoWork(context);
    SaveAppContext(context);
    std::wstring text = std::to_wstring(data);

    UpdateWindowText(hwndOutput, text);
    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);

    return static_cast<int>(msg.wParam);
}


