#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "userauth.h"
#include "editConnectionDialog.h"
#include <fstream>
#include <iostream>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(nullptr) {
    ui->setupUi(this);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    pgLogin = ui->pgLogin;
    connect(ui->btnLogin, &QPushButton::clicked, this, &MainWindow::handleLogin);
    connect(ui->btnRegister, &QPushButton::clicked, this, &MainWindow::handleRegister);

    pgChat = ui->pgChat;
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectFromServer);
    connect(ui->actionEditConnection, &QAction::triggered, this, &MainWindow::editConnection);

    stackedWidget->addWidget(pgLogin);
    stackedWidget->addWidget(pgChat);

    stackedWidget->setCurrentWidget(pgLogin);
    ui->actionDisconnect->setEnabled(false);

    QString configFilePath = QDir::currentPath() + "/connection.conf";
    std::ifstream infile(configFilePath.toStdString());

    if (!infile.good()) {
        std::ofstream outfile(configFilePath.toStdString());
        std::cout << "connection.conf created successfully in " << configFilePath.toStdString() << std::endl;
        outfile.close();
    }
}

MainWindow::~MainWindow() {
    delete ui;
    delete socket;
}

void MainWindow::handleLogin() {
    disconnectFromServer();
    QString username = ui->inputUname->text();  // Access widgets directly from UI
    QString password = ui->inputPass->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login", "Please enter both username and password.");
        return;
    }

    std::string hashedPassword = hashPassword(password.toStdString());
    QString loginRequest = QString("%1 %2 %3\n")
                               .arg(MAGIC_LOGIN)
                               .arg(username)
                               .arg(QString::fromStdString(hashedPassword));

    // Attempt to connect to server
    connectToServer(loginRequest);
}

// Handle register action
void MainWindow::handleRegister() {
    QString username = ui->inputUname->text();
    QString password = ui->inputPass->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Register", "Please enter both username and password.");
        return;
    }

    std::string hashedPassword = hashPassword(password.toStdString());

    QString registerRequest = QString("%1 %2 %3\n")
                                  .arg(MAGIC_REGISTER)
                                  .arg(username)
                                  .arg(QString::fromStdString(hashedPassword));

    // Attempt to register with the server
    QTcpSocket tempSocket;
    tempSocket.connectToHost("127.0.0.1", 1644);

    if (tempSocket.waitForConnected(3000)) {
        tempSocket.write(registerRequest.toUtf8());
        tempSocket.waitForBytesWritten();
        tempSocket.waitForReadyRead();

        QString response = tempSocket.readAll();
        QMessageBox::information(this, "Registration", response);
    } else {
        QMessageBox::warning(this, "Connection Error", "Unable to connect to the server.");
    }
}

void MainWindow::writeConnection(const std::string &ip, const std::string &port) {
    std::ofstream file("connection.conf", std::ios::trunc);

    if (!file) {
        std::cerr << "Error opening connection.conf for writing." << std::endl;
        return;
    }

    file << ip << std::endl << port;
}

void MainWindow::editConnection() {
    EditConnectionDialog dialog(this);

    if(dialog.exec() == QDialog::Accepted) {
        QString ip = dialog.getIp();
        QString port = dialog.getPort();

        writeConnection(ip.toStdString(), port.toStdString());
    }
}

// Connect to the server
void MainWindow::connectToServer(const QString &message) {
    if (socket) {
        disconnectFromServer();
    }

    QString serverIP;
    quint16 serverPort;

    QFile file("connection.conf");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        serverIP = in.readLine().trimmed();
        serverPort = in.readLine().trimmed().toUShort();
        file.close();
    } else {
        QMessageBox::warning(this, "Error", "Failed to open connection config file. Using defaults.");
        serverIP = "127.0.0.1";
        serverPort = 1644;
    }

    socket = new QTcpSocket(this);

    // Connect to the server
    connect(socket, &QTcpSocket::connected, this, [this, message]() {
        qDebug() << "Socket connected";
        socket->write(message.toUtf8());
        socket->waitForBytesWritten();
    });

    connect(socket, &QTcpSocket::readyRead, this, [this]() {
        QString response = QString::fromUtf8(socket->readAll()).trimmed();
        std::string normalResString = response.toStdString();
        std::cout << normalResString << std::endl;

        if (this->username.isEmpty()) {
            if (response.contains("Login successful")) {
                qDebug() << "Login successful!";
                int welcomePos = response.indexOf("Welcome");
                if (welcomePos != -1) {
                    this->username = response.mid(welcomePos + 8).trimmed();
                }
                showChatUI();
            } else {
                QMessageBox::warning(this, "Login Failed", response);
            }
        } else {
            ui->txtChatHistory->append(response.trimmed());
        }
    });

    // Error handling
    connect(socket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError error) {
        qDebug() << "Socket error occurred: " << error;
    });

    // Handle disconnection
    connect(socket, &QTcpSocket::disconnected, this, []() {
        qDebug() << "Socket disconnected";
    });

    // Start connection
    qDebug() << "Connecting to server...";
    socket->connectToHost(serverIP, serverPort);
}


// Disconnect from the server
void MainWindow::disconnectFromServer() {
    if (socket) {
        socket->disconnectFromHost();
        socket->close();
        socket->deleteLater();
        socket = nullptr;
    }
    ui->txtChatHistory->clear();
    this->username.clear();
    showAuthUI();  // Show login page after logging out
}

// Show the chat UI (switch to chat page)
void MainWindow::showChatUI() {
    stackedWidget->setCurrentWidget(pgChat);  // Switch to chat page after successful login
    ui->actionDisconnect->setEnabled(true);
    ui->actionEditConnection->setEnabled(false);
}

// Show the authentication UI (switch to login page)
void MainWindow::showAuthUI() {
    stackedWidget->setCurrentWidget(pgLogin);  // Switch to login page
    ui->actionDisconnect->setEnabled(false);
    ui->actionEditConnection->setEnabled(true);
}

// Send a message
void MainWindow::sendMessage() {
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        QString message = ui->inputMessage->text();  // Access directly from UI
        if(message.isEmpty()) return;

        QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy 'at' HH:mm ");
        QString formattedMessage = QString("%1 - %2: %3").arg(username, dateTime, message);
        qDebug() << formattedMessage;

        socket->write(formattedMessage.toUtf8());
        ui->inputMessage->clear();  // Access directly from UI
    } else {
        QMessageBox::warning(this, "Connection", "Not connected to server!");
    }
}
