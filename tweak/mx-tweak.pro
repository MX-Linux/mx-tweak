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

TRANSLATIONS += translations/mx-tweak_am.ts \
                translations/mx-tweak_ca.ts \
                translations/mx-tweak_cs.ts \
                translations/mx-tweak_de.ts \
                translations/mx-tweak_el.ts \
                translations/mx-tweak_es.ts \
                translations/mx-tweak_fi.ts \
                translations/mx-tweak_fr.ts \
                translations/mx-tweak_hi.ts \
                translations/mx-tweak_hr.ts \
                translations/mx-tweak_hu.ts \
                translations/mx-tweak_it.ts \
                translations/mx-tweak_ja.ts \
                translations/mx-tweak_kk.ts \
                translations/mx-tweak_lt.ts \
                translations/mx-tweak_nl.ts \
                translations/mx-tweak_pl.ts \
                translations/mx-tweak_pt.ts \
                translations/mx-tweak_pt_BR.ts \
                translations/mx-tweak_ro.ts \
                translations/mx-tweak_ru.ts \
                translations/mx-tweak_sk.ts \
                translations/mx-tweak_sv.ts \
                translations/mx-tweak_tr.ts \
                translations/mx-tweak_uk.ts \
                translations/mx-tweak_zh_TW.ts

RESOURCES += \
    images.qrc
