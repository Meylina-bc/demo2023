#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

/**
 * Запись строки в лог-файл с текущей временной меткой.
 */
void writeLog(const std::string& logFile, const std::string& message);

/**
 * Вывод сообщения на экран и запись строки в лог-файл.
 */
void printLog(const std::string& logFile, const std::string& message);

void log(const std::string& logFile, const std::string& message, bool print);

#endif
