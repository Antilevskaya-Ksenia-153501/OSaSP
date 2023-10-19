#include<windows.h>
#include <string>
#include <sstream>

HWND hwndEdit;
HWND hwndButton[16];
HWND hwndButtonClear;
std::string currentExpression;

LPCWSTR ConvertToLPCWSTR(const char* charString)
{
    int stringLength = strlen(charString) + 1; // +1 for null terminator
    int wideLength = MultiByteToWideChar(CP_UTF8, 0, charString, stringLength, NULL, 0);
    wchar_t* wideString = new wchar_t[wideLength];
    MultiByteToWideChar(CP_UTF8, 0, charString, stringLength, wideString, wideLength);
    return wideString;
}

LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
        case WM_CREATE: {
            hwndEdit = CreateWindow(L"EDIT", L"",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_RIGHT,
                10, 10, 235, 30,
                hwnd, NULL, NULL, NULL);

            char buttonText[16][4] = { "7", "8", "9", "/",
                                       "4", "5", "6", "*",
                                       "1", "2", "3", "-",
                                       "0", ".", "=", "+"};

            int buttonPos[16][2] = { {10, 50}, {70, 50},
                                     {130, 50}, {190, 50},
                                     {10, 90}, {70, 90}, 
                                     {130, 90}, {190, 90},
                                     {10, 130}, {70, 130},
                                     {130, 130}, {190, 130},
                                     {10, 170}, {70, 170},
                                     {130, 170}, {190, 170}};
          
            for (int i = 0; i < 16; i++)
            {
                hwndButton[i] = CreateWindow(L"BUTTON", ConvertToLPCWSTR(buttonText[i]),
                    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                    buttonPos[i][0], buttonPos[i][1], 50, 30,
                    hwnd, (HMENU)(i + 1), NULL, NULL);
            }
            hwndButtonClear = CreateWindow(L"BUTTON", L"C",
                WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                250,10, 50, 30,
                hwnd, (HMENU)(17), NULL, NULL);
            break;
        }

        case WM_COMMAND: {
            if (HIWORD(wparam) == BN_CLICKED)
            {
                int buttonId = LOWORD(wparam);

                if (buttonId >= 1 && buttonId <= 16 && buttonId != 15)
                {
                    char buffer[2];
                    GetWindowTextA(hwndButton[buttonId - 1], buffer, 2);
                    currentExpression += buffer;

                    SetWindowTextA(hwndEdit, currentExpression.c_str());
                }
                else if (buttonId == 15) // "=" button
                {
                    std::stringstream expressionStream(currentExpression);
                    double result = 0.0;
                    char op;

                    expressionStream >> result;

                    while (expressionStream >> op)
                    {
                        double operand;
                        expressionStream >> operand;

                        switch (op){
                            case '+':
                                result += operand;
                                break;
                            case '-':
                                result -= operand;
                                break;
                            case '*':
                                result *= operand;
                                break;
                            case '/':
                                result /= operand;
                                break;
                        }
                    }

                    currentExpression = std::to_string(result);
                    SetWindowTextA(hwndEdit, currentExpression.c_str());
                }
                else if (buttonId == 17) {
                    SetWindowTextA(hwndEdit, " ");
                    currentExpression = " ";
                }
            }
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            break; 
        }

	    default:
		    return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	MSG msg{};
	HWND hwnd{};
	WNDCLASS ws{};
	ws.cbClsExtra = 0;
	ws.cbWndExtra = 0;
	ws.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	ws.hCursor = LoadCursor(nullptr, IDC_ARROW);
	ws.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	ws.hInstance = hInstance;
	ws.lpfnWndProc = WndProc;
	ws.lpszClassName = L"MyCalculatorClass";
	ws.lpszMenuName = nullptr;

	if (!RegisterClass(&ws))
		return 1;

	hwnd = CreateWindow(ws.lpszClassName, L"Calculator", WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, nullptr, nullptr, ws.hInstance, nullptr);
	if (!hwnd)
		return EXIT_FAILURE;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}