#include "usr_list.h"
#include "../../utils/color.h"

UserList::UserList(QWidget* parent) : QListWidget(parent) {
    setSelectionMode(QAbstractItemView::NoSelection);
}

void UserList::updateUsers(const QString& channel, const QStringList& users) {
    channelUsers[channel] = users;
    if (channel == activeChannel) {
        clear();
        for (const auto& user : users) {
            auto item = new QListWidgetItem(user);
            item->setForeground(ColorGenerator::generateNickColor(user));
            addItem(item);
        }
    }
}

void UserList::clearChannel(const QString& channel) {
    channelUsers.remove(channel);
    if (channel == activeChannel) {
        clear();
    }
}

void UserList::setCurrentChannel(const QString& channel) {
    activeChannel = channel;
    clear();
    if (channelUsers.contains(channel)) {
        for (const auto& user : channelUsers[channel]) {
            auto item = new QListWidgetItem(user);
            item->setForeground(ColorGenerator::generateNickColor(user));
            addItem(item);
        }
    }
}