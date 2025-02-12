#include "messageHandler.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>

#include <sys/socket.h>

MessageHandler::MessageHandler() {
    // Constructor can be used for any initialization if needed
}

void MessageHandler::sendChatHistoryToClients(int clientSocket) {
    std::ifstream file(chatHistoryFile);

    if(!file.is_open()) {
        std::cerr << "Could not open chat history file for reading." << std::endl;

        return;
    }

    std::string line;

    while(std::getline(file, line)) {
        send(clientSocket, line.c_str(), line.size(), 0); //send each line to client
        send(clientSocket, "\n", 1, 0); //send newline character
    }

    file.close();
}

// Function to load chat history from the file when the server starts
void MessageHandler::loadChatHistory(std::vector<int> &clientSockets, std::mutex &mtx) { // Match the declaration
    std::ifstream file(chatHistoryFile);

    if (!file.is_open()) {
        std::cerr << "Could not open chat history file for reading." << std::endl;

        return;
    }

    std::string line;

    while (std::getline(file, line)) {
        // Broadcast the loaded message to all clients
        broadcastMessage(line, clientSockets, mtx);
    }

    file.close();
}

// Function to save a chat message to the file
void MessageHandler::saveMessage(const std::string &message) {
    std::ofstream file(chatHistoryFile, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Could not open chat history file for writing." << std::endl;

        return;
    }

    file << message << std::endl; // Write the message to the file

    file.close();
}

// Function to broadcast messages to all connected clients
void MessageHandler::broadcastMessage(const std::string &message, const std::vector<int> &clientSockets, std::mutex &mtx) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Broadcasting message: " << message << std::endl;
    for (int clientSocket : clientSockets) {
        int bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
        if (bytesSent == -1) {
            std::cerr << "Error sending message to client socket: " << clientSocket << std::endl;
        } else {
            std::cout << "Message sent to client: " << clientSocket << std::endl;
            send(clientSocket, "\n", 1, 0);  // Ensure newline termination for the client
        }
    }
}

// Function to handle client communication messages
void MessageHandler::handleClientMessage(const std::string &message, std::vector<int> &clientSockets, std::mutex &mtx) {
    std::string chatMessage = message; // Assume message is already formatted

    //std::cout << "Received: " << chatMessage << std::endl;

    broadcastMessage(chatMessage, clientSockets, mtx); // Broadcast the message with username
    saveMessage(chatMessage); // Save chat message to file
}
