QT += core gui widgets

TARGET = simple_ide
TEMPLATE = app

CONFIG += c++17

INCLUDEPATH += include 

HEADERS += \
    include/CodeEditor.h

SOURCES += \
    src/main.cpp \
    src/CodeEditor.cpp