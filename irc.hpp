#ifndef COMSOCK_H
#define COMSOCK_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QListWidget>
#include <QMenuBar>

class ComSock : public QWidget {
    Q_OBJECT

public:
    ComSock(QWidget *parent = nullptr);

private slots:
    void connectToServer();
    void sendMessage();
    void readMessage();
    void updateUserList(const QStringList &users);
    void about();
    void connectDialog();

private:
    QLineEdit *serverInput;
    QLineEdit *nicknameInput;
    QTextEdit *messageDisplay;
    QLineEdit *messageInput;
    QTcpSocket *socket;
    QListWidget *userList;
    QString currentChannel;
    QMenuBar *menuBar;
};

#endif 