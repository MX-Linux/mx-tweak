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
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QComboBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("mx-tweak");

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QString("mx-tweak_") + QLocale::system().name(), "/usr/share/mx-tweak/locale");
    a.installTranslator(&appTran);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption noguiOption{QStringList() << "terminal" << "t", "Enables commandline mode, not launching the gui"};
    parser.addOption(noguiOption);
    QCommandLineOption setThemeSetOption{QStringList() << "set-theme-set", "Applys the theme set with <THEME_SET_NAME>, is case sensitive", "THEME_SET_NAME"};
    parser.addOption(setThemeSetOption);

    parser.process(a.arguments());

    defaultlook w;
    if(parser.isSet(noguiOption))
    {
        // enable terminal mode
        w.terminalFlag = true;
        if(parser.isSet(setThemeSetOption))
        {
            QString themeSetName = parser.value(setThemeSetOption);
            int index = w.ui_comboTheme()->findText(themeSetName);
            if(index == -1)
            {
                qCritical("Invalid theme set name");
                return -1;
            }
            w.ui_comboTheme()->setCurrentIndex(index);
            // calling slot directly
            w.on_buttonThemeApply_clicked();
        }
        // got to return before a.exec() to terminate the application
        return 0;
    }
    else
    {
        w.terminalFlag = false;
        w.show();
    }

    return a.exec();
}
