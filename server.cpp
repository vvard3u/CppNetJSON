#include "config.h"
#include "nlohmann/json.hpp"

#pragma comment(lib, "Ws2_32.lib")

using json = nlohmann::json;
bool serverRunning = true;
SOCKET ListenSocket = INVALID_SOCKET;

void signalHandler(int signum);
void cleanup();
std::string CheckLocalFile(std::map<std::string, std::string> params);
std::string QuarantineLocalFile(std::map<std::string, std::string> params);
std::vector<uint8_t> hexToBytes(const std::string& hex);
DWORD WINAPI clientHandler(LPVOID lpParam);
std::vector<HANDLE> threadHandles;
std::vector<SOCKET> clientSockets;

int main() {
    // handling SIGINT signal
    signal(SIGINT, signalHandler);

    if (!loadConfig("server.cfg")) {
        printf("Failed to load config file.\n");
        return 1;
    }

    WSADATA wsaData;
    int iResult;

    const std::string DEFAULT_PORT = getConfigValue("DEFAULT_PORT", "56789");
    const std::string DEFAULT_ADDRESS = getConfigValue("DEFAULT_ADDRESS", "127.0.0.1");
    const int DEFAULT_BUFFLEN = getConfigValue("DEFAULT_BUFFLEN", 2048);
    const int DEFAULT_THREADS = getConfigValue("DEFAULT_THREADS", 5);
    const std::string DEFAULT_QUARANTINE_DIR = getConfigValue("DEFAULT_QUARANTINE_DIR", "quarantine");
    printf("Config loaded successfully.\n");

    printf("Listening address %s\n", DEFAULT_ADDRESS.c_str());
    printf("Listening port %s\n", DEFAULT_PORT.c_str());

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    printf("WinSock initialized successfully.\n");

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(DEFAULT_ADDRESS.c_str(), DEFAULT_PORT.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while (serverRunning) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);

        if (ClientSocket == INVALID_SOCKET) {
            if (serverRunning) {
                printf("accept failed: %d\n", WSAGetLastError());
            }
            break;
        }
        if (threadHandles.size() >= DEFAULT_THREADS) {
            closesocket(ClientSocket);
            continue;
        }

        clientSockets.push_back(ClientSocket);

        DWORD threadId;
        HANDLE threadHandle = CreateThread(
            NULL,              // default security attributes
            0,                 // default stack size
            clientHandler,     // thread function
            (LPVOID)ClientSocket, // parameter to thread function
            0,                 // default creation flags
            &threadId);        // returns the thread identifier

        if (threadHandle == NULL) {
            printf("CreateThread failed: %d\n", GetLastError());
            closesocket(ClientSocket);
        }
        else {
            threadHandles.push_back(threadHandle);
        }
    }

    cleanup();
    return 0;
}

DWORD WINAPI clientHandler(LPVOID lpParam) {
    SOCKET ClientSocket = (SOCKET)lpParam;
    int DEFAULT_BUFFLEN = getConfigValue("DEFAULT_BUFFLEN", 2048);
    std::vector<char> recvbuff(DEFAULT_BUFFLEN);
    int iResult, iSendResult;
    json requestData;
    std::string requestStr;
    std::string command;
    std::map<std::string, std::string> params;
    json responseData;
    std::string responseStr;

    do {
        iResult = recv(ClientSocket, recvbuff.data(), recvbuff.size(), 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
            requestStr.append(recvbuff.data(), iResult);
            try {
                requestData = json::parse(requestStr);
                printf("Parsed request: %s\n", requestStr.c_str());
                command = requestData["command"];
                params = requestData["params"];

                if (command == "CheckLocalFile") responseData = json::parse(CheckLocalFile(params));
                else if (command == "QuarantineLocalFile") responseData = json::parse(QuarantineLocalFile(params));
                else responseData = json::parse("{\"error\": \"unknown command\"}");

                responseStr = responseData.dump();
                iSendResult = send(ClientSocket, responseStr.c_str(), responseStr.length(), 0);
                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed: %d\n", WSAGetLastError());
                    break;
                }
                printf("Bytes sent: %d\n", iSendResult);
            }
            catch (const std::exception& e) {
                printf("JSON parse error: %s\n", e.what());
            }
        }
        else if (iResult == 0) {
            printf("Connection closing...\n");
            break;
        }
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            break;
        }
    } while (iResult > 0);

    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
    }
    closesocket(ClientSocket);
    return 0;
}

void signalHandler(int signum) {
    printf("Interrupt signal (%d) received.\n", signum);
    serverRunning = false;
    cleanup();
    closesocket(ListenSocket);
    exit(signum);
}

void cleanup() {
    for (SOCKET s : clientSockets) {
        closesocket(s);
    }
    for (HANDLE h : threadHandles) {
        WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
    }
    WSACleanup();
    printf("Cleanup completed.\n");
}

std::string CheckLocalFile(std::map<std::string, std::string> params) {
    std::string filePath = params["file_path"];
    std::string signature = params["signature"];
    std::vector<uint8_t> bytes = hexToBytes(signature);
    std::vector<int> offsets;
    std::string response;
    std::ifstream file(filePath, std::ios::binary);


    if (filePath.empty()) {
        response = "{\"error\": \"file path is empty\"}";
        return response;
    }
    if (std::filesystem::is_directory(filePath)) {
        response = "{\"error\": \"file is a directory\"}";
        return response;
    }
    if (!std::filesystem::exists(filePath)) {
        response = "{\"error\": \"file not found\"}";
        return response;
    }

    std::vector<uint8_t> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    int offset = 0;
    while ((offset = std::search(content.begin() + offset, content.end(), bytes.begin(), bytes.end()) - content.begin()) != content.size()) {
        offsets.push_back(offset);
        offset += 1;
    }

    response = "{\"offsets\": [";
    for (int i = 0; i < offsets.size(); ++i) {
        response += std::to_string(offsets[i]);
        if (i != offsets.size() - 1) {
            response += ", ";
        }
    }
    response += "]}";
    return response;
}

std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (int i = 0; i < hex.length(); i += 2) {
        uint8_t byte = static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string QuarantineLocalFile(std::map<std::string, std::string> params) {
    std::string filePath = params["file_path"];
    const std::string DEFAULT_QUARANTINE_DIR = getConfigValue("DEFAULT_QUARANTINE_DIR", "quarantine");
    std::string response;

    std::cout << "File path: " << filePath << std::endl;
    std::cout << "Quarantine directory: " << DEFAULT_QUARANTINE_DIR << std::endl;

    if (filePath.empty()) {
        response = "{\"error\": \"file path is empty\"}";
        return response;
    }
    if (std::filesystem::is_directory(filePath)) {
        response = "{\"error\": \"file is a directory\"}";
        return response;
    }
    if (!std::filesystem::exists(filePath)) {
        response = "{\"error\": \"file not found\"}";
        return response;
    }

    // Creating quarantine directory
    std::filesystem::create_directories(DEFAULT_QUARANTINE_DIR);
    std::filesystem::path newFilePath = std::filesystem::path(DEFAULT_QUARANTINE_DIR) / std::filesystem::path(filePath).filename();
    std::filesystem::path oldFilePath = std::filesystem::path(filePath);

    std::cout << "New file path: " << newFilePath << std::endl;
    std::cout << "Old file path: " << oldFilePath << std::endl;

    // Use MoveFile to move the file and capture any errors
    if (MoveFileA(oldFilePath.string().c_str(), newFilePath.string().c_str()) == 0) {
        DWORD error = GetLastError();
        std::cout << "MoveFile failed with error: " << error << std::endl;
        response = "{\"error\": \"failed to quarantine file, error code: " + std::to_string(error) + "\"}";
    }
    else {
        response = "{\"success\": \"file moved to quarantine successfully\"}";
    }

    return response;
}