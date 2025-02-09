#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTextStream>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QStackedWidget>
#include <QMessageBox>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QDir>

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
    void connectToServer(const QString &message);
    void writeConnection(const std::string &ip, const std::string &port);
    void editConnection();
    void showAuthUI();
    void showChatUI();


private:
    Ui::MainWindow *ui;

    QTcpSocket *socket;
    QString username;

    QStackedWidget *stackedWidget;
    QWidget *pgLogin;
    QWidget *pgChat;

    QLineEdit *inputUname;
    QLineEdit *inputPass;
    QPushButton *btnLogin;
    QPushButton *btnRegister;

    QTextEdit *textChatHistory;
    QLineEdit *inputMessage;
    QPushButton *btnSend;
    QAction *actionDisconnect;
    QAction *actionEditConnection;
    QMenu *menuOptions;
};
#endif // MAINWINDOW_H
