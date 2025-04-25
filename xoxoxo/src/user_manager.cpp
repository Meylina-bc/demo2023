#include "user_manager.hpp"

#include <fstream>
#include <iostream>

std::vector<User> loadUsers(const std::string& filename) {
    std::vector<User> users;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл пользователей: " << filename << std::endl;
        return users;
    }

    std::string username, password;
    while (file >> username >> password) {
        if (!username.empty() && !password.empty()) {
            users.push_back({username, password});
        }
    }

    return users;
}

bool checkCredentials(const std::vector<User>& users,
                      const std::string& username,
                      const std::string& password) {
    for (const auto& user : users) {
        if (user.username == username && user.password == password) {
            return true;
        }
    }
    return false;
}
