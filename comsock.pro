QT += core gui network widgets

# Add this line to specify the include path for Qt headers
INCLUDEPATH += /usr/include/qt6 /usr/include/qt6/QtGui

# Ensure the QtGui include path is specified
INCLUDEPATH += /usr/include/qt6/QtGui

TARGET = ComSock
TEMPLATE = app

SOURCES += main.cpp \
           irc.cpp

HEADERS += irc.hpp
