/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-tweak.
 *
 * mx-tweak is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-tweak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-tweak.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <unistd.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>
#include "brightness_small.h"
#include "tweak.h"

// VERSION should come from compiler flags.
#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion(VERSION);

    QTranslator qtTran;
    (void)qtTran.load("qt_"_L1 + QLocale::system().name());
    QApplication::installTranslator(&qtTran);

    QTranslator appTran;
    (void)appTran.load("mx-tweak_"_L1 + QLocale::system().name(), u"/usr/share/mx-tweak/locale"_s);
    QApplication::installTranslator(&appTran);


    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("GUI for applying assorted useful tweaks"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({u"tray"_s, QObject::tr("launches brightness-systray")});
    parser.addOption({u"display"_s, QObject::tr("opens with display tab open.  Only valid with Xfce desktop running")});
    parser.addOption({u"theme"_s, QObject::tr("Opens theme tab directly.  Valid on Xfce & Fluxbox desktops")});
    parser.addOption({u"verbose"_s, QObject::tr("Display additional debug output in console")});
    parser.addOption({u"other"_s, QObject::tr("Opens Other tab directly.  Valid on all desktops")});
    parser.process(a);


    if (parser.isSet(u"tray"_s)){
        brightness_small fred(nullptr,QApplication::arguments());
        return QApplication::exec();
    }

    Tweak w(nullptr, QApplication::arguments());
    w.show();
    return QApplication::exec();
}
