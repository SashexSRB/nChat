#include "userauth.h"
#include "../shared/messageHandler.h"
#include "clientTask.h"

#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <QDir>
#include <QThreadPool>

const int PORT = 1644;

std::vector<int> clientSockets;  // List of client socket descriptors
std::mutex mtx;  // Mutex for thread safety

// Structure to hold client information
struct ClientInfo {
    int socket;
    std::string username;
    bool isLoggedIn;

    ClientInfo(int sock, const std::string& user) : socket(sock), username(user), isLoggedIn(false) {}
};

std::vector<ClientInfo> clients;  // Stores connected clients

// Create the user file if it doesn't exist
void createUserFileIfNotExists() {
    std::ifstream infile("users.txt");
    if (!infile.good()) {
        std::ofstream outfile("users.txt");
        outfile.close();
    }
}

// Function to handle client communication
void handleClient(int clientSocket, MessageHandler &messageHandler) {
    char buffer[4096];
    ClientInfo client(clientSocket, "");  // Initialize client info with a blank username

    while (true) {
        memset(buffer, 0, 4096);
        int bytesRecv = recv(clientSocket, buffer, 4096, 0);

        if (bytesRecv <= 0) {
            std::cout << "Client disconnected!" << std::endl;
            break;
        }

        std::string message(buffer, 0, bytesRecv);

        // Handle registration and login commands
        if (message.rfind(std::to_string(MAGIC_REGISTER) + " ", 0) == 0) {
            std::string credentials = message.substr(std::to_string(MAGIC_REGISTER).length() + 1);
            size_t separatorPos = credentials.find(" ");

            if (separatorPos == std::string::npos) {
                std::string error = "Invalid REGISTER format.\n";
                send(clientSocket, error.c_str(), error.size(), 0);
                return;
            }

            std::string username = credentials.substr(0, separatorPos);
            std::string hashedPassword = credentials.substr(separatorPos + 1);

            if (registerUser(username, hashedPassword)) {
                std::string success = "Registration successful for user: " + username + "\n";
                send(clientSocket, success.c_str(), success.size(), 0);
            } else {
                std::string error = "Error during registration.\n";
                send(clientSocket, error.c_str(), error.size(), 0);
            }

        } else if (message.rfind(std::to_string(MAGIC_LOGIN) + " ", 0) == 0) {
            std::string credentials = message.substr(std::to_string(MAGIC_LOGIN).length() + 1);
            size_t separatorPos = credentials.find(" ");

            if (separatorPos == std::string::npos) {
                std::string error = "Invalid LOGIN format.\n";
                send(clientSocket, error.c_str(), error.size(), 0);
                return;
            }

            client.username = credentials.substr(0, separatorPos);
            std::string receivedHash = credentials.substr(separatorPos + 1);

            if (validateUser(client.username, receivedHash)) {
                std::string success = "Login successful! Welcome " + client.username + "\n";
                send(clientSocket, success.c_str(), success.size(), 0);

                client.isLoggedIn = true;  // Mark the user as logged in

                std::string loginMessage = client.username + " has logged in.";
                messageHandler.handleClientMessage(loginMessage, clientSockets, mtx);

                // Add the client to the list
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    clients.push_back(client);
                }

                messageHandler.sendChatHistoryToClients(clientSocket);
            } else {
                std::string error = "Invalid login credentials.\n";
                send(clientSocket, error.c_str(), error.size(), 0);
            }
        } else {
            // Check if client is logged in
            bool isClientLoggedIn = false;
            {
                std::lock_guard<std::mutex> lock(mtx);
                for (const auto& c : clients) {
                    if (c.socket == clientSocket && c.isLoggedIn) {
                        isClientLoggedIn = true;
                        break;
                    }
                }
            }

            if (!isClientLoggedIn) {
                std::string funnyMessage = "NERVChat SERVER RESPONSE: You really thought you could use the terminal to echo netcat into the server? Go fuck yourself and use the client.\n";  // Error message
                send(clientSocket, funnyMessage.c_str(), funnyMessage.size(), 0);

                std::cout << "Unauthorized access attempt. Closing socket." << std::endl;

                close(clientSocket);  // Close the socket
                return;
            }

            // Handle chat messages
            messageHandler.handleClientMessage(message, clientSockets, mtx);
        }
    }

    // Client disconnected, clean up
    close(clientSocket);  // Close the client socket

    {
        std::lock_guard<std::mutex> lock(mtx);

        // Remove client from the list by username
        auto it = std::remove_if(clients.begin(), clients.end(), [clientSocket](const ClientInfo &c) { return c.socket == clientSocket; });
        clients.erase(it, clients.end());

        // Also remove socket from clientSockets
        auto sockIt = std::remove(clientSockets.begin(), clientSockets.end(), clientSocket);
        clientSockets.erase(sockIt, clientSockets.end());
    }

    std::cout << "Client socket closed and removed from client list." << std::endl;
}

// Main server function
int main() {
    createUserFileIfNotExists();  // Ensure users.txt exists

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket!" << std::endl;
        return -1;
    }

    sockaddr_in serverHint;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(PORT);
    serverHint.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == -1) {
        std::cerr << "Error binding socket!" << std::endl;
        return -2;
    }

    if (listen(serverSocket, SOMAXCONN) == -1) {
        std::cerr << "Can't listen!" << std::endl;
        return -3;
    }

    std::cout << "Server is listening on port: " << PORT << std::endl;

    MessageHandler messageHandler;  // Create message handler instance
    messageHandler.loadChatHistory(clientSockets, mtx);

    QThreadPool::globalInstance()->setMaxThreadCount(100);

    while (true) {
        sockaddr_in client;
        socklen_t clientSize = sizeof(client);

        int clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);
        if (clientSocket == -1) {
            std::cerr << "Problem with client connecting!" << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            clientSockets.push_back(clientSocket);
        }

        ClientTask* task = new ClientTask(clientSocket, &messageHandler);
        QThreadPool::globalInstance()->start(task);  // Assign task to thread pool
    }

    close(serverSocket);  // Close the server socket

    return 0;
}
//TODO: Device fingerprinting to ensure multiple clients from same local network can use the server, and not just one. wiki: https://en.wikipedia.org/wiki/Fingerprint_(computing)
