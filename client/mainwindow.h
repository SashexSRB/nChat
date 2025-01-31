#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QStackedWidget>
#include <QMessageBox>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleLogin();
    void handleRegister();
    void sendMessage();
    void onMessageReceived();
    void disconnectFromServer();
    void connectToServer(const QString &username, const QString &password);
    void showAuthUI();
    void showChatUI();


private:
    Ui::MainWindow *ui;

    QTcpSocket *socket;
    QString username;

    QStackedWidget *stackedWidget;
    QWidget *loginPage;
    QWidget *chatPage;

    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QPushButton *loginButton;
    QPushButton *registerButton;

    QTextEdit *chatHistory;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QPushButton *disconnectButton;
};
#endif // MAINWINDOW_H
