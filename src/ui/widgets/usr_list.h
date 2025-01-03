#pragma once
#include <QListWidget>
#include <QMap>

class UserList : public QListWidget {
    Q_OBJECT
public:
    explicit UserList(QWidget* parent = nullptr);
    
    void updateUsers(const QString& channel, const QStringList& users);
    void clearChannel(const QString& channel);
    void setCurrentChannel(const QString& channel);

private:
    QMap<QString, QStringList> channelUsers;
    QString activeChannel;
};