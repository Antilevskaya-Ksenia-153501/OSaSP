#include <iostream>
#include <windows.h>
#include <string>

#define BUFFER_SIZE 1024

int main()
{
    HANDLE pipe;
    char buffer[BUFFER_SIZE];

    // Подключение к именованному каналу
    pipe = CreateFile(
        L"\\\\.\\pipe\\LogPipe", // Имя канала
        GENERIC_READ |          // Режим доступа: чтение
        GENERIC_WRITE,          // Режим доступа: запись
        0,                      // Режим деления: эксклюзивный доступ
        NULL,                   // Атрибуты безопасности по умолчанию
        OPEN_EXISTING,          // Действие при открытии существующего канала
        0,                      // Флаги и атрибуты файла по умолчанию
        NULL                    // Используется только для шаблонов файлов
    );

    if (pipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Error connecting to channel. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "The client is connected to the server. Enter message to send (q - output): " << std::endl;

    while (true)
    {
        // Чтение сообщения с консоли
        std::string message;
        std::getline(std::cin, message);

        if (message == "q")
            break;

        // Отправка сообщения серверу
        DWORD bytesWritten;
        if (!WriteFile(pipe, message.c_str(), message.length(), &bytesWritten, NULL))
        {
            std::cout << "Error writing to channel. Error code: " << GetLastError() << std::endl;
            break;
        }

        // Ожидание подтверждения от сервера
        DWORD bytesRead;
        if (!ReadFile(pipe, buffer, BUFFER_SIZE, &bytesRead, NULL))
        {
            std::cout << "Error reading from channel. Error code: " << GetLastError() << std::endl;
            break;
        }

        // Вывод подтверждения от сервера
        std::cout << "Received response from server: " << std::string(buffer, bytesRead) << std::endl;
    }

    // Закрытие канала
    CloseHandle(pipe);

    return 0;
}

