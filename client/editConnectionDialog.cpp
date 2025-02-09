#include "editConnectionDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

EditConnectionDialog::EditConnectionDialog(QWidget *parent)
    : QDialog(parent), ipLineEdit(new QLineEdit(this)), portLineEdit(new QLineEdit(this)), saveButton(new QPushButton("Save", this)) {

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("IP Address:", ipLineEdit);
    formLayout->addRow("Port:", portLineEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(saveButton);

    setLayout(mainLayout);
    setWindowTitle("Edit Connection");

    connect(saveButton, &QPushButton::clicked, this, [this]() {
        if (ipLineEdit->text().isEmpty() || portLineEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Invalid Input", "Please enter both IP address and port.");
            return;
        }
        accept();
    });
    loadConnectionData();
}

QString EditConnectionDialog::getIp() const {
    return ipLineEdit->text();
}

QString EditConnectionDialog::getPort() const {
    return portLineEdit->text();
}

void EditConnectionDialog::loadConnectionData() {
    QFile file("connection.conf");

    // Check if the file exists
    if (!file.exists()) {
        QMessageBox::warning(this, "File Not Found", "connection.conf not found.");
        return;
    }

    // Open the file for reading
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        QString ip = in.readLine();  // First line for IP address
        QString port = in.readLine();  // Second line for port

        ipLineEdit->setText(ip);  // Set the IP field
        portLineEdit->setText(port);  // Set the port field

        file.close();
    } else {
        QMessageBox::warning(this, "Error", "Could not read connection.conf.");
    }
}
