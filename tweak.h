/**********************************************************************
 *  tweak.h
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

#ifndef TWEAK_H
#define TWEAK_H

#include <QDialog>
#include <QFile>
#include <QMessageBox>

namespace Ui {
class Tweak;
}

namespace IconSize {
enum {Default, Small, Medium, Large, Larger, Largest};
}

namespace Tab {
enum {Panel, Theme, Compositor, Display, Config, Fluxbox, Plasma, Superkey, Others};
}

namespace ValueSize { // kde value starts from 1 vs. combobox index 1=0,2=1,3=2,4=3,5=4,6=5
enum {Default = 1, Small, Medium, Large, Larger, Largest};
}

class Tweak : public QDialog
{
    Q_OBJECT

public:
    explicit Tweak(QWidget *parent = 0, const QStringList &args = QStringList()) noexcept;
    ~Tweak() noexcept;
    bool verbose = false;
    bool displayflag = false;
    bool themetabflag = false;
    bool othertabflag = false;
    bool isXfce = false;
    bool isFluxbox = false;
    bool isKDE = false;
    bool isSuperkey = false;

    void setup() noexcept;
    void checkSession() noexcept;
private:
    Ui::Tweak *ui;
    class TweakTheme *tweakTheme = nullptr;
    class TweakPlasma *tweakPlasma = nullptr;
    class TweakXfce *tweakXfce = nullptr;
    class TweakFluxbox *tweakFluxbox = nullptr;
    class TweakXfcePanel *tweakXfcePanel = nullptr;
    class TweakThunar *tweakThunar = nullptr;
    class TweakCompositor *tweakCompositor = nullptr;
    class TweakDisplay *tweakDisplay = nullptr;
    class TweakSuperKey *tweakSuperKey = nullptr;
    class TweakMisc *tweakMisc = nullptr;

    void saveSettings() noexcept;
    void loadSettings() noexcept;

    void pushAbout_clicked() noexcept;
    void pushHelp_clicked() noexcept;

    void tabWidget_currentChanged(int index) noexcept;
};

#endif // TWEAK_H
