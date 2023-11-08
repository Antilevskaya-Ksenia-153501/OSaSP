#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <Windows.h>
#include <Commdlg.h>

struct AsyncData {
    HANDLE fileHandle;
    HWND buttonHandle;
    HWND resultTextHandle;
    std::vector<char> buffer;
    OVERLAPPED overlapped;
    void (*readCallback)(DWORD dwErrorCode, DWORD dwBytesRead, OVERLAPPED* pOverlapped);
};

VOID CALLBACK ReadCallback(DWORD errorCode, DWORD bytesRead, LPOVERLAPPED overlapped) {
    AsyncData* asyncData = reinterpret_cast<AsyncData*>(overlapped->hEvent);

    if (errorCode != ERROR_SUCCESS) {
        std::cerr << "Error while reading file: " << errorCode << std::endl;
        return;
    }

    std::string inputString(asyncData->buffer.begin(), asyncData->buffer.end());

    // Разбиение строки на слова
    std::vector<std::string> words;
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    while ((pos = inputString.find(delimiter)) != std::string::npos) {
        token = inputString.substr(0, pos);
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        words.push_back(token);
        inputString.erase(0, pos + delimiter.length());
    }
    words.push_back(inputString); // Добавление последнего слова
    std::sort(words.begin(), words.end());

    for (const auto& word : words) {
        int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, word.c_str(), -1, nullptr, 0);
        std::wstring wideString(wideCharLength, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, word.c_str(), -1, &wideString[0], wideCharLength);

        LPCWSTR lpcwstr = wideString.c_str();
        SendMessage(asyncData->resultTextHandle, EM_REPLACESEL, TRUE, (LPARAM)lpcwstr);

        // Отправка сообщения EM_REPLACESEL для добавления символа перевода строки
        SendMessage(asyncData->resultTextHandle, EM_REPLACESEL, TRUE, (LPARAM)L"\r\n");
    }
    EnableWindow(asyncData->buttonHandle, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static AsyncData asyncData;

    switch (msg) {
    case WM_CREATE: {
        // Create the "Open File" button
        asyncData.buttonHandle = CreateWindow(
            L"BUTTON", L"Open File",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 10, 100, 30,
            hwnd, reinterpret_cast<HMENU>(1), nullptr, nullptr
        );

        // Create the text field to display the sorted data
        asyncData.resultTextHandle = CreateWindow(
            L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 50, 500, 500,
            hwnd, nullptr, nullptr, nullptr
        );

        break;
    }
    case WM_COMMAND: {
        if (HIWORD(wParam) == BN_CLICKED) {
            if (reinterpret_cast<HWND>(lParam) == asyncData.buttonHandle) {
                OPENFILENAMEW ofn;
                wchar_t fileName[MAX_PATH] = { 0 };

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = L"All Files\0*.*\0";
                ofn.lpstrFile = fileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

                if (GetOpenFileNameW(&ofn)) {
                    // Disable the button while reading the file
                    EnableWindow(asyncData.buttonHandle, FALSE);

                    // Open the selected file
                    asyncData.fileHandle = CreateFileW(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

                    if (asyncData.fileHandle != INVALID_HANDLE_VALUE) {
                        // Allocate the buffer for reading file data
                        asyncData.buffer.resize(10000);

                        // Prepare the overlapped structure
                        asyncData.overlapped.Offset = 0;
                        asyncData.overlapped.OffsetHigh = 0;
                        asyncData.overlapped.hEvent = reinterpret_cast<HANDLE>(&asyncData);
                        asyncData.readCallback = ReadCallback;

                        // Start the asynchronous read operation
                        bool res = ReadFileEx(asyncData.fileHandle, asyncData.buffer.data(), asyncData.buffer.size(), &asyncData.overlapped, asyncData.readCallback);
                        DWORD dwBytesTransferred;
                        BOOL bResult = GetOverlappedResult(asyncData.fileHandle, &(asyncData.overlapped), &dwBytesTransferred, TRUE);
                        if (!bResult) {
                            DWORD dwError = GetLastError();
                            std::cerr << "Error of getting data from file: " << dwError << std::endl;
                        }
                        asyncData.readCallback(ERROR_SUCCESS, 0, &(asyncData.overlapped));
                    }
                    else {
                        std::cerr << "Failed to open file: " << ofn.lpstrFile << std::endl;
                        EnableWindow(asyncData.buttonHandle, TRUE);
                    }
                }
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = hInstance;
    wc.lpszClassName = L"AsyncFileProcClass";
    wc.lpfnWndProc = WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassEx(&wc)) {
        return -1;
    }

    // Create the window
    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, L"Asynchronous File Processing",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 1000, nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        return -1;
    }

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Enter the message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}