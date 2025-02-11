#ifndef CLIENTTASK_H
#define CLIENTTASK_H

#include <QRunnable>
#include <QThreadPool>
#include "messageHandler.h"

extern void handleClient(int clientSocket, MessageHandler &messageHandler);

class ClientTask : public QRunnable {
    public:
        ClientTask(int clientSocket, MessageHandler* handler)
                : socket(clientSocket), messageHandler(handler) {}

        void run() override {
            handleClient(socket, *messageHandler);
        }
    private:
        int socket;
        MessageHandler* messageHandler;
};

#endif // CLIENTTASK_H
