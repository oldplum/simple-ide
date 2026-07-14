QT += core gui widgets

TARGET = simple_ide
TEMPLATE = app

CONFIG += c++17

INCLUDEPATH += include 

HEADERS += \
    include/CodeEditor.h \
    include/Highlighter.h \
    include/MainWindow.h \
    include/FindReplaceDialog.h \
    include/CatWidget.h

SOURCES += \
    src/main.cpp \
    src/CodeEditor.cpp \
    src/Highlighter.cpp \
    src/MainWindow.cpp \
    src/FindReplaceDialog.cpp \
    src/CatWidget.cpp

FORMS += \
    ui/MainWindow.ui
