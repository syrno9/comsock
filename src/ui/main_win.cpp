#include "main_win.h"
#include "dialogs/connect.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("ComSock");
    resize(800, 600);

    // Initialize all pointers first
    ircClient = new IrcClient(this);
    channelList = new ChannelList(this);
    userList = new UserList(this);
    channelTabs = new QTabWidget(this);
    messageInput = new QLineEdit(this);
    nickDisplay = new QLabel(this);
    
    // Setup UI
    setupMenuBar();
    setupLayout();
    
    // Connect signals
    connect(ircClient, &IrcClient::messageReceived, this, &MainWindow::handleMessageReceived);
    connect(ircClient, &IrcClient::userJoined, this, &MainWindow::handleUserJoined);
    connect(ircClient, &IrcClient::userLeft, this, &MainWindow::handleUserLeft);
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    connect(channelTabs, &QTabWidget::currentChanged, this, &MainWindow::handleTabChanged);
    
    // Show connect dialog on startup
    QTimer::singleShot(0, this, &MainWindow::showConnectDialog);
}

void MainWindow::setupMenuBar() {
    auto menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    auto fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Connect"), this, &MainWindow::showConnectDialog);
    fileMenu->addAction(tr("&Disconnect"), this, &MainWindow::handleDisconnect);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), qApp, &QApplication::quit);
    
    auto helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
}

void MainWindow::setupLayout() {
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto mainLayout = new QVBoxLayout(centralWidget);
    auto splitter = new QSplitter(Qt::Horizontal);
    
    // Left side - Channel list
    splitter->addWidget(channelList);
    
    // Middle - Chat area
    auto chatWidget = new QWidget;
    auto chatLayout = new QVBoxLayout(chatWidget);
    
    chatLayout->addWidget(channelTabs);
    chatLayout->addWidget(messageInput);
    
    splitter->addWidget(chatWidget);
    
    // Right side - User list
    splitter->addWidget(userList);
    
    // Set splitter sizes
    splitter->setStretchFactor(0, 1);  // Channel list
    splitter->setStretchFactor(1, 4);  // Chat area
    splitter->setStretchFactor(2, 1);  // User list
    
    // Add nickname display at the top
    auto topLayout = new QHBoxLayout;
    topLayout->addWidget(new QLabel(tr("Nickname:")));
    topLayout->addWidget(nickDisplay);
    topLayout->addStretch();
    
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(splitter);
}

void MainWindow::showConnectDialog() {
    auto dialog = new ConnectDialog(this);
    if (dialog->exec() == QDialog::Accepted) {
        // Disconnect old signal connections if any
        disconnect(ircClient, &IrcClient::connected, nullptr, nullptr);
        
        QString nickname = dialog->getNickname();
        QString username = dialog->getUsername();
        QString server = dialog->getServer();
        
        if (nickname.isEmpty() || server.isEmpty()) {
            QMessageBox::warning(this, "Invalid Input", 
                               "Please enter a nickname and select a server.");
            dialog->deleteLater();
            return;
        }
        
        // Create default channel tab before connecting
        createChannelTab("#test");
        
        // Set up client before connecting
        ircClient->setNickname(nickname);
        ircClient->setUsername(username);
        
        // Connect signals
        connect(ircClient, &IrcClient::connected, this, [this]() {
            qDebug() << "Successfully registered with server, joining channel...";
            ircClient->joinChannel("#test");
        });
        
        connect(ircClient, &IrcClient::error, this, [this](const QString& error) {
            QMessageBox::critical(this, "Connection Error", 
                                "Failed to connect: " + error);
        });
        
        // Update UI
        nickDisplay->setText(nickname);
        
        // Connect to server
        qDebug() << "Connecting to server:" << server;
        ircClient->connectToServer(server);
    }
    dialog->deleteLater();
}

void MainWindow::handleMessageReceived(const IrcMessage& message) {
    // Create channel tab if it doesn't exist
    if (!channelDisplays.contains("#test")) {
        createChannelTab("#test");
    }
    
    // Display all messages in the default channel for now
    auto display = channelDisplays["#test"];
    if (!display) return;
    
    if (message.command == "PRIVMSG") {
        display->addMessage(message.nickname(), message.trailing);
    }
    else if (message.command == "NOTICE") {
        display->addSystemMessage(QString("NOTICE: %1").arg(message.trailing));
    }
    else if (message.command == "ERROR") {
        display->addSystemMessage(QString("ERROR: %1").arg(message.trailing));
    }
    else if (message.command.toInt() > 0) {
        // Numeric replies
        display->addSystemMessage(message.trailing);
    }
}

void MainWindow::sendMessage() {
    if (currentChannel.isEmpty() || messageInput->text().isEmpty()) {
        return;
    }
    
    QString message = messageInput->text();
    ircClient->sendMessage(currentChannel, message);
    
    // Show message in our own chat display
    if (auto display = channelDisplays[currentChannel]) {
        display->addMessage(ircClient->nickname(), message);
    }
    
    messageInput->clear();
}

void MainWindow::createChannelTab(const QString& channel) {
    if (!channelDisplays.contains(channel)) {
        auto display = new ChatDisplay(this);
        channelDisplays[channel] = display;
        channelTabs->addTab(display, channel);
        channelList->addChannel(channel);
        display->addSystemMessage(QString("Joined channel %1").arg(channel));
    }
}

void MainWindow::handleChannelChanged(const QString& channel) {
    currentChannel = channel;
    userList->setCurrentChannel(channel);
    
    // Find and activate the corresponding tab
    for (int i = 0; i < channelTabs->count(); i++) {
        if (channelTabs->tabText(i) == channel) {
            channelTabs->setCurrentIndex(i);
            break;
        }
    }
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About ComSock"),
        tr("ComSock IRC Client\n"
           "A simple IRC client written in Qt\n"
           "Licensed under GNU GPL v2"));
}

void MainWindow::handleConnect() {
    // Already implemented in showConnectDialog()
}

void MainWindow::handleDisconnect() {
    ircClient->disconnect();
    for (auto display : channelDisplays) {
        display->addSystemMessage("Disconnected from server");
    }
}

void MainWindow::handleUserJoined(const QString& channel, const QString& user) {
    if (channelDisplays.contains(channel)) {
        channelDisplays[channel]->addSystemMessage(
            QString("%1 has joined %2").arg(user, channel)
        );
    }
}

void MainWindow::handleUserLeft(const QString& channel, const QString& user) {
    if (channelDisplays.contains(channel)) {
        channelDisplays[channel]->addSystemMessage(
            QString("%1 has left %2").arg(user, channel)
        );
    }
}

void MainWindow::handleTabChanged(int index) {
    if (index >= 0) {
        QString channel = channelTabs->tabText(index);
        handleChannelChanged(channel);
    }
}