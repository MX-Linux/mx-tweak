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

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <unistd.h>
#include "brightness_small.h"
#include "defaultlook.h"
#include "QCommandLineParser"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTran;
    qtTran.load(QStringLiteral("qt_") + QLocale::system().name());
    QApplication::installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QStringLiteral("mx-tweak_") + QLocale::system().name(), QStringLiteral("/usr/share/mx-tweak/locale"));
    QApplication::installTranslator(&appTran);


    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("GUI for applying assorted useful tweaks"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"tray", QObject::tr("launches brightness-systray")});
    parser.addOption({"display", QObject::tr("opens with display tab open.  Only valid with Xfce desktop running")});
    parser.addOption({"theme", QObject::tr("Opens theme tab directly.  Valid on Xfce & Fluxbox desktops")});
    parser.addOption({"other", QObject::tr("Opens Other tab directly.  Valid on all desktops")});
    parser.process(a);


    if (parser.isSet("tray")){
        brightness_small fred(nullptr,QApplication::arguments());
        return QApplication::exec();
    }

    defaultlook w(nullptr, QApplication::arguments());
    w.show();
    return QApplication::exec();
}

