#pragma once
#include <QTextEdit>
#include <QDateTime>

class ChatDisplay : public QTextEdit {
    Q_OBJECT
public:
    explicit ChatDisplay(QWidget* parent = nullptr);
    
    void addMessage(const QString& sender, const QString& message, 
                   const QDateTime& timestamp = QDateTime::currentDateTime());
    void addSystemMessage(const QString& message,
                         const QDateTime& timestamp = QDateTime::currentDateTime());
    void addUserAction(const QString& user, const QString& action,
                      const QDateTime& timestamp = QDateTime::currentDateTime());
};