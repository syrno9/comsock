#include <QApplication>
#include "irc.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ComSock client;
    client.setWindowTitle("ComSock");
    client.resize(400, 300);
    client.show();
    return app.exec();
}
