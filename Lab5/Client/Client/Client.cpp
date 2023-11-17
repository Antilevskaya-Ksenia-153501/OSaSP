#include <iostream>
#include <windows.h>
#include <string>

#define BUFFER_SIZE 1024

int main()
{
    HANDLE pipe;
    char buffer[BUFFER_SIZE];

    // ����������� � ������������ ������
    pipe = CreateFile(
        L"\\\\.\\pipe\\LogPipe", // ��� ������
        GENERIC_READ |          // ����� �������: ������
        GENERIC_WRITE,          // ����� �������: ������
        0,                      // ����� �������: ������������ ������
        NULL,                   // �������� ������������ �� ���������
        OPEN_EXISTING,          // �������� ��� �������� ������������� ������
        0,                      // ����� � �������� ����� �� ���������
        NULL                    // ������������ ������ ��� �������� ������
    );

    if (pipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Error connecting to channel. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "The client is connected to the server. Enter message to send (q - output): " << std::endl;

    while (true)
    {
        // ������ ��������� � �������
        std::string message;
        std::getline(std::cin, message);

        if (message == "q")
            break;

        // �������� ��������� �������
        DWORD bytesWritten;
        if (!WriteFile(pipe, message.c_str(), message.length(), &bytesWritten, NULL))
        {
            std::cout << "Error writing to channel. Error code: " << GetLastError() << std::endl;
            break;
        }

        // �������� ������������� �� �������
        DWORD bytesRead;
        if (!ReadFile(pipe, buffer, BUFFER_SIZE, &bytesRead, NULL))
        {
            std::cout << "Error reading from channel. Error code: " << GetLastError() << std::endl;
            break;
        }

        // ����� ������������� �� �������
        std::cout << "Received response from server: " << std::string(buffer, bytesRead) << std::endl;
    }

    // �������� ������
    CloseHandle(pipe);

    return 0;
}

