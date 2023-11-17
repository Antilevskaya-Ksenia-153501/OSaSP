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

// Функция для обработки каждого клиента в отдельном потоке
void ProcessClient(HANDLE hPipe, int clientID)
{
    char buffer[1024];
    DWORD bytesRead;

    // Чтение сообщений от клиента
    while (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL) != FALSE)
    {

        // TODO: Запись сообщения в файл
        /* Проверка, является ли сообщение командой завершения*/
        std::string message(buffer, bytesRead);
        if (message == "q")
            break;

        // Обработка полученного сообщения
        std::cout << "Received message from client " << clientID << ": " << message << std::endl;

        // Запись сообщения в файл (вместо вывода на консоль)
        std::ofstream logFile("log.txt", std::ios::app);
        if (logFile.is_open())
        {
            std::time_t currentTime = std::time(nullptr);
            std::tm localTime{};
            localtime_s(&localTime, &currentTime);

            char timeString[50]; // Буфер для хранения форматированного времени
            std::strftime(timeString, sizeof(timeString), "[%Y-%m-%d %H:%M:%S]", &localTime);

            logFile << timeString << " [Client " << clientID << "] " << message << std::endl;

            logFile.close();
        }

        // Отправка подтверждения клиенту
        const char* response = "Message received";
        DWORD bytesWritten;
        WriteFile(hPipe, response, strlen(response) + 1, &bytesWritten, NULL);
    }

    // Закрытие канала
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

int main()
{
    std::vector<std::thread> clientThreads;
    int clientID = 0;

    // Создание именованного канала
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

    // Ожидание подключения клиентов
    while (clientID < MAX_CLIENTS)
    {
        if (ConnectNamedPipe(hPipe, NULL) != FALSE)
        {
            std::cout << "Client " << clientID << " connected. Waiting for messages..." << std::endl;

            // Создание нового потока для обработки клиента
            std::thread clientThread(ProcessClient, hPipe, clientID);
            clientThread.detach(); // Отсоединение потока от основного потока сервера

            // Создание нового именованного канала для следующего клиента
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

    // Закрытие канала
    CloseHandle(hPipe);

    return 0;
}

