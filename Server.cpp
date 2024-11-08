#define _WINSOCKAPI_
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <winsock2.h>
#include "Server.h"
#pragma comment(lib, "Ws2_32.lib")
#define PORT 4444                              // Global defintion of listening port for Server
#define BUFFER_SIZE 12900                       // Global definition of buffer size for Server

// Server main socket function
int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Set up the server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Start listening for client connections
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Start accepting clients in a separate thread without printing the startup message
    std::thread clientThread(acceptClients, serverSocket);

    // Run the main menu without waiting for clients to connect
    mainMenu();

    // Detach the clientThread so it continues accepting connections in the background
    clientThread.detach();

    // Close the server socket and clean up Winsock
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
