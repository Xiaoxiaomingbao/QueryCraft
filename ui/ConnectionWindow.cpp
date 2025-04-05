#include <QVBoxLayout>
#include <QFormLayout>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

#include "connectionwindow.h"

ConnectionWindow::ConnectionWindow(QWidget *parent)
    : QMainWindow(parent), db(QSqlDatabase::addDatabase("QMYSQL")) {

    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    auto *formLayout = new QFormLayout();

    hostLineEdit = new QLineEdit(this);
    portLineEdit = new QLineEdit(this);
    userLineEdit = new QLineEdit(this);
    passwordLineEdit = new QLineEdit(this);
    schemaLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("IP 地址:", hostLineEdit);
    formLayout->addRow("端口:", portLineEdit);
    formLayout->addRow("用户名:", userLineEdit);
    formLayout->addRow("密码:", passwordLineEdit);
    formLayout->addRow("数据库:", schemaLineEdit);

    const auto connectButton = new QPushButton("连接", this);
    const auto disconnectButton = new QPushButton("断开", this);
    statusLabel = new QLabel("未连接", this);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(connectButton);
    mainLayout->addWidget(disconnectButton);
    mainLayout->addWidget(statusLabel);

    connect(connectButton, &QPushButton::clicked, this, &ConnectionWindow::connectToDatabase);
    connect(disconnectButton, &QPushButton::clicked, this, &ConnectionWindow::disconnectDatabase);
}

ConnectionWindow::~ConnectionWindow() {
    if (db.isOpen()) {
        db.close();
    }
}

void ConnectionWindow::connectToDatabase() {
    if (db.isOpen()) {
        statusLabel->setText("已连接");
        return;
    }

    db.setHostName(hostLineEdit->text());
    db.setPort(portLineEdit->text().toInt());
    db.setUserName(userLineEdit->text());
    db.setPassword(passwordLineEdit->text());
    db.setDatabaseName(schemaLineEdit->text());

    if (db.open()) {
        statusLabel->setText("连接成功");
        emit connectionSuccess(db, schemaLineEdit->text());
    } else {
        statusLabel->setText("连接失败");
        qDebug() << "Error:" << db.lastError().text();
    }
}

void ConnectionWindow::disconnectDatabase() {
    if (db.isOpen()) {
        db.close();
        statusLabel->setText("已断开");
    }
}
