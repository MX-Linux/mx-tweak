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
#include "defaultlook.h"
#include <QApplication>
#include <unistd.h>
#include <QTranslator>
#include <QLocale>
#include "brightness_small.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QString("mx-tweak_") + QLocale::system().name(), "/usr/share/mx-tweak/locale");
    a.installTranslator(&appTran);

    if (a.arguments().contains("--tray")){
        brightness_small fred(0,a.arguments());
        return a.exec();
        //    } else {
        //        if (system("echo $XDG_CURRENT_DESKTOP | grep -q XFCE") != 0){
        //            QMessageBox::information(0, QApplication::tr("MX Tweak"),
        //                                     QApplication::tr("This app is Xfce-only"));
        //            exit(0);
    }
    defaultlook w(0, a.arguments());
    w.show();
    return a.exec();
}

