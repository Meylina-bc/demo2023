cmake_minimum_required(VERSION 3.10)

project(TicTacToeNetwork VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаём исполняемые файлы для сервера и клиента

# Список исходных файлов для сервера
add_executable(server
    src/server_main.cpp
    src/config_reader.cpp
    src/logger.cpp
    src/user_manager.cpp
    src/tic_tac_toe_logic.cpp
)

# Список исходных файлов для клиента
add_executable(client
    src/client_main.cpp
    src/config_reader.cpp
    src/logger.cpp
    src/user_manager.cpp
    src/tic_tac_toe_logic.cpp
)

# Указываем, где лежат заголовки
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Для Unix-систем нужно связать с библиотекой pthread (если используем потоки)
# и, возможно, с другими системными библиотеками для сокетов
# (в большинстве дистрибутивов Linux POSIX-сокеты есть "из коробки").

if(UNIX)
    target_link_libraries(server pthread)
    target_link_libraries(client pthread)
endif()
