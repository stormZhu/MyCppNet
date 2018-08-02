TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lws2_32

SOURCES += \
    client.cpp

HEADERS += \
    EasyTcpClient.h \
    MessageHeader.h
