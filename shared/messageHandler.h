#ifndef MESSAGEHANDLER_H
    #define MESSAGEHANDLER_H

    #include <string>
    #include <vector>
    #include <mutex>

    class MessageHandler {
    public:
        MessageHandler();
        void loadChatHistory(std::vector<int> &clientSockets, std::mutex &mtx); // Updated declaration
        void sendChatHistoryToClients(int clientSocket);
        void saveMessage(const std::string &message);
        void handleClientMessage(const std::string &message, int clientSocket, std::vector<int> &clientSockets, std::mutex &mtx);
        void broadcastMessage(const std::string &message, int senderSocket, const std::vector<int> &clientSockets, std::mutex &mtx);

    private:
        const std::string chatHistoryFile = "chat_history.txt";

    };
#endif
