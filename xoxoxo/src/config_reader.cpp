#include "config_reader.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

std::map<std::string, std::string> readConfig(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть конфигурационный файл: " << filename << std::endl;
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Игнорируем пустые строки и строки-комментарии (#)
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Ищем формат "ключ = значение"
        std::istringstream iss(line);
        std::string key, eq, value;
        if (iss >> key >> eq >> value && eq == "=") {
            config[key] = value;
        }
    }

    return config;
}
