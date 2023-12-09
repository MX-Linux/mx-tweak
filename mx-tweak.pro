#-------------------------------------------------
#
# Project created by QtCreator 2016-10-07T19:51:20
#
#-------------------------------------------------

QT       += core gui widgets
CONFIG   += c++1z

TARGET = mx-tweak
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    about.cpp \
        defaultlook.cpp \
    xfwm_compositor_settings.cpp \
    window_buttons.cpp \
    theming_to_tweak.cpp \
    remove_user_theme_set.cpp \
    brightness_small.cpp

HEADERS  += defaultlook.h \
    about.h \
    cmd.h \
    version.h \
    xfwm_compositor_settings.h \
    window_buttons.h \
    theming_to_tweak.h \
    remove_user_theme_set.h \
    brightness_small.h

FORMS    += defaultlook.ui \
    xfwm_compositor_settings.ui \
    window_buttons.ui \
    theming_to_tweak.ui \
    remove_user_theme_set.ui \
    brightness_small.ui

TRANSLATIONS += translations/mx-tweak_en.ts

RESOURCES += \
    images.qrc
