#-------------------------------------------------
#
# Project created by QtCreator 2013-07-10T13:19:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 static

TARGET = SWF-MP3-Looper
TEMPLATE = app

INCLUDEPATH += ../include
LIBS += -L../lib -lavcodec -lavformat -lswresample -lavutil -lmp3lame

win32 {
    # Debug build fails without this when using static Qt
    LIBS += -lgdi32
}


SOURCES += main.cpp \
    Transcode.cpp \
    SwfMp3Sound.cpp \
    SwfMp3Looper.cpp \
    AudioEncoder.cpp \
    AudioDecoder.cpp

HEADERS  += \
    Transcode.hpp \
    SwfMp3Sound.hpp \
    SwfMp3Looper.hpp \
    AudioEncoder.hpp \
    AudioDecoder.hpp

FORMS    += \
    SwfMp3Looper.ui
