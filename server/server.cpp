#include "userauth.h"

#include "../shared/messageHandler.h"

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
#include <thread>
#include <mutex>
#include <fstream>
#include <algorithm>

const int PORT = 1644;

std::vector<int> clientSockets; // Stores client socket descriptors

std::mutex mtx; // Mutex for thread safety

// Structure to hold client information
struct ClientInfo {
    int socket;
    std::string username;
    bool isLoggedIn;

    ClientInfo(int sock) : socket(sock), isLoggedIn(false) {}
};

// Vector to hold client information
std::vector<ClientInfo> clients;

// Function to create `users.txt` file if it doesn't exist
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

    ClientInfo client(clientSocket); // Initialize client info

    while (true) {
        memset(buffer, 0, 4096);

        int bytesRecv = recv(clientSocket, buffer, 4096, 0);

        if (bytesRecv <= 0) {
            std::cout << "Client disconnected!" << std::endl;

            break;
        }

        std::string message(buffer, 0, bytesRecv);

        // Check for login and registration commands
        if (message.rfind("REGISTER ", 0) == 0) {  // Register command
            std::string credentials = message.substr(9);
            size_t separatorPos = credentials.find(" ");
            std::string username = credentials.substr(0, separatorPos);
            std::string password = credentials.substr(separatorPos + 1);

            if (registerUser(username, password)) {
                std::string success = "Registration successful for user: " + username + "\n";

                send(clientSocket, success.c_str(), success.size(), 0);
            } else {
                std::string error = "Error during registration.\n";

                send(clientSocket, error.c_str(), error.size(), 0);
            }
        } else if (message.rfind("LOGIN ", 0) == 0) {  // Login command
            std::string credentials = message.substr(6);
            size_t separatorPos = credentials.find(" ");
            client.username = credentials.substr(0, separatorPos);
            std::string password = credentials.substr(separatorPos + 1);

            if (validateUser(client.username, password)) {
                std::string success = "Login successful! Welcome " + client.username + "\n";

                send(clientSocket, success.c_str(), success.size(), 0);

                client.isLoggedIn = true; // Mark the user as logged in

                std::string loginMessage = client.username + " has logged in.\n";

                messageHandler.broadcastMessage(loginMessage, clientSocket, clientSockets, mtx);

                // Add client to the list of clients
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
            // Check if the client is logged in
            bool isClientLoggedIn = false;

            {
                std::lock_guard<std::mutex> lock(mtx);

                for (const auto &c : clients) {
                    if (c.socket == clientSocket && c.isLoggedIn) {
                        isClientLoggedIn = true;
                        break;
                    }
                }
            }

            if (!isClientLoggedIn) {
                std::string funnyMessage = "Unauthorized access attempt.\n"; // Error response

                send(clientSocket, funnyMessage.c_str(), funnyMessage.size(), 0);

                std::cout << "Unauthorized access attempt. Sending warning message." << std::endl;

                close(clientSocket); // Close the connection

                return; // Exit the function, freeing resources
            }

            // Handle chat message
            messageHandler.handleClientMessage(message, clientSocket, clientSockets, mtx); // Use MessageHandler
        }
    }

    close(clientSocket);

    // Safely remove client from list
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto it = std::remove_if(clients.begin(), clients.end(), [clientSocket](const ClientInfo &c) { return c.socket == clientSocket; });

        clients.erase(it, clients.end());
    }
}

int main() {
    createUserFileIfNotExists(); // Ensure `users.txt` exists

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

    MessageHandler messageHandler; // Create MessageHandler instance
    messageHandler.loadChatHistory(clientSockets, mtx);

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

        std::thread(handleClient, clientSocket, std::ref(messageHandler)).detach();
    }

    close(serverSocket);

    return 0;
}
