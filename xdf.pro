QT -= gui

TARGET = xdf
TEMPLATE = lib
CONFIG += staticlib c++11

SOURCES += xdf.cpp \
    smarc/filtering.c \
    smarc/multi_stage.c \
    smarc/polyfilt.c \
    smarc/remez_lp.c \
    smarc/smarc.c \
    smarc/stage_impl.c \
    pugixml/pugixml.cpp

HEADERS += xdf.h \
    smarc/filtering.h \
    smarc/multi_stage.h \
    smarc/polyfilt.h \
    smarc/remez_lp.h \
    smarc/smarc.h \
    smarc/stage_impl.h \
    pugixml/pugiconfig.hpp \
    pugixml/pugixml.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}
