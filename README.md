
# Multithreaded TCP Server and Single-threaded Console Client

This project implements a multithreaded TCP server and a single-threaded console client for sending JSON formatted requests to the server. The server handles two types of requests:
1. `CheckLocalFile`: Checks a specified file for a signature and returns a list of offsets where the signature is found.
2. `QuarantineLocalFile`: Moves a specified file to a quarantine directory.

## Features
- **Multithreaded TCP Server**: Uses a thread pool for handling multiple client connections concurrently.
- **Single-threaded Console Client**: Sends requests to the server and processes the responses.
- **Configuration File**: Server configuration is specified in an external configuration file (`server.cfg`).

## Prerequisites
- Python 3.6 or higher

## Installation
1. Clone the repository:
    ```sh
    git clone https://github.com/vvard3u/INT-3.git
    ```
2. Navigate to the project directory:
    ```sh
    cd INT-3
    ```

## Configuration
Change a `server.cfg` file in the project directory with the following content:
```ini
[DEFAULT]
host = 127.0.0.1
port = 56789
quarantine_dir = ./quarantine
threads = 4
buffer_size = 2048
```

## Running the Server
To start the server, run:
```sh
python server.py
```

## Running the Client
To send a request to the server, use the following command:
```sh
python client.py <command> [<parameters>]
```
### Example Usage
1. Check a local file for a signature:
    ```sh
    python client.py CheckLocalFile file_path=good_file.jpg signature=deadbeef
    ```
2. Quarantine a local file:
    ```sh
    python client.py QuarantineLocalFile file_path=good_file.jpg
    ```

## Server Configuration
- `host`: The IP address the server listens on.
- `port`: The port the server listens on.
- `quarantine_dir`: The directory where quarantined files are moved.
- `threads`: The number of threads in the thread pool.
- `buffer_size`: Number of bytes allowed to transfer and recieve

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

## Logging
The server and client use Python's `logging` module for logging information and errors. Logs are printed to the console.
