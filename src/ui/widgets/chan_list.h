#pragma once
#include <QListWidget>

class ChannelList : public QListWidget {
    Q_OBJECT
public:
    explicit ChannelList(QWidget* parent = nullptr);
    
    void addChannel(const QString& channel);
    void removeChannel(const QString& channel);
    QString currentChannel() const;

signals:
    void channelChanged(const QString& channel);

private slots:
    void handleItemClicked(QListWidgetItem* item);
};