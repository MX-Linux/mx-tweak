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

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTran;
    qtTran.load(QStringLiteral("qt_") + QLocale::system().name());
    QApplication::installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QStringLiteral("mx-tweak_") + QLocale::system().name(), QStringLiteral("/usr/share/mx-tweak/locale"));
    QApplication::installTranslator(&appTran);

    if (QApplication::arguments().contains(QStringLiteral("--tray"))){
        brightness_small fred(nullptr,QApplication::arguments());
        return QApplication::exec();
        //    } else {
        //        if (system("echo $XDG_CURRENT_DESKTOP | grep -q XFCE") != 0){
        //            QMessageBox::information(0, QApplication::tr("MX Tweak"),
        //                                     QApplication::tr("This app is Xfce-only"));
        //            exit(0);
    }
    defaultlook w(nullptr, QApplication::arguments());
    w.show();
    return QApplication::exec();
}

