cmake_minimum_required(VERSION 3.20)
project(MyProject)

# C++ 20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executables setting
add_executable(server server.cpp config.cpp)
add_executable(client client.cpp config.cpp)

# Copying server.cfg to directories 
configure_file(server.cfg ${CMAKE_CURRENT_BINARY_DIR}/Debug/server.cfg COPYONLY)
configure_file(server.cfg ${CMAKE_CURRENT_BINARY_DIR}/Release/server.cfg COPYONLY)
