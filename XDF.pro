#-------------------------------------------------
#
# Project created by QtCreator 2016-12-03T22:30:55
#
#-------------------------------------------------

QT       -= gui

TARGET = xdf
TEMPLATE = lib
CONFIG += staticlib

SOURCES += xdf.cpp \
    pugixml.cpp \
    filtering.c \
    multi_stage.c \
    polyfilt.c \
    remez_lp.c \
    smarc.c \
    stage_impl.c

HEADERS += xdf.h \
    filtering.h \
    multi_stage.h \
    polyfilt.h \
    pugiconfig.hpp \
    pugixml.hpp \
    remez_lp.h \
    smarc.h \
    stage_impl.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

QMAKE_CFLAGS += -std=c99

QMAKE_CXXFLAGS=-std=gnu++11
