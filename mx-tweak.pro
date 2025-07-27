#-------------------------------------------------
#
# Project created by QtCreator 2016-10-07T19:51:20
#
#-------------------------------------------------

QT       += core gui widgets
CONFIG   += c++20
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060800

TARGET = mx-tweak
TEMPLATE = app
CONFIG += debug_and_release warn_on strict_c++
CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -flto=auto
    QMAKE_LFLAGS += -flto=auto
}

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
