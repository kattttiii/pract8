#define WIN32_LEAN_AND_MEAN  // Макрос отключает редко используемые компоненты Windows API для уменьшения размера подключаемых заголовков
#include <iostream>  // Подключение стандартной библиотеки ввода-вывода C++
#include <Windows.h>  // Подключение основного заголовка Windows API
#include <WinSock2.h>  // Подключение заголовка для работы с сокетами
#include <WS2tcpip.h>  // Подключение дополнительных функций для работы с TCP/IP

using namespace std;

int main()
{
    WSADATA wsaData;  // Структура для хранения информации о реализации Windows Sockets
    ADDRINFO hints;  // Структура для хранения критериев поиска адресов
    ADDRINFO* addrResult;  // Указатель для хранения результатов функции getaddrinfo
    SOCKET ConnectSocket = INVALID_SOCKET;  // Сокет для установленного соединения
    char recvBuffer[512];  // Буфер для приема данных

    const char* sendBuffer1 = "Bread";  // Первое сообщение для отправки
    const char* sendBuffer2 = "Хлеб";  // Второе сообщение для отправки

    // Инициализация библиотеки Winsock, использование версии 2.2
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup завершился с ошибкой: " << result << endl;
        return 1;
    }

    // Обнуление структуры hints и установка параметров для разрешения адресов
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;  // Использование IPv4
    hints.ai_socktype = SOCK_STREAM;  // Потоковый сокет (TCP)
    hints.ai_protocol = IPPROTO_TCP;  // Протокол TCP

    // Разрешение адреса сервера и порта для подключения клиента
    result = getaddrinfo("localhost", "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo завершился с ошибкой: " << result << endl;
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Создание сокета для подключения к серверу
    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Не удалось создать сокет" << endl;
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Подключение к серверу
    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Не удалось подключиться к серверу" << endl;
        closesocket(ConnectSocket);  // Закрытие сокета
        ConnectSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Отправка первого сообщения на сервер
    result = send(ConnectSocket, sendBuffer1, (int)strlen(sendBuffer1), 0);
    if (result == SOCKET_ERROR) {
        cout << "Не удалось отправить данные, ошибка: " << result << endl;
        closesocket(ConnectSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }
    cout << "Отправлено: " << result << " байт" << endl;

    // Отправка второго сообщения на сервер
    result = send(ConnectSocket, sendBuffer2, (int)strlen(sendBuffer2), 0);
    if (result == SOCKET_ERROR) {
        cout << "Не удалось отправить данные, ошибка: " << result << endl;
        closesocket(ConnectSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }
    cout << "Отправлено: " << result << " байт" << endl;

    // Завершение отправки данных, так как больше не будем ничего отправлять
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Не удалось выполнить shutdown, ошибка: " << result << endl;
        closesocket(ConnectSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Получение данных от сервера
    do {
        ZeroMemory(recvBuffer, 512);  // Очистка буфера для приема данных
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получение данных
        if (result > 0) {
            cout << "Получено " << result << " байт" << endl;
            cout << "Полученные данные: " << recvBuffer << endl;
        }
        else if (result == 0) {
            cout << "Соединение закрыто" << endl;
        }
        else {
            cout << "Не удалось выполнить recv, ошибка: " << WSAGetLastError() << endl;
        }
    } while (result > 0);

    // Очистка и закрытие сокета
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);  // Освобождение структуры адреса
    WSACleanup();  // Завершение работы с библиотекой Winsock
    return 0;
}
