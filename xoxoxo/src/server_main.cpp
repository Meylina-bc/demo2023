#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

#include "config_reader.hpp"
#include "logger.hpp"
#include "user_manager.hpp"
#include "tic_tac_toe_logic.hpp"

// Вспомогательная функция приёма строки
std::string receiveMessage(int sock) {
    std::string line;
    char c;
    while (true) {
        int ret = recv(sock, &c, 1, 0);
        if (ret <= 0) {
            return "";
        }
        if (c == '\n') {
            break;
        }
        line.push_back(c);
    }
    return line;
}

// Отправка строки
bool sendMessage(int sock, const std::string& message) {
    std::string msg = message + "\n";
    int sent = send(sock, msg.c_str(), msg.size(), 0);
    return (sent == (int)msg.size());
}

// Предположим, у нас есть функция authorize(...), как и раньше:
std::string authorize(int sock, const std::vector<User>& users, const std::string& logFile) {
    std::string login = receiveMessage(sock);
    std::string pass  = receiveMessage(sock);
    printLog(logFile, "Логин: " + login);
    printLog(logFile, "Пароль: " + pass);

    if (checkCredentials(users, login, pass)) {
        sendMessage(sock, "OK");
        printLog(logFile, "Пользователь " + login + " успешно авторизовался");
        return login; // возвращаем логин
    } else {
        sendMessage(sock, "FAIL");
        printLog(logFile, "Неуспешная авторизация: " + login);
        return "";
    }
}

bool isSocketAlive(int sock) {
    // 1) Шлём PING
    bool ok = sendMessage(sock, "PING");
    if (!ok) {
        // send вернул ошибку => клиент, скорее всего, отвалился
        return false;
    }

    // 2) Ждём PONG c помощью poll
    struct pollfd pfd;
    pfd.fd = sock;
    pfd.events = POLLIN; // ждём на чтение
    int timeoutMs = 2000; // две секунды на ответ
    int pollRes = poll(&pfd, 1, timeoutMs);

    if (pollRes <= 0) {
        // = 0 => таймаут, < 0 => ошибка
        return false;
    }

    // Есть данные => читаем
    std::string resp = receiveMessage(sock);
    if (resp == "PONG") {
        return true; // клиенты ответил нормально
    } else {
        return false; // пришло что-то не то
    }
}

int main(int argc, char* argv[]) {
    // По умолчанию используем "server_config.txt"
    std::string configFile = "server_config.txt";
    if (argc > 1) {
        // Если пользователь указал имя файла в аргументах
        configFile = argv[1];
    }
    // Считываем конфигурационный файл
    auto config = readConfig(configFile);
    // Получаем необходимые параметры
    int port = 5555;
    if (config.find("port") != config.end()) {
        port = std::stoi(config["port"]);
    }
    int moveTime = 10;
    if (config.find("moveTime") != config.end()) {
        moveTime = std::stoi(config["moveTime"]);
    }
    std::string logFile = "server.log";
    if (config.find("logFile") != config.end()) {
        logFile = config["logFile"];
    }

    // Разделяем логи от прошлого запуска
    writeLog(logFile, "");
    writeLog(logFile, "------");

    // Загружаем список пользователей
    auto users = loadUsers("users.txt");

    // Создаём серверный сокет
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printLog(logFile, "Ошибка создания сокета");
        return 1;
    }

    // Опция для быстрой перепривязки порта (необязательно)
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Связываем сокет с портом
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printLog(logFile, "Ошибка bind");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 2) < 0) {
        printLog(logFile, "Ошибка listen");
        close(serverSocket);
        return 1;
    }

    printLog(logFile, "Сервер запущен. Порт: " + std::to_string(port));

    // Принимаем двух клиентов
    std::vector<int> clientSockets;
    std::vector<std::string> clientUsers;
    clientSockets.reserve(2);
    clientUsers.reserve(2);
    while (true) {
        // Перед тем, как звать accept, можно проверить, не отвалился ли кто-то из уже подключённых:
        size_t i = 0;
        while (i < clientSockets.size()) {
            if (!isSocketAlive(clientSockets[i])) {
                printLog(logFile, "Клиент " + clientUsers[i] + " (сокет " +
                                std::to_string(clientSockets[i]) + ") отвалился до старта игры.");
                close(clientSockets[i]);
                clientSockets.erase(clientSockets.begin() + i);
                clientUsers.erase(clientUsers.begin() + i);
            } else {
                i++;
            }
        }
        // Если вдруг уже снова набралось 2, прерываем while
        if ((int)clientSockets.size() >= 2) {
            break;
        }

        // Принимаем нового клиента
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int newClientSock = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (newClientSock < 0) {
            printLog(logFile, "Ошибка accept");
            continue;
        }
        printLog(logFile, "Новый клиент пытается подключиться");

        // Авторизуем нового клиента
        std::string user = authorize(newClientSock, users, logFile);
        if (user.empty()) {
            // Авторизация провалена
            printLog(logFile, "Клиент отклонён: неверная авторизация");
            close(newClientSock);
            continue;
        }

        // Проверяем, нет ли уже такого user:
        bool alreadyPresent = false;
        for (const auto& existingUser : clientUsers) {
            if (existingUser == user) {
                alreadyPresent = true;
                break;
            }
        }
        if (alreadyPresent) {
            printLog(logFile, "Пользователь с именем " + user + " уже в игре. Отклоняем.");
            close(newClientSock);
            continue;
        }

        // Клиент успешно авторизован
        clientSockets.push_back(newClientSock);
        clientUsers.push_back(user);
        printLog(logFile, "Клиент " + user + " авторизован. Всего авторизовано: "
                  + std::to_string(clientSockets.size()));
    }

    printLog(logFile, "Оба игрока авторизованы. Начало игры.");
    int clientSocket1 = clientSockets[0];
    int clientSocket2 = clientSockets[1];

    // Инициализация поля
    std::string board = initBoard();

    // Определяем, кто ходит первым (случайно)
    std::srand(std::time(nullptr));
    bool firstPlayerTurn = (std::rand() % 2 == 0); 
    char currentPlayerChar = firstPlayerTurn ? 'X' : 'O';

    // Сообщаем клиентам, кто X и кто O
    // Чтобы было проще, скажем, что client1 = X, client2 = O, 
    // Но кто ходит первым — уже random:
    sendMessage(clientSocket1, "ROLE:X");
    sendMessage(clientSocket2, "ROLE:O");

    // Сообщим первому ходящему, что сейчас его ход
    if (currentPlayerChar == 'X') {
        sendMessage(clientSocket1, "TURN");
        sendMessage(clientSocket2, "WAIT");
    } else {
        sendMessage(clientSocket1, "WAIT");
        sendMessage(clientSocket2, "TURN");
    }

    // При начале хода:
    auto turnStartTime = std::chrono::steady_clock::now();

    // Основная логика игры:
    while (true) {
        int activeSock = (currentPlayerChar == 'X') ? clientSocket1 : clientSocket2;
        int passiveSock = (currentPlayerChar == 'X') ? clientSocket2 : clientSocket1;

        // Сколько времени осталось до истечения moveTime?
        // Эта проверка нужна, чтобы игрок не мог обнулять таймер, заслав некорретный ход
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - turnStartTime).count();
        if (elapsed >= moveTime) {
            // Слишком долго
            sendMessage(activeSock, "TIMEOUT_LOSE");
            sendMessage(passiveSock, "TIMEOUT_WIN");
            break;
        }

        // Сервер ждёт ход от активного игрока
        long msLeft = (moveTime - elapsed) * 1000;
        if (msLeft < 0) msLeft = 0;
        struct pollfd pfd;
        pfd.fd = activeSock;
        pfd.events = POLLIN;
        // Ждем не более чем moveTime секунд
        int pollRes = poll(&pfd, 1, moveTime * 1000);
        if (pollRes == 0) {
            // Таймаут
            printLog(logFile, "Игрок " + std::string(1, currentPlayerChar) + 
                              " не сделал ход за отведённое время. Проигрыш по таймауту.");

            // Сообщаем обоим игрокам:
            sendMessage(activeSock, "TIMEOUT_LOSE");
            sendMessage(passiveSock, "TIMEOUT_WIN");
            break;
        }
        if (pollRes < 0) {
            // Неизвестная ошибка
            printLog(logFile, "Ошибка poll при ожидании хода");
            break;
        }

        // Если дошли до сюда, новый ход был сделан за отведенное время. Читаем ход.
        std::string moveMsg = receiveMessage(activeSock);
        if (moveMsg.empty()) {
            // Соединение разорвано
            printLog(logFile, "Соединение с одним из клиентов потеряно. Завершение игры.");
            break;
        }

        // Проверяем, что ход был сделан вовремя:
        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - turnStartTime).count();
        if (elapsed >= moveTime) {
            // Слишком долго
            sendMessage(activeSock, "TIMEOUT_LOSE");
            sendMessage(passiveSock, "TIMEOUT_WIN");
            break;
        }

        // Предположим, что приходит что-то вроде "MOVE:3"
        if (moveMsg.rfind("MOVE:", 0) == 0) {
            std::string numStr = moveMsg.substr(5);
            int cellIndex = std::stoi(numStr);
            // Проверяем валидность хода
            if (canMakeMove(board, cellIndex)) {
                board = makeMove(board, cellIndex, currentPlayerChar);
                printLog(logFile, "Игрок " + std::string(1, currentPlayerChar) + 
                                  " сделал ход в ячейку " + std::to_string(cellIndex));

                // Проверяем состояние
                char state = checkWin(board);
                if (state == 'X' || state == 'O') {
                    // Победа
                    // Шлём обоим игрокам BOARD и RESULT
                    sendMessage(clientSocket1, "BOARD:" + board);
                    sendMessage(clientSocket2, "BOARD:" + board);

                    std::string resMsg = "WIN:" + std::string(1, state);
                    sendMessage(clientSocket1, resMsg);
                    sendMessage(clientSocket2, resMsg);
                    
                    printLog(logFile, "Игрок " + std::string(1, state) + " победил.");
                    break;
                } else if (state == 'D') {
                    // Ничья
                    sendMessage(clientSocket1, "BOARD:" + board);
                    sendMessage(clientSocket2, "BOARD:" + board);

                    sendMessage(clientSocket1, "DRAW");
                    sendMessage(clientSocket2, "DRAW");
                    
                    printLog(logFile, "Ничья.");
                    break;
                } else {
                    // Игра продолжается
                    // Обновляем доску на клиентах
                    sendMessage(clientSocket1, "BOARD:" + board);
                    sendMessage(clientSocket2, "BOARD:" + board);

                    // Передаём ход другому игроку
                    currentPlayerChar = (currentPlayerChar == 'X') ? 'O' : 'X';

                    if (currentPlayerChar == 'X') {
                        sendMessage(clientSocket1, "TURN");
                        sendMessage(clientSocket2, "WAIT");
                    } else {
                        sendMessage(clientSocket1, "WAIT");
                        sendMessage(clientSocket2, "TURN");
                    }
                    turnStartTime = std::chrono::steady_clock::now();
                }
            } else {
                // Невалидный ход
                sendMessage(activeSock, "INVALID");
                continue;
            }
        } else {
            // Неизвестная команда
            sendMessage(activeSock, "INVALID_CMD");
            continue;
        }
    }

    // Закрываем соединения
    close(clientSocket1);
    close(clientSocket2);
    close(serverSocket);
    printLog(logFile, "Сервер остановлен.");

    return 0;
}
