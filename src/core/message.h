#pragma once
#include <QString>
#include <QDateTime>

struct IrcMessage {
    QString prefix;
    QString command;
    QString params;
    QString trailing;
    QString raw;
    QDateTime timestamp;
    
    static IrcMessage parse(const QString& raw);
    QString nickname() const;
};