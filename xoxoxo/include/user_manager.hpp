#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include <string>
#include <vector>

struct User {
    std::string username;
    std::string password;
};

/**
 * Загрузка пользователей из файла (логин и пароль).
 */
std::vector<User> loadUsers(const std::string& filename);

/**
 * Проверка, есть ли пользователь с таким логином и паролем в списке.
 */
bool checkCredentials(const std::vector<User>& users,
                      const std::string& username,
                      const std::string& password);

#endif
