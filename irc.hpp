#ifndef COMSOCK_H
#define COMSOCK_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QTcpSocket>
#include <QListWidget>
#include <QMenuBar>
#include <QStackedWidget>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>

class ComSock : public QWidget {
    Q_OBJECT

public:
    explicit ComSock(QWidget *parent = nullptr);

private slots:
    void connectToServer();
    void sendMessage();
    void readMessage();
    void connectDialog();
    void switchChannel(QListWidgetItem *item);
    void updateUserList(const QStringList &users);
    void updateUserListForChannel(const QString &channel);
    void about() { QMessageBox::about(this, "About ComSock", "This is a simple IRC client."); }

private:
    QTcpSocket *socket;
    QMenuBar *menuBar;
    QListWidget *channelList, *userList;
    QLineEdit *serverInput, *nicknameInput, *messageInput;
    QTextEdit *messageDisplay;
    QStackedWidget *messageStack;
    QMap<QString, QTextEdit*> channelDisplays;
    QString currentChannel;
};

#endif 
