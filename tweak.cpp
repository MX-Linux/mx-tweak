/**********************************************************************
 *  tweak.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Dolphin Oracle
 *         MX Linux <http://mxlinux.org>
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

#include <cassert>
#include <utility>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QDirIterator>
#include <QFileDialog>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QRegularExpression>
#include <QSettings>

#include "tweak_theme.h"
#include "tweak_plasma.h"
#include "tweak_xfce.h"
#include "tweak_fluxbox.h"
#include "tweak_xfce_panel.h"
#include "tweak_thunar.h"
#include "tweak_compositor.h"
#include "tweak_display.h"
#include "tweak_superkey.h"
#include "tweak_misc.h"
#include "tweak.h"
#include "remove_user_theme_set.h"
#include "ui_tweak.h"
#include "version.h"

import about;
import command;

using namespace Qt::Literals::StringLiterals;

Tweak::Tweak(QWidget *parent, const QStringList &args) noexcept
    : QDialog(parent), ui(new Ui::Tweak)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    // check session type
    checkSession();
    if ( args.contains(u"--display"_s)) {
        if (isXfce) {
            displayflag = true;
        } else {
            QMessageBox::information(nullptr, tr("MX Tweak"),
                                     tr("--display switch only valid for Xfce"));
        }
    }

    if (args.contains(u"--theme"_s)){
        themetabflag = true;
    }

    if (args.contains(u"--other"_s)){
        othertabflag = true;
    }
    if (args.contains(u"--verbose"_s)) {
        verbose = true;
    }

    connect(ui->pushAbout, &QPushButton::clicked, this, &Tweak::pushAbout_clicked);
    connect(ui->pushHelp, &QPushButton::clicked, this, &Tweak::pushHelp_clicked);
    connect(ui->pushClose, &QPushButton::clicked, this, &Tweak::close);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &Tweak::tabWidget_currentChanged);

    setup();
}

Tweak::~Tweak() noexcept
{
    saveSettings();
    if (tweakTheme) delete tweakTheme;
    if (tweakPlasma) delete tweakPlasma;
    if (tweakXfce) delete tweakXfce;
    if (tweakFluxbox) delete tweakFluxbox;
    if (tweakXfcePanel) delete tweakXfcePanel;
    if (tweakThunar) delete tweakThunar;
    if (tweakCompositor) delete tweakCompositor;
    if (tweakDisplay) delete tweakDisplay;
    if (tweakSuperKey) delete tweakSuperKey;
    if (tweakMisc) delete tweakMisc;
    delete ui;
}

void Tweak::checkSession() noexcept
{
    QString test = runCmd(u"ps -aux |grep  -E \'plasmashell|xfce4-session|fluxbox|xfce-superkey\' |grep -v grep"_s).output;
    if (test.contains("xfce4-session"_L1)){
        isXfce = true;
    } else if (test.contains("plasmashell"_L1)) {
        isKDE = true;
    } else if (test.contains("fluxbox"_L1)){
        isFluxbox = true;
    }
    if (test.contains("xfce-superkey"_L1)){
        isSuperkey = true;
    }
    if (verbose) qDebug() << "isXfce is " << isXfce << "isKDE is " << isKDE << "isFluxbox is " << isFluxbox << "isSuperkey is" << isSuperkey;

}
// setup versious items first time program runs
void Tweak::setup() noexcept
{
    QString home_path = QDir::homePath();
    //load saved settings, for now only fluxbox legacy styles checkbox
    loadSettings();

    if (themetabflag) ui->tabWidget->setCurrentIndex(Tab::Theme);
    if (othertabflag) ui->tabWidget->setCurrentIndex(Tab::Others);
    if (displayflag) ui->tabWidget->setCurrentIndex(Tab::Display);

    if (isXfce) {
        ui->pushXfcePanelSettings->setIcon(QIcon::fromTheme(u"org.xfce.panel"_s));
        ui->pushXfceAppearance->setIcon(QIcon::fromTheme(u"org.xfce.settings.appearance"_s));
        ui->pushXfceWindowManager->setIcon(QIcon::fromTheme(u"org.xfce.xfwm4"_s));
        //set first tab as default
        if (!displayflag && !themetabflag && !othertabflag) {
            ui->tabWidget->setCurrentIndex(Tab::Panel);
        }
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Fluxbox);
        tweakXfce = new TweakXfce(ui, verbose, this);
        tweakXfcePanel = new TweakXfcePanel(ui, verbose, this);
        tweakTheme = new TweakTheme(ui, verbose, tweakXfcePanel, this);
        tweakThunar = new TweakThunar(ui, false, this);
        if (TweakCompositor::check()) {
            tweakCompositor = new TweakCompositor(ui, verbose, this);
        } else {
            ui->tabWidget->removeTab(Tab::Compositor);
        }
        if (isSuperkey && TweakSuperKey::checkSuperKey()) {
            tweakSuperKey = new TweakSuperKey(ui, verbose, this);
        } else {
            ui->tabWidget->removeTab(Tab::Superkey);
        }
    }

    //setup fluxbox
    else if (isFluxbox) {
        ui->comboTheme->hide();
        ui->groupTheme->hide();
        ui->pushThemeApply->hide();
        ui->pushThemeRemoveSet->hide();
        ui->groupXFCESettings->hide();
        if (!themetabflag && !othertabflag) {
            ui->tabWidget->setCurrentIndex(Tab::Fluxbox);
        }
        ui->tabWidget->removeTab(Tab::Superkey);
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Config);
        ui->tabWidget->removeTab(Tab::Display);
        ui->tabWidget->removeTab(Tab::Panel);
        tweakFluxbox = new TweakFluxbox(ui, verbose, this);
        tweakTheme = new TweakTheme(ui, verbose, TweakTheme::Fluxbox, this);
        if (TweakThunar::check()) {
            tweakThunar = new TweakThunar(ui, true, this);
        }
    }
//Panel, Theme, Compositor, Display, Config, Fluxbox, Plasma, Superkey, Others
    //setup plasma
    else if (isKDE) {
        ui->pushThemeRemoveSet->hide();
        ui->groupXFCESettings->hide();
        if (!themetabflag && !othertabflag) {
            ui->tabWidget->setCurrentIndex(Tab::Plasma);
        }
        ui->tabWidget->removeTab(Tab::Superkey);
        ui->tabWidget->removeTab(Tab::Fluxbox);
        ui->tabWidget->removeTab(Tab::Config);
        ui->tabWidget->removeTab(Tab::Display);
        ui->tabWidget->removeTab(Tab::Compositor);
        ui->tabWidget->removeTab(Tab::Panel);
        tweakPlasma = new TweakPlasma(ui, verbose, this);
        tweakTheme = new TweakTheme(ui, verbose, TweakTheme::Plasma, this);
    //for other non-supported desktops, show only
    } else {
        ui->groupXFCESettings->hide();
        ui->tabWidget->setCurrentIndex(Tab::Others);
        for (int i = 6; i >= 0; --i) {
            ui->tabWidget->removeTab(i);
        }
    }

    tweakMisc = new TweakMisc(ui, verbose, this);

    //copy template file to ~/.local/share/mx-tweak-data if it doesn't exist
    QDir userdir(home_path + "/.local/share/mx-tweak-data"_L1);
    QFileInfo template_file(home_path + "/.local/share/mx-tweak-data/mx.tweak.template"_L1);
    if (template_file.exists()) {
        if (verbose) qDebug() << "template file found";
    } else {
        if (userdir.exists()) {
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template "_L1 + userdir.absolutePath());
        } else {
            runCmd("mkdir -p "_L1 + userdir.absolutePath());
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template "_L1 + userdir.absolutePath());
        }
    }
    version = getVersion(u"mx-tweak"_s);
    this->adjustSize();
}

void Tweak::pushAbout_clicked() noexcept
{
    this->hide();
    displayAboutMsgBox(tr("About MX Tweak"),
                       "<p align=\"center\"><b><h2>"_L1 + tr("MX Tweak") + "</h2></b></p><p align=\"center\">"_L1 +
                       tr("Version: ") + VERSION"</p><p align=\"center\"><h3>"_L1 +
                       tr("App for quick default ui theme changes and tweaks") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"_L1
                       "<p align=\"center\">"_L1 + tr("Copyright (c) MX Linux") + "<br /><br /></p>"_L1,
                       u"/usr/share/doc/mx-tweak/license.html"_s, tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void Tweak::pushHelp_clicked() noexcept
{
    QLocale locale;
    QString lang = locale.bcp47Name();

    QString url = u"file:///usr/share/doc/mx-tweak/mx-tweak.html"_s;

    if (lang.startsWith("fr"_L1)) {
        url = u"https://mxlinux.org/wiki/help-files/help-tweak-ajustements"_s;
    }
    displayDoc(url, tr("%1 Help").arg(tr("MX Tweak")));
}

// Get version of the program
QString Tweak::getVersion(const QString &name) noexcept
{
    return runCmd("dpkg-query -f '${Version}' -W "_L1 + name).output;
}

void Tweak::tabWidget_currentChanged(int index) noexcept
{
    if (index == Tab::Display && tweakDisplay == nullptr) {
        tweakDisplay = new TweakDisplay(ui, verbose, this);
    }
}

void Tweak::saveSettings() noexcept
{
    QSettings settings(u"MX-Linux"_s, u"MX-Tweak"_s);
    settings.setValue("checkbox_state", ui->checkThemeFluxboxLegacyStyles->checkState());
}

void Tweak::loadSettings() noexcept
{
    QSettings settings(u"MX-Linux"_s, u"MX-Tweak"_s);
    bool checked = settings.value("checkbox_state", false).toBool();
    ui->checkThemeFluxboxLegacyStyles->setChecked(checked);
}
