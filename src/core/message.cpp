#include "message.h"

IrcMessage IrcMessage::parse(const QString& raw) {
    IrcMessage msg;
    msg.raw = raw;
    msg.timestamp = QDateTime::currentDateTime();

    QString work = raw;
    
    if (work.startsWith(':')) {
        int space = work.indexOf(' ');
        msg.prefix = work.mid(1, space - 1);
        work = work.mid(space + 1);
    }
    
    int space = work.indexOf(' ');
    if (space == -1) {
        msg.command = work;
        return msg;
    }
    msg.command = work.left(space);
    work = work.mid(space + 1);
    
    if (work.contains(" :")) {
        int colon = work.indexOf(" :");
        msg.params = work.left(colon);
        msg.trailing = work.mid(colon + 2);
    } else {
        msg.params = work;
    }
    
    return msg;
}

QString IrcMessage::nickname() const {
    if (prefix.isEmpty()) return QString();
    int exclamation = prefix.indexOf('!');
    return exclamation == -1 ? prefix : prefix.left(exclamation);
}