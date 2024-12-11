#include "irc.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QDialog>

ComSock::ComSock(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

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

    messageDisplay = new QTextEdit(this);
    messageDisplay->setReadOnly(true);
    chatLayout->addWidget(messageDisplay);

    userList = new QListWidget(this);
    userList->setFixedWidth(100);
    userList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    chatLayout->addWidget(userList);

    mainLayout->addLayout(chatLayout);

    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message...");
    mainLayout->addWidget(messageInput);

    connect(messageInput, &QLineEdit::returnPressed, this, &ComSock::sendMessage);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ComSock::readMessage);
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

        QString chatMessage = QString("<%1> %2").arg(nicknameInput->text(), message);
        messageDisplay->append(chatMessage);

        QString ircMessage = QString("PRIVMSG %1 :%2\r\n").arg(currentChannel, message);
        socket->write(ircMessage.toUtf8());
        messageInput->clear();
    }
}

void ComSock::readMessage() {
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        qDebug() << "Server message:" << line;

        if (line.startsWith("PING")) {
            QString pingResponse = line.mid(5); 
            socket->write(QString("PONG %1\r\n").arg(pingResponse).toUtf8());
            continue;
        }

        // handle the NAMES reply (353)
        if (line.contains(" 353 ")) {
            QString userListData = line.section(':', 2);
            QStringList users = userListData.split(" ", Qt::SkipEmptyParts); 
            updateUserList(users);
            continue;
        }

        // handle other messages
        if (line.startsWith(":")) {
            QStringList parts = line.split(" :", Qt::SkipEmptyParts);
            if (parts.size() > 1) {
                QString senderInfo = parts[0];
                QString message = parts[1];

                QString senderNickname = senderInfo.split("!")[0].mid(1);
                messageDisplay->append(QString("<%1> %2").arg(senderNickname, message));
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
    dialogLayout->addWidget(serverInput);

    nicknameInput = new QLineEdit(dialog);
    nicknameInput->setPlaceholderText("Nickname");
    dialogLayout->addWidget(nicknameInput);

    QPushButton *connectButton = new QPushButton("Connect", dialog);
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
