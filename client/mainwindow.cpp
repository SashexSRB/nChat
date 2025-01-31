#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(nullptr) {
    ui->setupUi(this);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    loginPage = ui->loginPage;
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::handleLogin);
    connect(ui->registerButton, &QPushButton::clicked, this, &MainWindow::handleRegister);

    chatPage = ui->chatPage;
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(ui->disconnectButton, &QPushButton::clicked, this, &MainWindow::disconnectFromServer);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onMessageReceived);

    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(chatPage);

    stackedWidget->setCurrentWidget(loginPage);
}

MainWindow::~MainWindow() {
    delete ui;
    delete socket;
}

void MainWindow::handleLogin() {
    QString username = ui->usernameInput->text();  // Access widgets directly from UI
    QString password = ui->passwordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login", "Please enter both username and password.");
        return;
    }

    // Attempt to connect to server
    connectToServer(username, password);
}

// Handle register action
void MainWindow::handleRegister() {
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Register", "Please enter both username and password.");
        return;
    }

    // Attempt to register with the server
    QTcpSocket tempSocket;
    tempSocket.connectToHost("127.0.0.1", 1644);

    if (tempSocket.waitForConnected(3000)) {
        QString registerRequest = "REGISTER " + username + " " + password + "\n";
        tempSocket.write(registerRequest.toUtf8());
        tempSocket.waitForBytesWritten();
        tempSocket.waitForReadyRead();

        QString response = tempSocket.readAll();
        QMessageBox::information(this, "Registration", response);
    } else {
        QMessageBox::warning(this, "Connection Error", "Unable to connect to the server.");
    }
}

// Connect to the server
void MainWindow::connectToServer(const QString &username, const QString &password) {
    if (socket) {
        disconnectFromServer();
    }

    socket = new QTcpSocket(this);
    socket->connectToHost("127.0.0.1", 1644);

    if (socket->waitForConnected(3000)) {
        QString credentials = "LOGIN " + username + " " + password + "\n";
        socket->write(credentials.toUtf8());
        socket->waitForBytesWritten();
        socket->waitForReadyRead();

        QString response = socket->readAll();
        //qDebug() << "Server response after login: " << response;

        if (response.contains("Login successful")) {
            this->username = username;
            showChatUI();

            //qDebug() << "Chat history should be loaded?";
            QTimer::singleShot(100, this, [this](){
                while(socket->canReadLine()) {
                    QByteArray message = socket->readLine();
                    //qDebug() << "Received chat history line: " << message;
                    ui->chatHistory->append(QString::fromUtf8(message.trimmed()));
                }
            });
        } else {
            QMessageBox::warning(this, "Login Failed", response);
        }
    } else {
        QMessageBox::warning(this, "Connection Error", "Unable to connect to the server.");
    }
}

// Show the chat UI (switch to chat page)
void MainWindow::showChatUI() {
    stackedWidget->setCurrentWidget(chatPage);  // Switch to chat page after successful login
}

// Disconnect from the server
void MainWindow::disconnectFromServer() {
    if (socket) {
        socket->disconnectFromHost();
        socket->close();
        socket->deleteLater();
        socket = nullptr;
    }
    ui->chatHistory->clear();
    showAuthUI();  // Show login page after logging out
}

// Show the authentication UI (switch to login page)
void MainWindow::showAuthUI() {
    stackedWidget->setCurrentWidget(loginPage);  // Switch to login page
}

// Handle received messages
void MainWindow::onMessageReceived() {
    while (socket && socket->canReadLine()) {
        QString message = QString::fromUtf8(socket->readLine()).trimmed();
        ui->chatHistory->append(message);  // Access directly from UI
    }
}

// Send a message
void MainWindow::sendMessage() {
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        QString message = ui->messageInput->text();  // Access directly from UI
        QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy 'at' HH:mm ");
        QString formattedMessage = QString("%1 - %2: %3").arg(username, dateTime, message);

        socket->write(formattedMessage.toUtf8());
        ui->chatHistory->append(formattedMessage);  // Access directly from UI
        ui->messageInput->clear();  // Access directly from UI
    } else {
        QMessageBox::warning(this, "Connection", "Not connected to server!");
    }
}
