# Создать поддиректорию сборки
mkdir build
cd build

# Запустить CMake
cmake ..

# Собрать проект
make

# Перейти в корень проекта
cd ..

# Запустить сервер
./build/server ./examples/server_config.txt

# Во втором окошке запустить первый клиент:
./build/client ./examples/client1_config.txt

# В третьем окошке запустить второй клиент:
./build/client ./examples/client2_config.txt
