#include "irc.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include <QFontDatabase>
#include <QApplication>
#include <QMap>

ComSock::ComSock(QWidget *parent) : QWidget(parent) {
    mainLayout = new QVBoxLayout(this);

    int fontId = QFontDatabase::addApplicationFont("./TerminusTTF-4.49.3.ttf");
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont mainFont(fontFamily, 14);

    int boldFontId = QFontDatabase::addApplicationFont("./TerminusTTF-Bold-4.49.3.ttf");
    QString boldFontFamily = QFontDatabase::applicationFontFamilies(boldFontId).at(0);
    QFont boldFont(boldFontFamily, 14);

    // Set global stylesheet for dark theme and font
    qApp->setStyleSheet(QString(
        "* { "
        "    font-family: '%1'; "
        "    font-size: 18px; "
        "    color: #FFFFFF; "
        "    background-color: #2B2B2B; "
        "    selection-background-color: #555555; "
        "    selection-color: #FFFFFF; "
        "    border: 1px solid #555555; "
        "} "
        "QMenuBar { "
        "    font-family: '%1'; "
        "    font-weight: bold; "
        "    font-size: 18px; "
        "    background-color: #3C3C3C; "
        "} "
        "QListWidget { "
        "   font-family: '%1'; "
        "   font-weight: bold; "
        "} "
        "QMenuBar { background-color: #3C3C3C; } "
        "QMenuBar::item { background: transparent; } "
        "QMenuBar::item:selected { background: #555555; } "
        "QMenu { background-color: #3C3C3C; } "
        "QMenu::item:selected { background-color: #555555; } "
        "QPushButton { background-color: #444444; } "
        "QPushButton:pressed { background-color: #666666; } "
        "QLineEdit { background-color: #3C3C3C; } "
        "QTextEdit { background-color: #3C3C3C; } "
        "QListWidget { background-color: #3C3C3C; } "
    ).arg(fontFamily));

    menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu("ComSock");
    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *connectAction = fileMenu->addAction("Connect");
    connect(connectAction, &QAction::triggered, this, &ComSock::connectDialog);
    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    QAction *aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &ComSock::about);
    mainLayout->setMenuBar(menuBar);

    QHBoxLayout *chatLayout = new QHBoxLayout();
    
    channelList = new QListWidget(this);
    channelList->setFixedWidth(100);
    chatLayout->addWidget(channelList);
    
    QVBoxLayout *messageLayout = new QVBoxLayout();

    messageStack = new QStackedWidget(this);
    messageDisplay = new QTextEdit(this);
    messageDisplay->setReadOnly(true);
    messageDisplay->setFont(mainFont);

    messageStack->addWidget(messageDisplay);
    messageLayout->addWidget(messageStack);

    channelDisplays[""] = messageDisplay;

    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message...");
    messageInput->setFont(mainFont);
    messageLayout->addWidget(messageInput);
    chatLayout->addLayout(messageLayout);

    userList = new QListWidget(this);
    userList->setFixedWidth(100);
    userList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    userList->setFont(boldFont);
    chatLayout->addWidget(userList);

    mainLayout->addLayout(chatLayout);
    connect(messageInput, &QLineEdit::returnPressed, this, &ComSock::sendMessage);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ComSock::readMessage);

    connect(channelList, &QListWidget::itemClicked, this, &ComSock::switchChannel);

    setLayout(mainLayout);
}

void ComSock::connectToServer() {
    QString server = serverInput->text();
    if (!socket) {
        qDebug() << "Socket is not initialized.";
        return;
    }
    if (socket->state() == QAbstractSocket::ConnectedState || 
        socket->state() == QAbstractSocket::ConnectingState) {
        qDebug() << "Already connected or connecting.";
        return;
    }

    socket->connectToHost(server, 6667);
    if (!socket->waitForConnected(5000)) {
        qDebug() << "Connection failed:" << socket->errorString();
        QMessageBox::critical(this, "Connection Error", "Could not connect to server.");
        return;
    }

    QString nickname = nicknameInput->text();
    socket->write(QString("NICK %1\r\n").arg(nickname).toUtf8());
    socket->write(QString("USER %1 0 * :My IRC Client\r\n").arg(nickname).toUtf8());
}

void ComSock::sendMessage() {
    QString message = messageInput->text();
    if (message.startsWith("/")) {
        QStringList commandParts = message.split(" ");
        QString command = commandParts[0];

        if (command == "/join" && commandParts.size() > 1) {
            QString channel = commandParts[1];
            currentChannel = channel;

            if (!channelList->findItems(channel, Qt::MatchExactly).isEmpty()) {
                messageDisplay->append("Already in channel: " + channel);
                messageInput->clear();
                return;
            }
            channelList->addItem(channel);

            QTextEdit *channelTextEdit = new QTextEdit(this);
            channelTextEdit->setReadOnly(true);
            channelTextEdit->setFont(messageDisplay->font());
            channelTextEdit->setStyleSheet(messageDisplay->styleSheet());

            channelDisplays[channel] = channelTextEdit;
            messageStack->addWidget(channelTextEdit);

            messageStack->setCurrentWidget(channelTextEdit);

            socket->write(QString("JOIN %1\r\n").arg(channel).toUtf8());
            messageInput->clear();
            return;
        } else if (command == "/nick" && commandParts.size() > 1) {
            QString nickname = commandParts[1];
            socket->write(QString("NICK %1\r\n").arg(nickname).toUtf8());
            messageInput->clear();
            return;
        } else {
            messageDisplay->append("Unknown command: " + message);
            messageInput->clear();
            return;
        }
    }

    if (!message.isEmpty()) {
        if (socket->state() != QAbstractSocket::ConnectedState) {
            messageDisplay->append("Not connected to server.");
            return;
        }
        if (currentChannel.isEmpty()) {
            messageDisplay->append("You must join a channel before sending messages.");
            return;
        }
        
        QTextEdit *currentDisplay = channelDisplays[currentChannel];
        if (currentDisplay) {
            QString chatMessage = QString("<%1> %2").arg(nicknameInput->text(), message);
            currentDisplay->append(chatMessage);
        }
        
        QString ircMessage = QString("PRIVMSG %1 :%2\r\n").arg(currentChannel, message);
        socket->write(ircMessage.toUtf8());
        messageInput->clear();
    }
}

void ComSock::readMessage() {
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        qDebug() << "Server message:" << line;

        messageDisplay->append(line);

        if (line.startsWith("PING")) {
            QString pingResponse = line.mid(5); 
            socket->write(QString("PONG %1\r\n").arg(pingResponse).toUtf8());
            continue;
        }
        if (line.contains(" 353 ")) {
            QString userListData = line.section(':', 2);
            QStringList users = userListData.split(" ", Qt::SkipEmptyParts); 
            updateUserList(users);
            continue;
        }
        if (line.startsWith(":")) {
            QStringList parts = line.split(" :", Qt::SkipEmptyParts);
            if (parts.size() > 1) {
                QString senderInfo = parts[0];
                QString message = parts[1];
                QString senderNickname = senderInfo.split("!")[0].mid(1);
                
                if (!currentChannel.isEmpty() && line.contains(QString("PRIVMSG %1").arg(currentChannel))) {
                    QTextEdit *currentDisplay = channelDisplays[currentChannel];
                    if (currentDisplay) {
                        currentDisplay->append(QString("<%1> %2").arg(senderNickname, message));
                    }
                }
            }
        }
    }
}

void ComSock::updateUserList(const QStringList &users) {
    userList->clear();
    userList->addItems(users);
}

void ComSock::connectDialog() {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Connect to IRC Server");
    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
    serverInput = new QLineEdit(dialog);
    serverInput->setPlaceholderText("Server Address");
    serverInput->setFont(QFont(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont("./TerminusTTF-Bold-4.49.3.ttf")).at(0), 13));
    dialogLayout->addWidget(serverInput);
    nicknameInput = new QLineEdit(dialog);
    nicknameInput->setPlaceholderText("Nickname");
    nicknameInput->setFont(QFont(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont("./TerminusTTF-Bold-4.49.3.ttf")).at(0), 13));
    dialogLayout->addWidget(nicknameInput);
    QPushButton *connectButton = new QPushButton("Connect", dialog);
    connectButton->setFont(QFont(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont("./TerminusTTF-Bold-4.49.3.ttf")).at(0), 13));
    dialogLayout->addWidget(connectButton);
    connect(connectButton, &QPushButton::clicked, [this, dialog]() {
        connectToServer();
        dialog->accept();
    });

    dialog->setLayout(dialogLayout);
    dialog->exec();
}

void ComSock::about() {
    QMessageBox::about(this, "About ComSock", "This is a simple IRC client.");
}

void ComSock::switchChannel(QListWidgetItem *item) {
    QString channel = item->text();
    if (channelDisplays.contains(channel)) {
        currentChannel = channel;
        messageStack->setCurrentWidget(channelDisplays[channel]);
        
        updateUserListForChannel(channel);
    }
}

void ComSock::updateUserListForChannel(const QString &channel) {
    userList->clear();

    QString request = QString("NAMES %1\r\n").arg(channel);
    socket->write(request.toUtf8());
}
