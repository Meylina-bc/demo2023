#include "logger.hpp"

#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>

void writeLog(const std::string& logFile, const std::string& message) {
    log(logFile, message, false);
}

void printLog(const std::string& logFile, const std::string& message) {
    log(logFile, message, true);
}

void log(const std::string& logFile, const std::string& message, bool print) {
    std::ofstream file(logFile, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть лог-файл: " << logFile << std::endl;
        return;
    }

    // Получаем текущее время
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    // Форматируем временную метку [YYYY-MM-DD HH:MM:SS]
    file << std::put_time(now, "[%Y-%m-%d %H:%M:%S] ") << message << std::endl;
    file.close();

    if (print) {
        std::cout << message << std::endl;
    }
}