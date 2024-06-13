#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "config.h"
#include "nlohmann/json.hpp"
#pragma comment(lib, "Ws2_32.lib")


using json = nlohmann::json;
int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Usage: %s command param1=value1 param2=value2... \n", argv[0]);
        printf("Example: CheckLocalFile file_path=<path_to_file> signature=<hex>\n");
        printf("Example: QuarantineLocalFile file_path=<path_to_file>\n");
        return 1;
    }

    std::string command = argv[1];
    std::map<std::string, std::string> params;
    for (int i = 2; i < argc; i++) {
        std::string param = argv[i];
        int pos = param.find("=");
        if (pos != std::string::npos) {
            std::string key = param.substr(0, pos);
            std::string value = param.substr(pos + 1);
            params[key] = value;
        }
        else {
            printf("Invalid parameter format: %s. Expected format key=value\n", param.c_str());
            return 1;
        }
    }

    if (!loadConfig("server.cfg")) {
        printf("Failed to load config file.\n");
        return 1;
    }
    const std::string DEFAULT_PORT = getConfigValue("DEFAULT_PORT", "56789");
    const std::string DEFAULT_ADDRESS = getConfigValue("DEFAULT_ADDRESS", "127.0.0.1");
    const int DEFAULT_BUFFLEN = getConfigValue("DEFAULT_BUFFLEN", 2048);

    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    printf("WinSock initialized sucessfully.\n");

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(DEFAULT_ADDRESS.c_str(), DEFAULT_PORT.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    // Attempt to connect to the first address returned by the call to getaddrinfo
    ptr = result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    // Connect to server.
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    // Free the resources
    // returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    json requestData;
    requestData["command"] = command;
    requestData["params"] = params;

    std::string requestStr = requestData.dump();
    std::vector<char> recvbuff(DEFAULT_BUFFLEN);

    iResult = send(ConnectSocket, requestStr.c_str(), (int)requestStr.length(), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    printf("Bytes Sent: %ld\n", iResult);


    // shutdown the connection for sending since no more data will be sent
    // the client can still use the ConnectSocket for receiving data
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // recieving answer from the server
    std::string responseStr;
    do {
        iResult = recv(ConnectSocket, recvbuff.data(), recvbuff.size(), 0);
        if (iResult > 0) {
            responseStr.append(recvbuff.data(), iResult);
            printf("Bytes received: %d\n", iResult);
        }
        else if (iResult == 0) {
            printf("Connection closed\n");
        }
        else {
            printf("recv failed: %d\n", WSAGetLastError());
        }
    } while (iResult > 0);

    // Print or process the entire response
    std::cout << "Response: " << responseStr << std::endl;


    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
