#include "connect.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ConnectDialog::ConnectDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Connect to Server");
    
    auto layout = new QVBoxLayout(this);
    
    // Server list
    networkList = new QListWidget(this);
    networkList->addItems({"irc.libera.chat", "irc.freenode.net", "irc.rizon.net"});
    layout->addWidget(new QLabel("Server:"));
    layout->addWidget(networkList);
    
    // Nickname inputs
    auto nickLayout = new QVBoxLayout;
    nicknameInput = new QLineEdit(this);
    nickname2Input = new QLineEdit(this);
    nickname3Input = new QLineEdit(this);
    
    nickLayout->addWidget(new QLabel("Nickname:"));
    nickLayout->addWidget(nicknameInput);
    nickLayout->addWidget(new QLabel("Alternative nicknames:"));
    nickLayout->addWidget(nickname2Input);
    nickLayout->addWidget(nickname3Input);
    
    // Username input
    usernameInput = new QLineEdit(this);
    nickLayout->addWidget(new QLabel("Username:"));
    nickLayout->addWidget(usernameInput);
    
    layout->addLayout(nickLayout);
    
    // Buttons
    auto buttonBox = new QHBoxLayout;
    auto connectButton = new QPushButton("Connect", this);
    auto cancelButton = new QPushButton("Cancel", this);
    
    connect(connectButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonBox->addWidget(connectButton);
    buttonBox->addWidget(cancelButton);
    layout->addLayout(buttonBox);
    
    setupDefaultServers();
}

QString ConnectDialog::getServer() const {
    return networkList->currentItem() ? networkList->currentItem()->text() : QString();
}

QString ConnectDialog::getNickname() const {
    return nicknameInput->text();
}

QString ConnectDialog::getUsername() const {
    return usernameInput->text();
}

QStringList ConnectDialog::getAlternativeNicks() const {
    QStringList nicks;
    if (!nickname2Input->text().isEmpty()) nicks << nickname2Input->text();
    if (!nickname3Input->text().isEmpty()) nicks << nickname3Input->text();
    return nicks;
}

void ConnectDialog::setupDefaultServers() {
    networkList->setCurrentRow(0);  // Select first server by default
    
    // Make sure we have a valid server selected
    if (!networkList->currentItem()) {
        networkList->addItem("irc.libera.chat");
        networkList->setCurrentRow(0);
    }
    
    // Set default nickname if empty
    if (nicknameInput->text().isEmpty()) {
        nicknameInput->setText("Guest" + QString::number(qrand() % 1000));
    }
    
    // Set default username if empty
    if (usernameInput->text().isEmpty()) {
        usernameInput->setText(nicknameInput->text());
    }
}