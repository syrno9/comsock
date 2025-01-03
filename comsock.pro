QT = core gui network widgets
CONFIG += c++17

TARGET = ComSock
TEMPLATE = app

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/ui/main_win.cpp \
    src/core/client.cpp \
    src/core/message.cpp \
    src/ui/dialogs/connect.cpp \
    src/ui/widgets/chan_list.cpp \
    src/ui/widgets/usr_list.cpp \
    src/ui/widgets/msg_display.cpp \
    src/utils/color.cpp

HEADERS += \
    src/core/client.h \
    src/core/message.h \
    src/ui/dialogs/connect.h \
    src/ui/widgets/chan_list.h \
    src/ui/widgets/usr_list.h \
    src/ui/widgets/msg_display.h \
    src/ui/main_win.h \
    src/utils/color.h
