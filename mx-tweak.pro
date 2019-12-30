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
    xfwm_compositor_settings.cpp \
    window_buttons.cpp \
    theming_to_tweak.cpp \
    remove_user_theme_set.cpp

HEADERS  += defaultlook.h \
    xfwm_compositor_settings.h \
    window_buttons.h \
    theming_to_tweak.h \
    remove_user_theme_set.h

FORMS    += defaultlook.ui \
    xfwm_compositor_settings.ui \
    window_buttons.ui \
    theming_to_tweak.ui \
    remove_user_theme_set.ui

TRANSLATIONS += translations/mx-tweak_am.ts \
                translations/mx-tweak_ar.ts \
                translations/mx-tweak_bg.ts \
                translations/mx-tweak_bn.ts \
                translations/mx-tweak_ca.ts \
                translations/mx-tweak_cs.ts \
                translations/mx-tweak_da.ts \
                translations/mx-tweak_de.ts \
                translations/mx-tweak_el.ts \
                translations/mx-tweak_es.ts \
                translations/mx-tweak_et.ts \
                translations/mx-tweak_eu.ts \
                translations/mx-tweak_fa.ts \
                translations/mx-tweak_fi.ts \
                translations/mx-tweak_fil_PH.ts \
                translations/mx-tweak_fr.ts \
                translations/mx-tweak_he_IL.ts \
                translations/mx-tweak_hi.ts \
                translations/mx-tweak_hr.ts \
                translations/mx-tweak_hu.ts \
                translations/mx-tweak_id.ts \
                translations/mx-tweak_is.ts \
                translations/mx-tweak_it.ts \
                translations/mx-tweak_ja.ts \
                translations/mx-tweak_ja_JP.ts \
                translations/mx-tweak_kk.ts \
                translations/mx-tweak_ko.ts \
                translations/mx-tweak_lt.ts \
                translations/mx-tweak_mk.ts \
                translations/mx-tweak_mr.ts \
                translations/mx-tweak_nb.ts \
                translations/mx-tweak_nl.ts \
                translations/mx-tweak_pl.ts \
                translations/mx-tweak_pt.ts \
                translations/mx-tweak_pt_BR.ts \
                translations/mx-tweak_ro.ts \
                translations/mx-tweak_ru.ts \
                translations/mx-tweak_sk.ts \
                translations/mx-tweak_sl.ts \
                translations/mx-tweak_sq.ts \
                translations/mx-tweak_sr.ts \
                translations/mx-tweak_sv.ts \
                translations/mx-tweak_tr.ts \
                translations/mx-tweak_uk.ts \
                translations/mx-tweak_vi.ts \
                translations/mx-tweak_zh_CN.ts \
                translations/mx-tweak_zh_TW.ts

RESOURCES += \
    images.qrc
