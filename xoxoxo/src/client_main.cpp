#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

#include "config_reader.hpp"
#include "logger.hpp"
#include "user_manager.hpp" // опционально

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

bool sendMessage(int sock, const std::string& message) {
    std::string msg = message + "\n";
    int sent = send(sock, msg.c_str(), msg.size(), 0);
    return (sent == (int)msg.size());
}

// Функция, в которой мы безопасно считываем ход
int readMoveIndex() {
    while (true) {
        std::string line;
        if (!std::getline(std::cin, line)) {
            // Если вдруг EOF или ошибка ввода — можно вернуть признак
            // или кидать исключение, чтобы завершать программу
            std::cerr << "Ошибка ввода или EOF. Завершение...\n";
            exit(1);
        }
        try {
            // Пытаемся преобразовать к числу
            int value = std::stoi(line);
            // Дополнительно проверяем диапазон
            if (value < 0 || value > 8) {
                std::cout << "Введите число от 0 до 8: ";
                continue;
            }
            return value;  // Если всё хорошо, выходим из цикла
        }
        catch (const std::invalid_argument& ) {
            std::cout << "Некорректный ввод. Попробуйте снова: ";
        }
        catch (const std::out_of_range&) {
            std::cout << "Число слишком большое/маленькое. Попробуйте снова: ";
        }
    }
}

int main(int argc, char* argv[]) {
    // По умолчанию используем "client_config.txt"
    std::string configFile = "client_config.txt";
    if (argc > 1) {
        configFile = argv[1];
    }
    // Считываем конфигурационный файл
    auto config = readConfig(configFile);
    
    // Получаем необходимые параметры
    std::string serverAddress = "127.0.0.1";
    if (config.find("serverAddress") != config.end()) {
        serverAddress = config["serverAddress"];
    }
    int port = 5555;
    if (config.find("port") != config.end()) {
        port = std::stoi(config["port"]);
    }
    std::string username = "MyUser";
    if (config.find("username") != config.end()) {
        username = config["username"];
    }
    std::string password = "MyPassword";
    if (config.find("password") != config.end()) {
        password = config["password"];
    }
    std::string logFile = "game.log";
    if (config.find("logFile") != config.end()) {
        logFile = config["logFile"];
    }

    // Разделяем логи от прошлого запуска
    writeLog(logFile, "");
    writeLog(logFile, "------");

    // Подключаемся к серверу
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printLog(logFile, "Ошибка создания сокета");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr) <= 0) {
        printLog(logFile, "Неверный адрес сервера");
        close(sock);
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printLog(logFile, "Ошибка подключения к серверу");
        close(sock);
        return 1;
    }

    printLog(logFile, "Подключен к серверу " + serverAddress + ":" + std::to_string(port));

    // Отправляем логин и пароль
    sendMessage(sock, username);
    sendMessage(sock, password);

    // Получаем ответ
    std::string authResponse = receiveMessage(sock);
    writeLog(logFile, "Ответ авторизации: " + authResponse);
    if (authResponse == "FAIL") {
        std::cout << "Авторизация не удалась" << std::endl;
        close(sock);
        return 0;
    } else if (authResponse == "OK") {
        std::cout << "Авторизация успешна" << std::endl;
    } else {
        std::cout << "Неизвестный ответ при авторизации" << std::endl;
        close(sock);
        return 0;
    }

    // Игра
    char myRole = '_'; // X или O
    bool running = true;

    while (running) {
        std::string msg = receiveMessage(sock);
        if (msg.empty()) {
            printLog(logFile, "Соединение прервано");
            break;
        }

        // Обрабатываем сообщения
        if (msg.rfind("ROLE:", 0) == 0) {
            // Пример: ROLE:X
            myRole = msg[5];
            std::cout << "Ваша роль: " << myRole << std::endl;
        }
        else if (msg == "TURN") {
            // Наш ход
            std::cout << "Ваш ход! Введите номер клетки (0..8): ";
            int cellIndex = readMoveIndex();
            std::string moveCmd = "MOVE:" + std::to_string(cellIndex);
            sendMessage(sock, moveCmd);
        }
        else if (msg == "WAIT") {
            // Ждем
            std::cout << "Ожидаем ход соперника..." << std::endl;
        }
        else if (msg.rfind("BOARD:", 0) == 0) {
            // Обновление доски: BOARD:_________
            std::string boardState = msg.substr(6);
            // Выводим в консоль
            std::cout << "Текущее состояние доски:" << std::endl;
            for (int i = 0; i < 9; i++) {
                std::cout << boardState[i];
                if (i % 3 == 2) {
                    std::cout << std::endl;
                } else {
                    std::cout << ' ';
                }
            }
        }
        else if (msg.rfind("WIN:", 0) == 0) {
            char winner = msg[4];
            if (winner == myRole) {
                std::cout << "Вы выиграли!" << std::endl;
            } else {
                std::cout << "Вы проиграли. Победил игрок " << winner << std::endl;
            }
            writeLog(logFile, "Клиент: Получено сообщение о победе " + std::string(1, winner));
            running = false;
        }
        else if (msg == "DRAW") {
            std::cout << "Ничья!" << std::endl;
            writeLog(logFile, "Игра завершена: ничья");
            running = false;
        }
        else if (msg.rfind("TIMEOUT_LOSE", 0) == 0) {
            std::cout << "Вы проиграли по таймауту!" << std::endl;
            writeLog(logFile, "Игра завершена: проигрыш по таймауту");
            running = false;
        }
        else if (msg.rfind("TIMEOUT_WIN", 0) == 0) {
            std::cout << "Ваш соперник не уложился по времени. Вы победили!" << std::endl;
            writeLog(logFile, "Игра завершена: выигрыш по таймауту");
            running = false;
        }
        else if (msg == "INVALID") {
            std::cout << "Неверный ход. Попробуйте снова (0..8): " << std::endl;
            int cellIndex = readMoveIndex();
            std::string moveCmd = "MOVE:" + std::to_string(cellIndex);
            sendMessage(sock, moveCmd);
        }
        else if (msg == "PING") {
            sendMessage(sock, "PONG");
            continue;
        }
        else {
            std::cout << "Внутренняя ошибка программы." << std::endl;
            writeLog(logFile, "Непредвиденное сообщение: " + msg);
            running = false;
        }
        std::cout.flush();
    }

    close(sock);
    writeLog(logFile, "Работа завершена");
    return 0;
}
