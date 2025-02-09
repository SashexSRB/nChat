#ifndef EDITCONNECTIONDIALOG_H
#define EDITCONNECTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QFile>

class EditConnectionDialog : public QDialog {
    Q_OBJECT

public:
    EditConnectionDialog(QWidget *parent = nullptr);
    QString getIp() const;
    QString getPort() const;
    void loadConnectionData();

private:
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QPushButton *saveButton;

};

#endif // EDITCONNECTIONDIALOG_H
