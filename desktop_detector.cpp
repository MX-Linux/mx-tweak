/**********************************************************************
 *  desktop_detector.cpp
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

#include "desktop_detector.h"
#include "cmd.h"
#include <QDebug>
#include <QFile>

using namespace Qt::Literals::StringLiterals;

DesktopEnvironment DesktopDetector::detectDesktopEnvironment(bool verbose)
{
    DesktopEnvironment env;

    QString test
        = runCmd(u"ps -aux |grep  -E \'plasmashell|xfce4-session|fluxbox|lightdm|xfce-superkey\' |grep -v grep"_s)
              .output;

    if (test.contains("xfce4-session"_L1)) {
        env.isXfce = true;
    } else if (test.contains("plasmashell"_L1)) {
        env.isKDE = true;
    } else if (test.contains("fluxbox"_L1)) {
        env.isFluxbox = true;
    }

    if (test.contains("lightdm"_L1)) {
        env.isLightdm = true;
    }

    if (test.contains("xfce-superkey"_L1)) {
        env.isSuperkey = true;
    }

    if (verbose) {
        qDebug() << "isXfce is" << env.isXfce << "isKDE is" << env.isKDE << "isFluxbox is" << env.isFluxbox
                 << "isLightdm is" << env.isLightdm << "isSuperkey is" << env.isSuperkey;
    }

    return env;
}

bool DesktopDetector::checkLightdm()
{
    return QFile::exists(u"/usr/bin/lightdm"_s);
}
