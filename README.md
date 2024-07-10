# Multithreaded TCP Server and Single-threaded Console Client

## Overview

This project implements a multithreaded TCP server and a single-threaded console client in C++ for handling JSON formatted requests. The server processes two types of requests:
1. `CheckLocalFile`: Searches for a signature in a specified file and returns a list of offsets.
2. `QuarantineLocalFile`: Moves a specified file to a quarantine directory.

## Project Structure

```
cpp-solution/
│
├── CMakeLists.txt          # CMake configuration file for building the project
├── server.cpp              # Source code for the server
├── client.cpp              # Source code for the client
├── config.cpp              # Source code for configuration handling
├── config.h                # Header file for the configuration module
├── server.cfg              # Configuration file (common for both server and client)
└── nlohmann/               # Directory for storing nlohmann's json.hpp
    └── json.hpp            # Header file for JSON library by nlohmann
```

## Features

- **Multithreaded TCP Server**: Uses a thread pool for handling multiple client connections concurrently.
- **Single-threaded Console Client**: Sends requests to the server and processes the responses.
- **Configuration File**: Server configuration is specified in an external configuration file (`server.cfg`).

## Prerequisites

- C++ Compiler: Support for C++20 standard.
- CMake: Version 3.20 or higher.

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/vvard3u/CPPNetJSON.git
    ```
2. Navigate to the project directory:
    ```sh
    cd CPPNetJSON
    ```

## Configuration

Create a `server.cfg` file in the project directory with the following content:
```ini
[DEFAULT]
host = 127.0.0.1
port = 56789
quarantine_dir = ./quarantine
threads = 4
buffer_size = 2048
```

## Building the Project

Using CMake:

1. Create a build directory:
    ```sh
    mkdir build
    cd build
    ```
2. Configure the project:
    ```sh
    cmake ..
    ```
3. Build the project:
    ```sh
    cmake --build .
    ```

These commands will create a build directory, configure the project using CMake, and build the `server.exe` and `client.exe` executable files.

## Running the Applications

After building, the executable files `server.exe` and `client.exe` will be located in the `build/Debug` directory (or `build/Release`, depending on the build mode).

### Running the Server

To start the server, run:
```sh
./server
```

### Running the Client

To send a request to the server, use the following command:
```sh
./client <command> [<parameters>]
```

#### Example Usage

1. Check a local file for a signature:
    ```sh
    ./client CheckLocalFile file_path=good_file.jpg signature=deadbeef
    ```
2. Quarantine a local file:
    ```sh
    ./client QuarantineLocalFile file_path=good_file.jpg
    ```

## Server Configuration

- `host`: The IP address the server listens on.
- `port`: The port the server listens on.
- `quarantine_dir`: The directory where quarantined files are moved.
- `threads`: The number of threads in the thread pool.
- `buffer_size`: Number of bytes allowed to transfer and receive.

## Client Commands

- `CheckLocalFile`: Checks a specified file for a given signature.
  - Parameters:
    - `file_path`: Path to the file to be checked.
    - `signature`: Hexadecimal string representing the signature to check for.
- `QuarantineLocalFile`: Moves a specified file to the quarantine directory.
  - Parameters:
    - `file_path`: Path to the file to be moved to quarantine.

## Error Handling

The server and client handle various errors and exceptions:
- Invalid JSON in requests
- Missing or invalid parameters
- File not found
- Connection errors

## License

This project is licensed under the MIT License. See the LICENSE file for details.
