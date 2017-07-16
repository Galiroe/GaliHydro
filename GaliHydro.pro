#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T15:47:11
#
#-------------------------------------------------

QT       += core gui\
            sql\

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GaliHydro
TEMPLATE = app


SOURCES += main.cpp\
        galihydro.cpp \
    bdddir.cpp \
    stationselect.cpp \
    options.cpp \
    bdd_singleton.cpp \
    mqaction.cpp \
    apropos.cpp \
    groupedit.cpp \
    corth.cpp

HEADERS  += galihydro.h \
    bdddir.h \
    stationselect.h \
    options.h \
    bdd_singleton.h \
    mqaction.h \
    apropos.h \
    groupedit.h \
    corth.h

FORMS    += galihydro.ui \
    bdddir.ui \
    stationselect.ui \
    options.ui \
    apropos.ui \
    groupedit.ui \
    corth.ui

RC_FILE += \
    ressources.rc
