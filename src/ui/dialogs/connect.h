#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>

class ConnectDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConnectDialog(QWidget* parent = nullptr);
    
    QString getServer() const;
    QString getNickname() const;
    QString getUsername() const;
    QStringList getAlternativeNicks() const;

private:
    QListWidget* networkList;
    QLineEdit* nicknameInput;
    QLineEdit* nickname2Input;
    QLineEdit* nickname3Input;
    QLineEdit* usernameInput;
    
    void setupDefaultServers();
};