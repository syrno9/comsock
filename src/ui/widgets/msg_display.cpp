#include "msg_display.h"
#include "../../utils/color.h"

ChatDisplay::ChatDisplay(QWidget* parent) : QTextEdit(parent) {
    setReadOnly(true);
}

void ChatDisplay::addMessage(const QString& sender, const QString& message, 
                           const QDateTime& timestamp) {
    QString timeStr = timestamp.toString("[hh:mm:ss] ");
    QColor userColor = ColorGenerator::generateNickColor(sender);
    QString coloredNick = QString("<span style='color: %1'>%2</span>")
                         .arg(userColor.name(), sender);
    
    append(QString("%1%2 %3")
        .arg(timeStr)
        .arg(QString("&lt;%1&gt;").arg(coloredNick))
        .arg(message));
}

void ChatDisplay::addSystemMessage(const QString& message, const QDateTime& timestamp) {
    QString timeStr = timestamp.toString("[hh:mm:ss] ");
    append(QString("%1* %2").arg(timeStr, message));
}

void ChatDisplay::addUserAction(const QString& user, const QString& action,
                              const QDateTime& timestamp) {
    QString timeStr = timestamp.toString("[hh:mm:ss] ");
    QColor userColor = ColorGenerator::generateNickColor(user);
    QString coloredNick = QString("<span style='color: %1'>%2</span>")
                         .arg(userColor.name(), user);
    
    append(QString("%1* %2 %3").arg(timeStr, coloredNick, action));
}