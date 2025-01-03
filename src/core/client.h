#pragma once
#include <QObject>
#include <QTcpSocket>
#include "message.h"

class IrcClient : public QObject {
    Q_OBJECT
public:
    explicit IrcClient(QObject* parent = nullptr);
    
    void connectToServer(const QString& host, quint16 port = 6667);
    void disconnect();
    void sendMessage(const QString& channel, const QString& message);
    void joinChannel(const QString& channel);
    void setNickname(const QString& nickname);
    void setUsername(const QString& username);
    QString nickname() const { return currentNickname; }
    
signals:
    void connected();
    void disconnected();
    void messageReceived(const IrcMessage& message);
    void userJoined(const QString& channel, const QString& nickname);
    void userLeft(const QString& channel, const QString& nickname);
    void topicChanged(const QString& channel, const QString& topic);
    void error(const QString& error);
    
private slots:
    void handleConnected();
    void handleError(QAbstractSocket::SocketError error);
    void handleSocketData();
    
private:
    QTcpSocket* socket;
    QString currentNickname;
    QString currentUsername;
    bool registrationSent = false;
    
    // Helper methods
    void sendRegistration();
    void sendUserCommand();
    void reconnect();
};