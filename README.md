### Project: Example Client-Server Application in C++

This project is a simple example of a client-server application written in C++, utilizing sockets for data exchange. It consists of two executable files: `server.exe` and `client.exe`.

### Project Structure

```
cpp-solution/
│
├── CMakeLists.txt          # CMake configuration file for building the project
├── server.cpp              # Source code for the server
├── client.cpp              # Source code for the client
├── config.cpp              # Source code for configuration handling
├── config.h                # Header file for configuration module
├── server.cfg              # Configuration file (common for both server and client)
└── nlohmann/               # Directory for storing nlohmann's json.hpp
    └── json.hpp            # Header file for JSON library by nlohmann
```

### Requirements

1. **C++ Compiler**: Support for C++20 standard.
2. **CMake**: Version 3.20 or higher.

### Building the Project

1. **Using CMake:**

   In the root directory of the project, execute the following commands:

   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

   These commands will create a `build` directory, configure the project using CMake, and build the `server.exe` and `client.exe` executable files.

2. **Running the Applications**: After building, the executable files `server.exe` and `client.exe` will be located in the `build/Debug` directory (or `build/Release`, depending on the build mode).

### Configuration File `server.cfg`

The `server.cfg` file serves as the configuration file for both `server.exe` and `client.exe`. It should reside in the root directory of the project and contain necessary configuration parameters in the format expected by your application.

