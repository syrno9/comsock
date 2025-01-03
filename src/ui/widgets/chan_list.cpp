#include "chan_list.h"

ChannelList::ChannelList(QWidget* parent) : QListWidget(parent) {
    connect(this, &QListWidget::itemClicked, this, &ChannelList::handleItemClicked);
}

void ChannelList::addChannel(const QString& channel) {
    if (!findItems(channel, Qt::MatchExactly).isEmpty()) return;
    addItem(channel);
}

void ChannelList::removeChannel(const QString& channel) {
    auto items = findItems(channel, Qt::MatchExactly);
    for (auto item : items) {
        delete takeItem(row(item));
    }
}

QString ChannelList::currentChannel() const {
    return currentItem() ? currentItem()->text() : QString();
}

void ChannelList::handleItemClicked(QListWidgetItem* item) {
    emit channelChanged(item->text());
}