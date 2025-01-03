#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QMenuBar>
#include <QTabWidget>
#include <QLabel>
#include <QTimer>
#include "widgets/chan_list.h"
#include "widgets/usr_list.h"
#include "widgets/msg_display.h"
#include "../core/client.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void showConnectDialog();
    void handleConnect();
    void handleDisconnect();
    void handleMessageReceived(const IrcMessage& message);
    void handleUserJoined(const QString& channel, const QString& user);
    void handleUserLeft(const QString& channel, const QString& user);
    void handleChannelChanged(const QString& channel);
    void handleTabChanged(int index);
    void sendMessage();
    void about();

private:
    // Core IRC client
    IrcClient* ircClient;

    // UI Components
    QMenuBar* menuBar;
    ChannelList* channelList;
    UserList* userList;
    QTabWidget* channelTabs;
    QLineEdit* messageInput;
    QLabel* nickDisplay;
    
    // Channel management
    QMap<QString, ChatDisplay*> channelDisplays;
    QString currentChannel;
    
    void setupMenuBar();
    void setupLayout();
    void createChannelTab(const QString& channel);
    void removeChannelTab(const QString& channel);
};