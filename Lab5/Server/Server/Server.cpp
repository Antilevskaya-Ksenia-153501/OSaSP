#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <string>
#include <fstream>
#include <ctime>

const std::wstring PIPE_NAME = L"\\\\.\\pipe\\LogPipe";
const int MAX_CLIENTS = 5;

// ������� ��� ��������� ������� ������� � ��������� ������
void ProcessClient(HANDLE hPipe, int clientID)
{
    char buffer[1024];
    DWORD bytesRead;

    // ������ ��������� �� �������
    while (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL) != FALSE)
    {

        // TODO: ������ ��������� � ����
        /* ��������, �������� �� ��������� �������� ����������*/
        std::string message(buffer, bytesRead);
        if (message == "q")
            break;

        // ��������� ����������� ���������
        std::cout << "Received message from client " << clientID << ": " << message << std::endl;

        // ������ ��������� � ���� (������ ������ �� �������)
        std::ofstream logFile("log.txt", std::ios::app);
        if (logFile.is_open())
        {
            std::time_t currentTime = std::time(nullptr);
            std::tm localTime{};
            localtime_s(&localTime, &currentTime);

            char timeString[50]; // ����� ��� �������� ���������������� �������
            std::strftime(timeString, sizeof(timeString), "[%Y-%m-%d %H:%M:%S]", &localTime);

            logFile << timeString << " [Client " << clientID << "] " << message << std::endl;

            logFile.close();
        }

        // �������� ������������� �������
        const char* response = "Message received";
        DWORD bytesWritten;
        WriteFile(hPipe, response, strlen(response) + 1, &bytesWritten, NULL);
    }

    // �������� ������
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

int main()
{
    std::vector<std::thread> clientThreads;
    int clientID = 0;

    // �������� ������������ ������
    HANDLE hPipe = CreateNamedPipe(
        PIPE_NAME.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024,
        1024,
        0,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to create named pipe. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Waiting for clients to connect..." << std::endl;

    // �������� ����������� ��������
    while (clientID < MAX_CLIENTS)
    {
        if (ConnectNamedPipe(hPipe, NULL) != FALSE)
        {
            std::cout << "Client " << clientID << " connected. Waiting for messages..." << std::endl;

            // �������� ������ ������ ��� ��������� �������
            std::thread clientThread(ProcessClient, hPipe, clientID);
            clientThread.detach(); // ������������ ������ �� ��������� ������ �������

            // �������� ������ ������������ ������ ��� ���������� �������
            hPipe = CreateNamedPipe(
                PIPE_NAME.c_str(),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                1024,
                1024,
                0,
                NULL);

            clientID++;
        }
    }

    // �������� ������
    CloseHandle(hPipe);

    return 0;
}

