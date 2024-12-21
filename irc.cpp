#include "irc.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDialog>
#include <QFontDatabase>
#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QInputDialog>

ComSock::ComSock(QWidget *parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout(this);
    
    auto fontId = QFontDatabase::addApplicationFont("./TerminusTTF-4.49.3.ttf");
    auto boldFontId = QFontDatabase::addApplicationFont("./TerminusTTF-Bold-4.49.3.ttf");
    auto fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    auto boldFontFamily = QFontDatabase::applicationFontFamilies(boldFontId).at(0);
    
    qApp->setStyleSheet(QString(
        "* { color: #FFFFFF; background-color: #2B2B2B; selection-background-color: #555555; "
        "selection-color: #FFFFFF; border: 1px solid #555555; font-family: '%1'; font-size: 14px; } "
        "QMenuBar { background-color: #3C3C3C; } "
        "QMenuBar::item { background: transparent; } "
        "QMenuBar::item:selected { background: #555555; } "
        "QMenu { background-color: #3C3C3C; } "
        "QMenu::item:selected { background-color: #555555; } "
        "QPushButton { background-color: #444444; } "
        "QPushButton:pressed { background-color: #666666; } "
        "QLineEdit, QTextEdit, QListWidget { background-color: #3C3C3C; } "
    ).arg(fontFamily));

    menuBar = new QMenuBar(this);
    auto fileMenu = menuBar->addMenu("ComSock");
    auto helpMenu = menuBar->addMenu("Help");
    connect(fileMenu->addAction("Connect"), &QAction::triggered, this, &ComSock::connectDialog);
    connect(fileMenu->addAction("Exit"), &QAction::triggered, this, &QWidget::close);
    connect(helpMenu->addAction("About"), &QAction::triggered, this, &ComSock::about);
    mainLayout->setMenuBar(menuBar);

    // Main Layout
    auto chatLayout = new QHBoxLayout();
    
    channelList = new QListWidget(this);
    channelList->setFixedWidth(100);
    
    auto messageLayout = new QVBoxLayout();
    messageStack = new QStackedWidget(this);
    messageDisplay = new QTextEdit(this);
    messageDisplay->setReadOnly(true);
    messageDisplay->setFont(QFont(fontFamily, 14));
    
    messageStack->addWidget(messageDisplay);
    channelDisplays[""] = messageDisplay;
    
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message...");
    messageInput->setFont(QFont(fontFamily, 14));
    
    userList = new QListWidget(this);
    userList->setFixedWidth(100);
    userList->setFont(QFont(boldFontFamily, 14));
    
    messageLayout->addWidget(messageStack);
    messageLayout->addWidget(messageInput);
    
    chatLayout->addWidget(channelList);
    chatLayout->addLayout(messageLayout);
    chatLayout->addWidget(userList);
    
    mainLayout->addLayout(chatLayout);

    // set connections
    socket = new QTcpSocket(this);
    connect(messageInput, &QLineEdit::returnPressed, this, &ComSock::sendMessage);
    connect(socket, &QTcpSocket::readyRead, this, &ComSock::readMessage);
    connect(channelList, &QListWidget::itemClicked, this, &ComSock::switchChannel);
}

void ComSock::connectToServer() {
    if (!socket || socket->state() == QAbstractSocket::ConnectedState || 
        socket->state() == QAbstractSocket::ConnectingState) return;

    socket->connectToHost(serverInput->text(), 6667);
    if (!socket->waitForConnected(5000)) {
        QMessageBox::critical(this, "Connection Error", "Could not connect to server.");
        return;
    }

    auto nickname = nicknameInput->text();
    // send initial nickname and alternatives if the first one fails
    socket->write(QString("NICK %1\r\nUSER %2 0 * :My IRC Client\r\n")
                 .arg(nickname)
                 .arg(username.isEmpty() ? nickname : username)
                 .toUtf8());
}

void ComSock::sendMessage() {
    auto message = messageInput->text();
    if (message.isEmpty()) return;
    
    if (message.startsWith("/")) {
        auto parts = message.split(" ");
        if (parts[0] == "/join" && parts.size() > 1) {
            currentChannel = parts[1];
            if (!channelList->findItems(currentChannel, Qt::MatchExactly).isEmpty()) {
                messageDisplay->append("Already in channel: " + currentChannel);
            } else {
                channelList->addItem(currentChannel);
                auto channelTextEdit = new QTextEdit(this);
                channelTextEdit->setReadOnly(true);
                channelTextEdit->setFont(messageDisplay->font());
                channelDisplays[currentChannel] = channelTextEdit;
                messageStack->addWidget(channelTextEdit);
                messageStack->setCurrentWidget(channelTextEdit);
                socket->write(QString("JOIN %1\r\n").arg(currentChannel).toUtf8());
            }
        } else if (parts[0] == "/nick" && parts.size() > 1) {
            socket->write(QString("NICK %1\r\n").arg(parts[1]).toUtf8());
        } else {
            messageDisplay->append("Unknown command: " + message);
        }
    } else if (socket->state() == QAbstractSocket::ConnectedState && !currentChannel.isEmpty()) {
        if (auto display = channelDisplays[currentChannel]) {
            display->append(QString("<%1> %2").arg(nicknameInput->text(), message));
            socket->write(QString("PRIVMSG %1 :%2\r\n").arg(currentChannel, message).toUtf8());
        }
    } else {
        messageDisplay->append(socket->state() != QAbstractSocket::ConnectedState ? 
            "Not connected to server." : "You must join a channel before sending messages.");
    }
    
    messageInput->clear();
}

void ComSock::readMessage() {
    while (socket->canReadLine()) {
        auto line = QString::fromUtf8(socket->readLine()).trimmed();
        messageDisplay->append(line);
        if (line.startsWith("PING")) {
            socket->write(QString("PONG %1\r\n").arg(line.mid(5)).toUtf8());
        } else if (line.contains(" 353 ")) {
            updateUserList(line.section(':', 2).split(" ", Qt::SkipEmptyParts));
        } else if (line.startsWith(":")) {
            auto parts = line.split(" :", Qt::SkipEmptyParts);
            if (parts.size() > 1 && !currentChannel.isEmpty() && 
                line.contains(QString("PRIVMSG %1").arg(currentChannel))) {
                if (auto display = channelDisplays[currentChannel]) {
                    display->append(QString("<%1> %2")
                        .arg(parts[0].split("!")[0].mid(1), parts[1]));
                }
            }
        }
    }
}

void ComSock::connectDialog() {
    auto dialog = new QDialog(this);
    dialog->setWindowTitle("Connect to IRC Server");
    auto layout = new QVBoxLayout(dialog);
    auto userGroup = new QGroupBox("User Information", dialog);
    auto userLayout = new QVBoxLayout(userGroup);
    nicknameInput = new QLineEdit(dialog);
    auto nickname2Input = new QLineEdit(dialog);
    auto nickname3Input = new QLineEdit(dialog);
    auto usernameInput = new QLineEdit(dialog);
    nicknameInput->setPlaceholderText("Nickname");
    nickname2Input->setPlaceholderText("Second choice");
    nickname3Input->setPlaceholderText("Third choice");
    usernameInput->setPlaceholderText("User name");
    userLayout->addWidget(nicknameInput);
    userLayout->addWidget(nickname2Input);
    userLayout->addWidget(nickname3Input);
    userLayout->addWidget(usernameInput);
    // networks group
    auto networkGroup = new QGroupBox("Networks", dialog);
    auto networkLayout = new QVBoxLayout(networkGroup);
    networkList = new QListWidget(dialog);
    networkList->addItem("irc.rizon.net");
    auto buttonLayout = new QHBoxLayout();
    auto addButton = new QPushButton("Add", dialog);
    auto removeButton = new QPushButton("Remove", dialog);
    auto editButton = new QPushButton("Edit...", dialog);
    auto sortButton = new QPushButton("Sort", dialog);
    auto favorButton = new QPushButton("Favor", dialog);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(sortButton);
    buttonLayout->addWidget(favorButton);
    networkLayout->addWidget(networkList);
    networkLayout->addLayout(buttonLayout);
    // skip network list checkbox
    auto skipNetworkCheck = new QCheckBox("Skip network list on startup", dialog);
    auto showFavoritesCheck = new QCheckBox("Show favorites only", dialog);
    
    auto connectButton = new QPushButton("Connect", dialog);
    auto closeButton = new QPushButton("Close", dialog);
    auto bottomButtonLayout = new QHBoxLayout();
    bottomButtonLayout->addWidget(closeButton);
    bottomButtonLayout->addWidget(connectButton);
    
    layout->addWidget(userGroup);
    layout->addWidget(networkGroup);
    layout->addWidget(skipNetworkCheck);
    layout->addWidget(showFavoritesCheck);
    layout->addLayout(bottomButtonLayout);
    
    // connect signals
    connect(addButton, &QPushButton::clicked, [this]() {
        bool ok;
        QString network = QInputDialog::getText(this, "Add Network",
                                              "Enter network address:",
                                              QLineEdit::Normal, "", &ok);
        if (ok && !network.isEmpty()) {
            networkList->addItem(network);
        }
    });
    
    connect(removeButton, &QPushButton::clicked, [this]() {
        delete networkList->currentItem();
    });
    
    connect(editButton, &QPushButton::clicked, [this]() {
        auto item = networkList->currentItem();
        if (!item) return;
        
        bool ok;
        QString network = QInputDialog::getText(this, "Edit Network",
                                              "Edit network address:",
                                              QLineEdit::Normal,
                                              item->text(), &ok);
        if (ok && !network.isEmpty()) {
            item->setText(network);
        }
    });
    
    connect(sortButton, &QPushButton::clicked, [this]() {
        networkList->sortItems();
    });
    
    connect(connectButton, &QPushButton::clicked, [this, dialog, nickname2Input, nickname3Input, usernameInput]() {
        alternativeNicks.clear();
        alternativeNicks << nickname2Input->text() << nickname3Input->text();
        username = usernameInput->text();
        
        // make sure network is selected
        if (auto item = networkList->currentItem()) {
            socket->connectToHost(item->text(), 6667);
            if (!socket->waitForConnected(5000)) {
                QMessageBox::critical(this, "Connection Error", "Could not connect to server.");
                return;
            }

            auto nickname = nicknameInput->text();
            socket->write(QString("NICK %1\r\nUSER %2 0 * :My IRC Client\r\n")
                        .arg(nickname)
                        .arg(username.isEmpty() ? nickname : username)
                        .toUtf8());
            
            dialog->accept();
        } else {
            QMessageBox::warning(this, "Connection Error", "Please select a server from the list.");
        }
    });
    
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::reject);
    
    dialog->exec();
}

void ComSock::switchChannel(QListWidgetItem *item) {
    if (channelDisplays.contains(item->text())) {
        currentChannel = item->text();
        messageStack->setCurrentWidget(channelDisplays[currentChannel]);
        updateUserListForChannel(currentChannel);
    }
}

void ComSock::updateUserList(const QStringList &users) {
    userList->clear();
    userList->addItems(users);
}

void ComSock::updateUserListForChannel(const QString &channel) {
    userList->clear();
    socket->write(QString("NAMES %1\r\n").arg(channel).toUtf8());
}
