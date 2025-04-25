#ifndef CONFIG_READER_HPP
#define CONFIG_READER_HPP

#include <string>
#include <map>

/**
 * Функция чтения конфигурационного файла.
 * Возвращает map<ключ, значение> с параметрами.
 */
std::map<std::string, std::string> readConfig(const std::string& filename);

#endif
