QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Finite-State_Machine_Emulator

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp \
        mainwindow.cpp \
        fsmachine.cpp

HEADERS  += mainwindow.h \
                   fsmachine.h

CONFIG   += exception

FORMS    += mainwindow.ui
