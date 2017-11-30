#-------------------------------------------------
#
# Project created by QtCreator 2016-10-07T19:51:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mx-tweak
TEMPLATE = app


SOURCES += main.cpp\
        defaultlook.cpp \
    xfwm_compositor_settings.cpp

HEADERS  += defaultlook.h \
    xfwm_compositor_settings.h

FORMS    += defaultlook.ui \
    xfwm_compositor_settings.ui

TRANSLATIONS += translations/mx-tweak_ca.ts \
                translations/mx-tweak_de.ts \
                translations/mx-tweak_el.ts \
                translations/mx-tweak_fr.ts \
                translations/mx-tweak_it.ts \
                translations/mx-tweak_lt.ts \
                translations/mx-tweak_nl.ts \
                translations/mx-tweak_pt.ts

RESOURCES += \
    images.qrc
