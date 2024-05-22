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
    SOCKET ListenSocket = INVALID_SOCKET;  // Сокет для прослушивания входящих соединений
    SOCKET ConnectSocket = INVALID_SOCKET;  // Сокет для установленного соединения
    char recvBuffer[512];  // Буфер для приема данных

    const char* sendBuffer = "Bread";  // Данные для отправки клиенту

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
    hints.ai_flags = AI_PASSIVE;  // Указание на использование для прослушивания

    // Разрешение локального адреса и порта для использования сервером
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo завершился с ошибкой: " << result << endl;
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Создание сокета для прослушивания входящих соединений
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Не удалось создать сокет" << endl;
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Привязка сокета к адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Не удалось выполнить bind, ошибка: " << result << endl;
        closesocket(ListenSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Начало прослушивания входящих запросов на соединение
    result = listen(ListenSocket, SOMAXCONN);  // SOMAXCONN указывает максимальное количество ожидающих соединений
    if (result == SOCKET_ERROR) {
        cout << "Не удалось выполнить listen, ошибка: " << result << endl;
        closesocket(ListenSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Принятие входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Не удалось выполнить accept, ошибка: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Закрытие сокета для прослушивания, так как больше не нужен
    closesocket(ListenSocket);

    // Получение данных от клиента
    do {
        ZeroMemory(recvBuffer, 512);  // Очистка буфера для приема данных
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получение данных
        if (result > 0) {
            cout << "Получено " << result << " байт" << endl;
            cout << "Полученные данные: " << recvBuffer << endl;

            // Отправка ответа клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Не удалось отправить данные, ошибка: " << result << endl;
                closesocket(ConnectSocket);  // Закрытие сокета
                freeaddrinfo(addrResult);  // Освобождение структуры адреса
                WSACleanup();  // Завершение работы с библиотекой Winsock
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Соединение закрывается" << endl;
        }
        else {
            cout << "Не удалось выполнить recv, ошибка: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);  // Закрытие сокета
            freeaddrinfo(addrResult);  // Освобождение структуры адреса
            WSACleanup();  // Завершение работы с библиотекой Winsock
            return 1;
        }
    } while (result > 0);

    // Завершение отправки данных, так как больше не будем ничего отправлять
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Не удалось выполнить shutdown, ошибка: " << result << endl;
        closesocket(ConnectSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);  // Освобождение структуры адреса
        WSACleanup();  // Завершение работы с библиотекой Winsock
        return 1;
    }

    // Очистка и закрытие сокета
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);  // Освобождение структуры адреса
    WSACleanup();  // Завершение работы с библиотекой Winsock
    return 0;
}
