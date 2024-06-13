cmake_minimum_required(VERSION 3.20)
project(MyProject)

# Устанавливаем стандарт языка C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем исполняемые файлы
add_executable(server server.cpp config.cpp)
add_executable(client client.cpp config.cpp)

# Копируем server.cfg в директорию сборки для обоих исполняемых файлов
configure_file(server.cfg ${CMAKE_CURRENT_BINARY_DIR}/Debug/server.cfg COPYONLY)
configure_file(server.cfg ${CMAKE_CURRENT_BINARY_DIR}/Release/server.cfg COPYONLY)
