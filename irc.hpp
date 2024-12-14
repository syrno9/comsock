#ifndef COMSOCK_H
#define COMSOCK_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QListWidget>
#include <QMenuBar>
#include <QTabWidget>
#include <QMap>
#include <QVBoxLayout>
#include <QStackedWidget>

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
    void switchChannel(QListWidgetItem *item);
    void updateUserListForChannel(const QString &channel);

private:
    QLineEdit *serverInput;
    QLineEdit *nicknameInput;
    QTextEdit *messageDisplay;
    QLineEdit *messageInput;
    QTcpSocket *socket;
    QListWidget *userList;
    QListWidget *channelList;
    QString currentChannel;
    QMenuBar *menuBar;
    QMap<QString, QTextEdit*> channelDisplays;
    QVBoxLayout *mainLayout;
    QStackedWidget *messageStack;
};

#endif 
